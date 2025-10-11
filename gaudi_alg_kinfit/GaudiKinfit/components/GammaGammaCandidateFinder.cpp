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
  debug() << fmt::format("Considering combinations of {} photons for gamma gamma candidates", photonCandidates.size())
          << endmsg;

  auto output = edm4hep::ReconstructedParticleCollection();

  // TODO:
  // - Setup the loop(s) that build the combinations of photons
  // - Obtain the four momenta from them
  //   - (Optionally) check whether their resulting combined mass is close
  //     enough to the configurable resonance mass
  // - Run a kinematic fit on them. **You will have to first implement some
  //   parts there!**
  //   - (Optionally) check if the (configurable) fit probability is good enough
  // - Create an output particle from the fit results if all checks pass
  //
  // HINT:
  // - Check the `edm4hep/utils/kinematics.h` header for useful functionality
  //   for obtaining four momenta of particles for feeding them into
  //   performKinematicFit. Relevant doxygen page:
  //   https://edm4hep.web.cern.ch/namespaceedm4hep_1_1utils.html
  // - We have added skeleton implementations for performKinematicFit and and
  //   createParticle that we recommond to use. You will have to implement a few
  //   things here and there, but you will not be required to change the
  //   interface.

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
  // TODO: Setup a mass constraint (ideally configurable via a property)

  // TODO: Setup fit objects for the two photons. Re-use the hard-wired
  // approximations for errors as in the original implementation

  auto pfitter = createFitter();
  BaseFitter& fitter = *pfitter;

  // TODO: Add the mass constraint and the fit objects to the fitter

  // NOTE: You don't have to change anything related to the fit below, but we
  // still encourage you at least take a look and try to see if everything makes
  // sense
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
    // TODO: Set the kinematics of the fittedParticle using the kinematics of
    // the fit objects you have created
    result.fittedParticle = {};

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
  // TODO: Create the return value
  auto recoPart = /* Fill me first */;

  const auto& p4 = fitResult.fittedParticle;
  // TODO: Set the kinematics from the fit

  // TODO: PDG and mass as configured

  // TODO: Add the photons as "daughters" of this particle

  // NOTE: Setting the covariance matrix is somewhat involved, so we again leave
  // it as mainly a reading exercise
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
