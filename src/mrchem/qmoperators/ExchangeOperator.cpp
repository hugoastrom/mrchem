#include "ExchangeOperator.h"
#include "MultiResolutionAnalysis.h"
#include "OrbitalVector.h"

using namespace std;
using namespace Eigen;

ExchangeOperator::ExchangeOperator(double build_prec,
                                   const MultiResolutionAnalysis<3> &mra,
                                   OrbitalVector &phi,
                                   double x_fac)
        : add(mra),
          mult(mra),
          grid(mra),
          poisson(mra, build_prec, build_prec),
          x_factor(x_fac),
          orbitals_0(&phi),
          screen(true) {
    int nOrbs = this->orbitals_0->size();
    this->tot_norms = VectorXd::Zero(nOrbs);
    this->part_norms = MatrixXd::Zero(nOrbs, nOrbs);
}

ExchangeOperator::~ExchangeOperator() {
    this->orbitals_0 = 0;
}

double ExchangeOperator::getScaledPrecision(int i, int j) const {
    double scaled_prec = this->apply_prec;
    if (getScreen()) {
        double tNorm = this->tot_norms(i);
        double pNorm = max(this->part_norms(i,j), this->part_norms(j,i));
        if (tNorm > 0.0) {
            scaled_prec *= tNorm/pNorm;
        }
    }
    return scaled_prec;
}
