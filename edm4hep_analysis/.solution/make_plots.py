#!/usr/bin/env python3

import glob

from podio import root_io
from edm4hep import utils, LorentzVectorE

import ROOT


def main():
    files = glob.glob("higgs_recoil_from_gaudi_*.edm4hep.root")
    reader = root_io.Reader(files)

    e_cms = LorentzVectorE(0.0, 0.0, 0.0, 250.0)

    h_z_mass = ROOT.TH1D("z_mass", ";Mass / GeV;Entries", 240, 60.0, 120.0)
    h_recoil_mass = ROOT.TH1D("recoil_mass", ";Mass / GeV;Entries", 380, 60.0, 250.0)

    for event in reader.get("events"):
        muons = event.get("Muons")
        if len(muons) != 2:
            continue

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
    hist_file.Close()


if __name__ == "__main__":
    main()
