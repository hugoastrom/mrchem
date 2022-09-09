/*
 * MRChem, a numerical real-space code for molecular electronic structure
 * calculations within the self-consistent field (SCF) approximations of quantum
 * chemistry (Hartree-Fock and Density Functional Theory).
 * Copyright (C) 2023 Stig Rune Jensen, Luca Frediani, Peter Wind and contributors.
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

#pragma once

#include "Permittivity.h"
#include "qmfunctions/Density.h"
#include "qmfunctions/Orbital.h"

namespace mrchem {
/** @class SCRF
 *  @brief class that performs the computation of the  ReactionPotential, named Self Consistent Reaction Field.
 */
class Nuclei;
class KAIN;
class SCRF final {
public:
    SCRF(Permittivity e,
         const Nuclei &N,
         std::shared_ptr<mrcpp::PoissonOperator> P,
         std::shared_ptr<mrcpp::DerivativeOperator<3>> D,
         double orb_prec,
         int kain_hist,
         int max_iter,
         bool acc_pot,
         bool dyn_thrs,
         std::string density_type);
    ~SCRF();
    void UpdateExternalDensity(Density new_density) { this->rho_ext = new_density; }

    double setConvergenceThreshold(double prec);

    mrcpp::CplxFunc &getCurrentReactionPotential() { return this->Vr_n; }
    mrcpp::CplxFunc &getPreviousReactionPotential() { return this->Vr_nm1; }
    mrcpp::CplxFunc &getCurrentDifferenceReactionPotential() { return this->dVr_n; }

    mrcpp::CplxFunc &getCurrentGamma() { return this->gamma_n; }
    mrcpp::CplxFunc &getPreviousGamma() { return this->gamma_nm1; }
    mrcpp::CplxFunc &getCurrentDifferenceGamma() { return this->dgamma_n; }

    Permittivity &getPermittivity() { return this->epsilon; }

    void updateMOResidual(double const err_t) { this->mo_residual = err_t; }

    friend class ReactionPotential;

protected:
    void clear();

private:
    bool accelerate_Vr;
    bool dynamic_thrs;
    std::string density_type;

    int max_iter;
    int history;
    double apply_prec;
    double conv_thrs;
    double mo_residual;

    Permittivity epsilon;

    Density rho_nuc;
    Density rho_ext;
    Density rho_tot;

    mrcpp::CplxFunc Vr_n;
    mrcpp::CplxFunc dVr_n;
    mrcpp::CplxFunc Vr_nm1;

    mrcpp::CplxFunc gamma_n;
    mrcpp::CplxFunc dgamma_n;
    mrcpp::CplxFunc gamma_nm1;

    mrcpp::FunctionTreeVector<3> d_cavity; //!< Vector containing the 3 partial derivatives of the cavity function
    std::shared_ptr<mrcpp::DerivativeOperator<3>> derivative;
    std::shared_ptr<mrcpp::PoissonOperator> poisson;

    void setDCavity();

    void computeDensities(OrbitalVector &Phi);
    void computeGamma(mrcpp::CplxFunc &potential, mrcpp::CplxFunc &out_gamma);

    mrcpp::CplxFunc solvePoissonEquation(const mrcpp::CplxFunc &ingamma);

    void accelerateConvergence(mrcpp::CplxFunc &dfunc, mrcpp::CplxFunc &func, KAIN &kain);

    // TODO    void variationalSCRF(mrcpp::CplxFunc V_vac);
    void nestedSCRF(mrcpp::CplxFunc V_vac);
    mrcpp::CplxFunc &setup(double prec, const std::shared_ptr<mrchem::OrbitalVector> &Phi);

    double getNuclearEnergy();
    double getElectronicEnergy();
    double getTotalEnergy();
    void resetCplxFunc(mrcpp::CplxFunc &function);
    void updateCurrentReactionPotential(mrcpp::CplxFunc &Vr_np1);
    void updateCurrentGamma(mrcpp::CplxFunc &gamma_np1);

    void printParameters() const;
    void printConvergenceRow(int i, double norm, double update, double time) const;
};
} // namespace mrchem
