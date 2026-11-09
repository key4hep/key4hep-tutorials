#include "edm4hep/ReconstructedParticleCollection.h"
#include "edm4hep/utils/kinematics.h"

#include "podio/Frame.h"
#include "podio/Reader.h"

#include "TFile.h"
#include "TH1D.h"

#include <array>
#include <cstdlib>
#include <string>
#include <vector>

void make_plots() {
  const std::vector<std::string> inputFiles = {
      "rv02-02.sv02-02.mILD_l5_o1_v02.E250-SetA.I402004.Pe2e2h.eR.pL.n000.d_"
      "dstm_15090_0.edm4hep.root",
      "rv02-02.sv02-02.mILD_l5_o1_v02.E250-SetA.I402004.Pe2e2h.eR.pL.n000.d_"
      "dstm_15090_1.edm4hep.root",
      "rv02-02.sv02-02.mILD_l5_o1_v02.E250-SetA.I402004.Pe2e2h.eR.pL.n000.d_"
      "dstm_15090_2.edm4hep.root"};

  const auto e_cms = edm4hep::LorentzVectorE(0, 0, 0, 250.);

  auto reader = podio::makeReader(inputFiles);

  auto h_z_mass = new TH1D("z_mass", ";Mass / GeV;Entries", 240, 60.0, 120.0);
  auto h_recoil_mass =
      new TH1D("recoil_mass", ";Mass / GeV;Entries", 380, 60.0, 250.0);

  const std::array<std::string, 3> cutflow_labels = {
      "all events", "n_muons == 2", "opposite charge"};
  auto h_cutflow = new TH1D("cutflow", ";;Events", cutflow_labels.size(), 0,
                            cutflow_labels.size());
  for (size_t i = 0; i < cutflow_labels.size(); ++i) {
    h_cutflow->GetXaxis()->SetBinLabel(i + 1, cutflow_labels[i].c_str());
  }

  for (size_t i = 0; i < reader.getEvents(); ++i) {
    const auto event = reader.readEvent(i);
    h_cutflow->Fill(0);

    const auto &pfos =
        event.get<edm4hep::ReconstructedParticleCollection>("PandoraPFOs");
    std::vector<edm4hep::ReconstructedParticle> muons;
    for (const auto pfo : pfos) {
      if (std::abs(pfo.getPDG()) == 13) {
        muons.push_back(pfo);
      }
    }
    if (muons.size() != 2) {
      continue;
    }
    h_cutflow->Fill(1);

    if (muons[0].getCharge() == muons[1].getCharge()) {
      continue;
    }
    h_cutflow->Fill(2);

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
  h_cutflow->Write();
  hist_file->Close();
}
