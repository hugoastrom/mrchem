//#include <fstream>

#include "Orbital.h"
#include "FunctionTree.h"
#include "SerialFunctionTree.h"
#include "parallel.h"

//used for MRA only:
#include "InterpolatingBasis.h"
#include "mrchem.h"
const MultiResolutionAnalysis<3> *default_mra=0;//used to define mra when not explicitely defined

using namespace std;


Orbital::Orbital(int occ, int s)
        : spin(s),
          occupancy(occ),
          error(1.0),
          real(0),
          imag(0) {
}

Orbital::Orbital(const Orbital &orb)
        : spin(orb.spin),
          occupancy(orb.occupancy),
          error(1.0),
          real(0),
          imag(0) {
}

Orbital& Orbital::operator=(const Orbital &orb) {
    if (this != &orb) {
        if (this->real != 0) MSG_ERROR("Orbital not empty");
        if (this->imag != 0) MSG_ERROR("Orbital not empty");
        this->spin = orb.spin;
        this->occupancy = orb.occupancy;
        this->error = orb.error;
        this->real = orb.real;
        this->imag = orb.imag;
    }
    return *this;
}

void Orbital::clear(bool free) {
    if (this->real != 0 and free) delete this->real;
    if (this->imag != 0 and free) delete this->imag;
    this->real = 0;
    this->imag = 0;
}

int Orbital::getNNodes() const {
    int nNodes = 0;
    if (this->real != 0) nNodes += this->real->getNNodes();
    if (this->imag != 0) nNodes += this->imag->getNNodes();
    return nNodes;
}

double Orbital::getSquareNorm() const {
    double sqNorm = 0.0;
    if (this->real != 0) sqNorm += this->real->getSquareNorm();
    if (this->imag != 0) sqNorm += this->imag->getSquareNorm();
    return sqNorm;
}

void Orbital::compare(const Orbital &orb) const {
    if (this->compareOccupancy(orb) < 0) {
        MSG_WARN("Different occupancy");
    }
    if (this->compareSpin(orb) < 0) {
        MSG_WARN("Different spin");
    }
}

int Orbital::compareOccupancy(const Orbital &orb) const {
    if (this->getOccupancy() == orb.getOccupancy()) {
        return this->getOccupancy();
    }
    return -1;
}

int Orbital::compareSpin(const Orbital &orb) const {
    if (this->getSpin() == orb.getSpin()) {
        return this->getSpin();
    }
    return -1;
}

double Orbital::getExchangeFactor(const Orbital &orb) const {
    if (orb.getSpin() == Paired) {
        return 1.0;
    } else if (this->getSpin() == Paired) {
        return 0.5;
    } else if (this->getSpin() == orb.getSpin()) {
        return 1.0;
    }
    return 0.0;
}

bool Orbital::isConverged(double prec) const {
    if (getError() > prec) {
        return false;
    } else {
        return true;
    }
}

complex<double> Orbital::dot(Orbital &ket) {
    Orbital &bra = *this;
    if ((bra.getSpin() == Alpha) and (ket.getSpin() == Beta)) {
        return 0.0;
    }
    if ((bra.getSpin() == Beta) and (ket.getSpin() == Alpha)) {
        return 0.0;
    }
    double re = 0.0;
    double im = 0.0;
    if (bra.hasReal() and ket.hasReal()) re += bra.re().dot(ket.re());
    if (bra.hasImag() and ket.hasImag()) re += bra.im().dot(ket.im());
    if (bra.hasReal() and ket.hasImag()) im += bra.re().dot(ket.im());
    if (bra.hasImag() and ket.hasReal()) im -= bra.im().dot(ket.re());
    return complex<double>(re, im);
}

void Orbital::normalize() {
    double norm = sqrt(getSquareNorm());
    if (hasReal()) this->re() *= 1.0/norm;
    if (hasImag()) this->im() *= 1.0/norm;
}

void Orbital::orthogonalize(Orbital &phi) {
    NOT_IMPLEMENTED_ABORT;
}


MultiResolutionAnalysis<3>* initializeMRA() {
    // Constructing world box
  const int D=3;
    int scale = Input.get<int>("World.scale");
    int max_depth = Input.get<int>("max_depth");
    vector<int> corner = Input.getIntVec("World.corner");
    vector<int> boxes = Input.getIntVec("World.boxes");
    NodeIndex<D> idx(scale, corner.data());
    BoundingBox<D> world(idx, boxes.data());

    // Constructing scaling basis
    int order = Input.get<int>("order");
    InterpolatingBasis basis(order);

    // Initializing MRA
    return new MultiResolutionAnalysis<D>(world, basis, max_depth);
}

//send an orbital with MPI
void Orbital::sendRcv_Orbital(int source, int dest, int tag){
#ifdef HAVE_MPI
  MPI_Status status;
  MPI_Comm comm=MPI_COMM_WORLD;

  struct Metadata{
    int spin;
    int occupancy;
    int NchunksReal;
    int NchunksImag;
    double error;
  };

  Metadata Orbinfo;

  if(MPI_rank==source){
    Orbinfo.spin=this->getSpin();
    Orbinfo.occupancy=this->getOccupancy();
    Orbinfo.error=this->getError();
    if(this->hasReal()){
      Orbinfo.NchunksReal = this->re().getSerialFunctionTree()->nodeChunks.size();//should reduce to actual number of chunks
    }else{Orbinfo.NchunksReal = 0;}
    if(this->hasImag()){
      Orbinfo.NchunksImag = this->im().getSerialFunctionTree()->nodeChunks.size();//should reduce to actual number of chunks
    }else{Orbinfo.NchunksImag = 0;}

    int count=sizeof(Metadata);
    MPI_Send(&Orbinfo, count, MPI_BYTE, dest, 0, comm);

    if(this->hasReal())SendRcv_SerialTree(&this->re(), Orbinfo.NchunksReal, source, dest, tag, comm);
    if(this->hasImag())SendRcv_SerialTree(&this->im(), Orbinfo.NchunksImag, source, dest, tag*10000, comm);

  }
  if(MPI_rank==dest){
    int count=sizeof(Metadata);
    MPI_Recv(&Orbinfo, count, MPI_BYTE, source, 0, comm, &status);
    this->setSpin(Orbinfo.spin);
    this->setOccupancy(Orbinfo.occupancy);
    this->setError(Orbinfo.error);

    if(Orbinfo.NchunksReal>0){
      if(not this->hasReal()){
	//We must have a tree defined for receiving nodes. Define one:
	if(default_mra==0){
	  default_mra = initializeMRA();
	  cout<<" defined new MRA with depth "<<default_mra->getMaxDepth()<<endl;
	}
	this->real = new FunctionTree<3>(*default_mra,MaxAllocNodes);
      }
    SendRcv_SerialTree(&this->re(), Orbinfo.NchunksReal, source, dest, tag, comm);}

    if(Orbinfo.NchunksImag>0){
      SendRcv_SerialTree(&this->im(), Orbinfo.NchunksImag, source, dest, tag*10000, comm);
    }else{
      //&(this->im())=0;
    }
  }

#endif

}
