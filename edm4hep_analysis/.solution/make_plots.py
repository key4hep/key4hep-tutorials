#!/usr/bin/env python3

import glob

from podio.reading import get_reader
from edm4hep import utils, LorentzVectorE

import ROOT


def main():
    files = glob.glob(
        "rv02-02.sv02-02.mILD_l5_o1_v02.E250-SetA.I402004.Pe2e2h.eR.pL.n000.d_dstm_15090_?.edm4hep.root"
    )
    reader = get_reader(files)

    e_cms = LorentzVectorE(0.0, 0.0, 0.0, 250.0)

    h_z_mass = ROOT.TH1D("z_mass", ";Mass / GeV;Entries", 240, 60.0, 120.0)
    h_recoil_mass = ROOT.TH1D("recoil_mass", ";Mass / GeV;Entries", 380, 60.0, 250.0)

    cutflow_labels = ["all events", "n_muons == 2", "opposite charge"]
    h_cutflow = ROOT.TH1D(
        "cutflow", ";;Events", len(cutflow_labels), 0, len(cutflow_labels)
    )
    for i, label in enumerate(cutflow_labels, start=1):
        h_cutflow.GetXaxis().SetBinLabel(i, label)

    for event in reader.get("events"):
        h_cutflow.Fill(0)

        pfos = event.get("PandoraPFOs")
        muons = [p for p in pfos if abs(p.getPDG()) == 13]
        if len(muons) != 2:
            continue
        h_cutflow.Fill(1)

        if muons[0].getCharge() == muons[1].getCharge():
            continue
        h_cutflow.Fill(2)

        mu1 = utils.p4(muons[0])
        mu2 = utils.p4(muons[1])

        z_p4 = mu1 + mu2
        z_mass = z_p4.M()
        h_z_mass.Fill(z_mass)

        recoil_mass = (e_cms - z_p4).M()
        h_recoil_mass.Fill(recoil_mass)

    hist_file = ROOT.TFile("higgs_recoil_hists.root", "recreate")
    h_z_mass.Write()
    h_recoil_mass.Write()
    h_cutflow.Write()
    hist_file.Close()


if __name__ == "__main__":
    main()
