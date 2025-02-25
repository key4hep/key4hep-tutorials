/*
 * Copyright (c) 2014-2024 Key4hep-Project.
 *
 * This file is part of Key4hep.
 * See https://key4hep.github.io/key4hep-doc/ for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Gaudi/Property.h"
#include "Gaudi/Accumulators/Histogram.h"
#include "Gaudi/Histograming/Sink/Utils.h"

#include "edm4hep/ReconstructedParticleCollection.h"
#include "edm4hep/utils/kinematics.h"

#include "k4FWCore/Transformer.h"

#include "TH1D.h"

#include <string>

// histogram compatibility when older version of Gaudi is used
#include "GAUDI_VERSION.h"
#if GAUDI_MAJOR_VERSION < 39
namespace Gaudi::Accumulators {
  template <unsigned int ND, atomicity Atomicity = atomicity::full, typename Arithmetic = double>
  using StaticHistogram =
      Gaudi::Accumulators::HistogramingCounterBase<ND, Atomicity, Arithmetic, naming::histogramString,
                                                   HistogramingAccumulator>;
}
#endif

struct HiggsRecoil final
  : k4FWCore::MultiTransformer<std::tuple<edm4hep::ReconstructedParticleCollection,
                                                   edm4hep::ReconstructedParticleCollection>
                                        (const edm4hep::ReconstructedParticleCollection&)> {
  HiggsRecoil(const std::string& name, ISvcLocator* svcLoc)
      : MultiTransformer(
            name, svcLoc,
            {KeyValues("InputMuons", {"Muons"})},
            {KeyValues("HiggsCollection", {"Higgs"}),
             KeyValues("ZCollection", {"Z"})}) {
  }

  std::tuple<edm4hep::ReconstructedParticleCollection,
             edm4hep::ReconstructedParticleCollection>
  operator()(const edm4hep::ReconstructedParticleCollection& recoColl) const override {

    // Create the collections we are going to return
    auto higgs = edm4hep::ReconstructedParticleCollection();
    auto z = edm4hep::ReconstructedParticleCollection();

    // Filter only events with exactly two muons
    if (recoColl.size() != 2) {
      return std::make_tuple(std::move(higgs), std::move(z));
    }

    // Get the four-momenta of the two muons
    auto first = edm4hep::utils::p4(recoColl[0]);
    auto second = edm4hep::utils::p4(recoColl[1]);

    // Calculate the invariant mass of the two muons
    auto zMass = (first+second).M();
    // Fill the histogram
    ++zHist[zMass];
    // Create a new Z candidate and set its mass
    auto newZ = z.create();
    newZ.setMass(zMass);

    // Calculate the recoil mass using that the beam energy is 250 GeV
    auto ecms = ROOT::Math::PxPyPzEVector(0., 0., 0., 250.);
    auto recoil = ecms - (first + second);
    // Fill the histogram
    ++higgsHist[recoil.M()];
    // Create a new Higgs candidate and set its mass
    auto newHiggs = higgs.create();
    newHiggs.setMass(recoil.M());

    return std::make_tuple(std::move(higgs), std::move(z));

  }

  // Thread-safe custom histograms from Gaudi
  // 1 is the dimension of the histogram
  // Here "Higgs mass" is the title of the histogram, then we pass the bins and the axis labels
  mutable Gaudi::Accumulators::StaticHistogram<1> higgsHist{this, "", "Higgs mass", {100, 0., 250., "m_{H} [GeV];Entries"}};
  mutable Gaudi::Accumulators::StaticHistogram<1> zHist{this, "", "Z mass", {100, 0., 250., "m_{Z} [GeV];Entries"}};

  /* We are going to leave thread-safe histogramming commented out since
     the version of Gaudi installed in the stack is not recent enough. Once a
     newer version is installed the following code has been tested and works


  // We run this to save histograms to a file
  StatusCode finalize() override {
    TFile file("histograms.root", "RECREATE");
    std::string name = "";
    // Name that will appear in the stats table
    std::string histName = "Higgs";
    nlohmann::json jH {higgsHist};
    auto [histo, dir] = Gaudi::Histograming::Sink::jsonToRootHistogram<Gaudi::Histograming::Sink::Traits<false, TH1D, 1>>(name, histName, jH[0]);
    // Name of the histogram in the ROOT file
    histo.Write("higgsHist");

    histName = "Z";
    nlohmann::json jZ {zHist};
    std::tie(histo, dir) = Gaudi::Histograming::Sink::jsonToRootHistogram<Gaudi::Histograming::Sink::Traits<false, TH1D, 1>>(name, histName, jZ[0]);
    histo.Write("zHist");

    return StatusCode::SUCCESS;
  }

  */

};

DECLARE_COMPONENT(HiggsRecoil)
