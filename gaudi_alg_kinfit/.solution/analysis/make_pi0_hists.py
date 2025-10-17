#!/usr/bin/env python3

import ROOT

from podio.reading import get_reader
from edm4hep import utils

p4 = utils.p4
UseEnergy = utils.UseEnergy


def main(args):
    """Main"""
    reader = get_reader(args.inputfile)
    events = reader.get("events")

    histfile = ROOT.TFile(args.outputfile, "recreate")
    pi0_mass = ROOT.TH1D("pi0_mass", ";M_{#pi^{0}};Entries", 100, 0.130, 0.139)
    pi0_mass_p4 = ROOT.TH1D("pi0_p4", ";M_{#gamma#gamma};Entries", 100, 0.130, 0.139)
    pi0_mass_prefit = ROOT.TH1D(
        "pi0_mass_prefit", ";M_{#gamma#gamma} (prefit);Entries", 100, 0.1, 0.19
    )
    fit_delta_m = ROOT.TH1D(
        "fit_delta_m",
        ";M_{#gamma#gamma} (postfit) - M_{#gamma#gamma} (prefit);Entries",
        100,
        -0.1,
        0.1,
    )

    for event in events:
        pi0s = event.get("Pi0s_New")
        for pi0 in pi0s:
            pi0_mass.Fill(pi0.getMass())
            pi0_p4 = p4(pi0, UseEnergy)
            pi0_mass_p4.Fill(pi0_p4.M())

            gamma1_p4 = p4(pi0.getParticles()[0], UseEnergy)
            gamma2_p4 = p4(pi0.getParticles()[1], UseEnergy)
            prefit_pi0 = gamma1_p4 + gamma2_p4
            pi0_mass_prefit.Fill(prefit_pi0.M())
            fit_delta_m.Fill(pi0_p4.M() - prefit_pi0.M())

    pi0_mass.Write()
    pi0_mass_p4.Write()
    pi0_mass_prefit.Write()
    fit_delta_m.Write()
    histfile.Close()


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(
        description="Small script to make some plots via python bindings and ROOT"
    )

    parser.add_argument("inputfile", help="The input file with the data")
    parser.add_argument(
        "outputfile",
        nargs="?",
        default="pi0_histograms.root",
        help="The output file into which the histograms will be stored",
    )

    args = parser.parse_args()
    main(args)
