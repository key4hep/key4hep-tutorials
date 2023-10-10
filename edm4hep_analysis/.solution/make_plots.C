#include "edm4hep/ReconstructedParticleCollection.h"
#include "edm4hep/utils/kinematics.h"

#include "podio/Frame.h"
#include "podio/ROOTFrameReader.h"

#include "TFile.h"
#include "TH1D.h"

#include <string>
#include <vector>

void make_plots() {
  const std::vector<std::string> inputFiles = {
      "higgs_recoil_from_gaudi_0.edm4hep.root",
      "higgs_recoil_from_gaudi_1.edm4hep.root"};

  const auto e_cms = edm4hep::LorentzVectorE(0, 0, 0, 250.);

  auto reader = podio::ROOTFrameReader();
  reader.openFiles(inputFiles);

  auto h_z_mass = new TH1D("z_mass", ";Mass / GeV;Entries", 240, 60.0, 120.0);
  auto h_recoil_mass =
      new TH1D("recoil_mass", ";Mass / GeV;Entries", 380, 60.0, 250.0);

  for (size_t i = 0; i < reader.getEntries("events"); ++i) {
    const auto event = podio::Frame(reader.readNextEntry("events"));

    const auto &muons =
        event.get<edm4hep::ReconstructedParticleCollection>("Muons");
    if (muons.size() != 2) {
      continue;
    }

    const auto mu1 = edm4hep::utils::p4(muons[0]);
    const auto mu2 = edm4hep::utils::p4(muons[1]);

    const auto z_p4 = mu1 + mu2;
    h_z_mass->Fill(z_p4.M());

    const auto recoil_mass = (e_cms - z_p4).M();
    h_recoil_mass->Fill(recoil_mass);
  }

  auto hist_file = new TFile("higgs_recoil_hists.root", "recreate");
  h_z_mass->Write();
  h_recoil_mass->Write();
  hist_file->Close();
}
