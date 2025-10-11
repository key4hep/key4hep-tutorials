#!/usr/bin/env python3

import ROOT
import os

ROOT.gROOT.SetBatch()


def get_all_histograms(rootfile):
    """Retrieve all TH1 histograms from the root level of a ROOT file."""
    histograms = []

    for key in rootfile.GetListOfKeys():
        obj = key.ReadObj()

        if obj.IsA().InheritsFrom("TH1"):
            histograms.append((key.GetName(), obj))

    return histograms


def main(args):
    inputfile = ROOT.TFile.Open(args.inputfile)
    histograms = get_all_histograms(inputfile)

    os.makedirs(args.outdir, exist_ok=True)

    canvas = ROOT.TCanvas("canvas", "canvas", 800, 600)
    for hist_name, hist in histograms:
        hist.Draw()
        canvas.SaveAs(os.path.join(args.outdir, f"{hist_name}.pdf"))


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(
        description="Small script ot simply create pdfs of all histograms in a ROOT file"
    )
    parser.add_argument("inputfile", help="The inputfile containing histograms")
    parser.add_argument(
        "-o",
        "--outdir",
        help="The output directory into which the plots should be placed",
        default=".",
    )

    args = parser.parse_args()
    main(args)
