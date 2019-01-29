/*
 * MRChem, a numerical real-space code for molecular electronic structure
 * calculations within the self-consistent field (SCF) approximations of quantum
 * chemistry (Hartree-Fock and Density Functional Theory).
 * Copyright (C) 2019 Stig Rune Jensen, Jonas Juselius, Luca Frediani, and contributors.
 *
 * This file is part of MRChem.
 *
 * MRChem is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * MRChem is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with MRChem.  If not, see <https://www.gnu.org/licenses/>.
 *
 * For information on the complete list of contributors to MRChem, see:
 * <https://mrchem.readthedocs.io/>
 */

/**
 *
 * \date Jul 13, 2010
 * \author Stig Rune Jensen \n
 *		   CTCC, University of Tromsø
 *
 *
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "Nucleus.h"
#include "properties/DipoleMoment.h"
#include "properties/GeometryDerivatives.h"
#include "properties/HyperFineCoupling.h"
#include "properties/Magnetizability.h"
#include "properties/NMRShielding.h"
#include "properties/Polarizability.h"
#include "properties/SCFEnergy.h"
#include "properties/SpinSpinCoupling.h"
#include "properties/properties_fwd.h"

/** @class Molecule
 *
 * @brief Collection of properties related to a molecule.
 *
 * A molecule is basically a collection of nuclei and a collection of properties.
 * Mainly used for bookkeeping and printing of properties.
 *
 */

namespace mrchem {

class Molecule final {
public:
    explicit Molecule(const Nuclei &nucs, int c = 0, int m = 1);
    explicit Molecule(const std::string &coord_file, int c = 0, int m = 1);
    explicit Molecule(const std::vector<std::string> &coord_str, int c = 0, int m = 1);
    Molecule(const Molecule &mol) = delete;
    Molecule &operator=(const Molecule &mol) = delete;

    int getCharge() const { return this->charge; }
    int getMultiplicity() const { return this->multiplicity; }
    int getNNuclei() const { return this->nuclei.size(); }
    int getNElectrons() const;

    const mrcpp::Coord<3> &getCenterOfMass() const { return this->COM; }
    const mrcpp::Coord<3> &getCenterOfCharge() const { return this->COC; }

    Nuclei &getNuclei() { return this->nuclei; }
    Nucleus &getNucleus(int i) { return this->nuclei[i]; }
    const Nuclei &getNuclei() const { return this->nuclei; }
    const Nucleus &getNucleus(int i) const { return this->nuclei[i]; }

    void printGeometry() const;
    void printProperties() const;

    void initSCFEnergy();
    void initDipoleMoment();
    void initGeometryDerivatives();
    void initMagnetizability();
    void initQuadrupoleMoment();
    void initNMRShielding(int k);
    void initHyperFineCoupling(int k);
    void initSpinSpinCoupling(int k, int l);
    void initPolarizability(double omega);
    void initOpticalRotation(double omega);

    SCFEnergy &getSCFEnergy();
    DipoleMoment &getDipoleMoment();
    QuadrupoleMoment &getQuadrupoleMoment();
    GeometryDerivatives &getGeometryDerivatives();
    Magnetizability &getMagnetizability();
    NMRShielding &getNMRShielding(int k);
    HyperFineCoupling &getHyperFineCoupling(int k);
    SpinSpinCoupling &getSpinSpinCoupling(int k, int l);
    Polarizability &getPolarizability(double omega);
    OpticalRotation &getOpticalRotation(double omega);

protected:
    int charge;
    int multiplicity;
    Nuclei nuclei;

    // Properties
    mrcpp::Coord<3> COM{};
    mrcpp::Coord<3> COC{};
    std::unique_ptr<SCFEnergy> energy{};
    std::unique_ptr<DipoleMoment> dipole{};
    std::unique_ptr<GeometryDerivatives> geomderiv{};
    std::unique_ptr<Magnetizability> magnetizability{};
    std::vector<std::unique_ptr<Polarizability>> polarizability{};
    std::vector<std::unique_ptr<NMRShielding>> nmr{};
    std::vector<std::unique_ptr<HyperFineCoupling>> hfcc{};
    std::vector<std::vector<std::unique_ptr<SpinSpinCoupling>>> sscc{};

    void calcCenterOfMass();
    void calcCenterOfCharge();

    void initNuclearProperties(int nNucs);

    void readCoordinateFile(const std::string &file);
    void readCoordinateString(const std::vector<std::string> &coord_str);
};

} // namespace mrchem
