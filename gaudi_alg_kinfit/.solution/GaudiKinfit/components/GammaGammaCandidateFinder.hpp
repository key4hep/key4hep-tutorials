#pragma once

#include <BaseFitter.h>

#include <k4FWCore/Transformer.h>

#include <edm4hep/ReconstructedParticleCollection.h>
#include <edm4hep/utils/kinematics.h>

#include <optional>
#include <vector>

struct GammaGammaCandidateFinder final : public k4FWCore::Transformer<edm4hep::ReconstructedParticleCollection(
                                             const edm4hep::ReconstructedParticleCollection&)> {

  GammaGammaCandidateFinder(const std::string& name, ISvcLocator* svcLoc);

  edm4hep::ReconstructedParticleCollection
  operator()(const edm4hep::ReconstructedParticleCollection& input) const override;

private:
  Gaudi::Property<int> m_resonancePDG{this, "ResonancePDG", 111, "Particle decaying to Gamma Gamma"};

  Gaudi::Property<float> m_resonanceMass{this, "ResonanceMass", 0.135f,
                                         "Nominal Mass of Particle Decaying to Gamma Gamma (GeV)"};

  Gaudi::Property<float> m_maxDeltaM{this, "MaxDeltaM", 0.040f,
                                     "Maximum difference between candidate mass and GammaGama Resonance mass (GeV)"};

  Gaudi::Property<double> m_fitProbabilityCut{this, "MinFitProbability", 0.001, "Minimum fit probability"};

  Gaudi::Property<std::string> m_fitterType{this, "Fitter", "OPALFitter",
                                            "Which fitter to use. Choices: OPALFitter, NewFitter, NewtonFitter"};

private:
  std::unique_ptr<BaseFitter> createFitter() const;

  struct FitResult {
    double fitProbability{};
    edm4hep::LorentzVectorE fittedParticle;
    std::vector<double> covarianceMatrix;
  };

  std::optional<FitResult> performKinematicFit(const edm4hep::LorentzVectorE& gamma1,
                                               const edm4hep::LorentzVectorE& gamma2) const;

  edm4hep::MutableReconstructedParticle createParticle(const FitResult& fitResult,
                                                       const edm4hep::ReconstructedParticle& gamma1,
                                                       const edm4hep::ReconstructedParticle& gamma2) const;
};
