#include "GammaGammaCandidateFinder.hpp"

// MarlinKinfit includes (assuming they're available in the environment)
#include <JetFitObject.h>
#include <MassConstraint.h>
#include <NewFitterGSL.h>
#include <NewtonFitterGSL.h>
#include <OPALFitterGSL.h>

#include <Math/Vector4D.h>

#include <edm4hep/Constants.h>
#include <edm4hep/utils/kinematics.h>
#include <edm4hep/utils/vector_utils.h>

#include <Eigen/Dense>

#include <fmt/format.h>

#include <cmath>
#include <memory>
#include <stdexcept>
#include <vector>

GammaGammaCandidateFinder::GammaGammaCandidateFinder(const std::string& name, ISvcLocator* svcLoc)
    : Transformer(name, svcLoc, {KeyValues("InputCollection", {"PandoraPhotons"})},
                  {KeyValues("OutputCollection", {"GammaGammaCandidates"})}) {

  // Validate fitter type
  if (m_fitterType != "OPALFitter" && m_fitterType != "NewFitter" && m_fitterType != "NewtonFitter") {
    throw std::invalid_argument("Invalid fitter type: " + m_fitterType.value() +
                                ". Allowed values are: OPALFitter, NewFitter, NewtonFitter");
  }
}

edm4hep::ReconstructedParticleCollection
GammaGammaCandidateFinder::operator()(const edm4hep::ReconstructedParticleCollection& photonCandidates) const {
  debug() << fmt::format("Considering combinations of {} photons for gamma gamma candidates ({} with mass {} GeV)",
                         photonCandidates.size(), m_resonancePDG.value(), m_resonanceMass.value())
          << endmsg;

  auto output = edm4hep::ReconstructedParticleCollection();

  if (photonCandidates.size() >= 2) {
    for (size_t i = 0; i < photonCandidates.size() - 1; i++) {
      for (size_t j = i + 1; j < photonCandidates.size(); j++) {

        const auto& gamma1 = edm4hep::utils::p4(photonCandidates[i], edm4hep::utils::UseEnergy);
        const auto& gamma2 = edm4hep::utils::p4(photonCandidates[j], edm4hep::utils::UseEnergy);
        const auto diPhotonP4 = gamma1 + gamma2;

        if (std::abs(diPhotonP4.M() - m_resonanceMass) > m_maxDeltaM) {
          debug() << fmt::format("Combination of photon {} and {} with combined mass {} too far away from configured "
                                 "resonance mass",
                                 i, j, diPhotonP4.M())
                  << endmsg;
          continue;
        }

        debug() << fmt::format("Performing kinematic fit for photon {} and photon {}", i, j) << endmsg;
        if (auto fitResult = performKinematicFit(gamma1, gamma2)) {
          if (fitResult->fitProbability < m_fitProbabilityCut) {
            debug() << fmt::format("Fit probability {} smaller than configured minimum fit probability",
                                   fitResult->fitProbability)
                    << endmsg;
            continue;
          }

          output.push_back(createParticle(fitResult.value(), photonCandidates[i], photonCandidates[j]));
        }
      }
    }
  }

  return output;
}

std::unique_ptr<BaseFitter> GammaGammaCandidateFinder::createFitter() const {
  if (m_fitterType == "NewFitter") {
    return std::make_unique<NewFitterGSL>();
  } else if (m_fitterType == "NewtonFitter") {
    return std::make_unique<NewtonFitterGSL>();
  } else {
    return std::make_unique<OPALFitterGSL>();
  }
}

std::optional<GammaGammaCandidateFinder::FitResult>
GammaGammaCandidateFinder::performKinematicFit(const edm4hep::LorentzVectorE& gamma1,
                                               const edm4hep::LorentzVectorE& gamma2) const {
  MassConstraint mc(m_resonanceMass.value());

  JetFitObject j1(gamma1.E(), gamma1.Theta(), gamma1.Phi(), 0.16 * std::sqrt(gamma1.E()), 0.001 / std::sqrt(gamma1.E()),
                  0.001 / std::sqrt(gamma1.E()), 0.0);
  JetFitObject j2(gamma2.E(), gamma2.Theta(), gamma2.Phi(), 0.16 * std::sqrt(gamma2.E()), 0.001 / std::sqrt(gamma2.E()),
                  0.001 / std::sqrt(gamma2.E()), 0.0);
  mc.addToFOList(j1);
  mc.addToFOList(j2);

  auto pfitter = createFitter();
  BaseFitter& fitter = *pfitter;

  fitter.addFitObject(j1);
  fitter.addFitObject(j2);
  fitter.addConstraint(mc);

  const auto fit_probability = fitter.fit();
  const int nIterations = fitter.getIterations();
  const int errorCode = fitter.getError();

  int cov_dim;
  double* cov = fitter.getGlobalCovarianceMatrix(cov_dim);

  verbose() << fmt::format(
      "Constrained fit results RC: {}, No. of iterations {}, fit probability = {}, cov matrix dimension = {}",
      errorCode, nIterations, fit_probability, cov_dim);

  if (errorCode == 0) {
    FitResult result;
    result.fitProbability = fit_probability;
    result.fittedParticle = {j1.getPx() + j2.getPx(), j1.getPy() + j2.getPy(), j1.getPz() + j2.getPz(),
                             j1.getE() + j2.getE()};

    // Store covariance matrix if available
    if (cov_dim > 0 && cov != nullptr) {
      result.covarianceMatrix.assign(cov, cov + cov_dim * cov_dim);
    }
    return result;
  }

  return std::nullopt;
}

edm4hep::MutableReconstructedParticle
GammaGammaCandidateFinder::createParticle(const FitResult& fitResult, const edm4hep::ReconstructedParticle& gamma1,
                                          const edm4hep::ReconstructedParticle& gamma2) const {
  debug() << fmt::format("Creating resonance particle (x,y,z,E) = ({}, {}, {}, {})", fitResult.fittedParticle.X(),
                         fitResult.fittedParticle.Y(), fitResult.fittedParticle.Z(), fitResult.fittedParticle.E())
          << endmsg;
  auto recoPart = edm4hep::MutableReconstructedParticle{};

  const auto& p4 = fitResult.fittedParticle;
  // Kinematics from fit
  recoPart.setEnergy(p4.E());
  recoPart.getMomentum().x = p4.X();
  recoPart.getMomentum().y = p4.Y();
  recoPart.getMomentum().z = p4.Z();
  recoPart.setGoodnessOfPID(fitResult.fitProbability);

  // PDG and mass as configured
  recoPart.setPDG(m_resonancePDG);
  recoPart.setMass(m_resonanceMass);

  recoPart.addToParticles(gamma1);
  recoPart.addToParticles(gamma2);

  // Convert the covariance matrix back to px, py, pz, E from the (E1, theta1,
  // phi, E2, theta2, phi2) coordinate system used in the fit, via:
  // V' = J^T * V * J, where J is the jacobian matrix of the transfomration
  if (fitResult.covarianceMatrix.size() == 36) {
    constexpr int nrows = 6; // Dimensions of the fit
    constexpr int ncols = 4; // Dimensions of result

    const auto e1 = gamma1.getEnergy();
    const auto e2 = gamma2.getEnergy();
    const auto p1 = gamma1.getMomentum();
    const auto p2 = gamma2.getMomentum();
    auto pt1 = edm4hep::utils::pt(gamma1);
    auto pt2 = edm4hep::utils::pt(gamma2);

    // clang-format off
    const auto J = Eigen::Matrix<double, nrows, ncols> {
      {p1.x / e1,         p1.y / e1,         p1.z / e1, 1.0},
      {p1.x * p1.z / pt1, p1.y * p1.z / pt1, -pt1,      0.0},
      {-p1.y,             p1.x,              0.0,       0.0},
      {p2.x / e2,         p2.y / e2,         p2.z / e2, 1.0},
      {p2.x * p2.z / pt2, p2.y * p2.z / pt2, -pt2,      0.0},
      {-p2.y,             p2.x,              0.0,       0.0}
    };
    // clang-format on
    const auto V = Eigen::Matrix<double, nrows, nrows>(fitResult.covarianceMatrix.data());
    const auto vP = J.transpose() * V * J;

    auto& cov = recoPart.getCovMatrix();
    using enum edm4hep::FourMomCoords;
    cov.setValue(vP(0, 0), x, x);
    cov.setValue(vP(0, 1), x, y);
    cov.setValue(vP(0, 2), x, z);
    cov.setValue(vP(0, 3), x, t);

    cov.setValue(vP(1, 1), y, y);
    cov.setValue(vP(1, 2), y, z);
    cov.setValue(vP(1, 3), y, t);

    cov.setValue(vP(2, 2), z, z);
    cov.setValue(vP(2, 3), z, t);

    cov.setValue(vP(3, 3), t, t);
  }

  return recoPart;
}

DECLARE_COMPONENT(GammaGammaCandidateFinder)
