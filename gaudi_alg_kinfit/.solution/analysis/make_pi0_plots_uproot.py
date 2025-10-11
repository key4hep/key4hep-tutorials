#!/usr/bin/env python3

import uproot
import awkward as ak
import numpy as np
import matplotlib.pyplot as plt


def bname(coll, member):
    """Construct the full branch name that uproot expects for accessing a member
    of a collection, e.g.

    >>> bname("MCParticles", "momentum.x")
    >>> "MCParticles/MCParticles.momentum.x"
    """
    return f"{coll}/{coll}.{member}"


def mass(px, py, pz, e):
    """Calculate the mass of the particle described by the 4 momentum inputs"""
    return np.sqrt(ak.flatten(e**2 - px**2 - py**2 - pz**2))


def create_histogram(data, bins, range_tuple, xlabel, ylabel, title, filename):
    """Create and save a histogram with the given parameters"""
    fig, ax = plt.subplots(figsize=(8, 6))
    ax.hist(data, bins=bins, range=range_tuple, alpha=0.7)
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    ax.set_title(title)
    plt.tight_layout()
    plt.savefig(filename)


def main(args):
    inputfile = uproot.open(args.inputfile)
    events = inputfile["events"]
    # We have to know that Pi0s_New is a subset collection and how the branch
    # name is constructed for that. In this case we know that the collectionID
    # will always be the same, so we can get only the index. We still need to
    # know which collection actually houses the data though. In this case we can
    # cheat a bit because we know that its the GammaGammaCandidates_Pi0_New, but
    # that might not be always possible
    pi0_idcs = events[bname("Pi0s_New_objIdx", "index")].array()
    pi0_cand_e = events[bname("GammaGammaCandidates_Pi0_New", "energy")].array()
    pi0_cand_px = events[bname("GammaGammaCandidates_Pi0_New", "momentum.x")].array()
    pi0_cand_py = events[bname("GammaGammaCandidates_Pi0_New", "momentum.y")].array()
    pi0_cand_pz = events[bname("GammaGammaCandidates_Pi0_New", "momentum.z")].array()

    pi0_mass = ak.flatten(
        events[bname("GammaGammaCandidates_Pi0_New", "mass")].array()[pi0_idcs]
    )
    pi0_mass_p4 = mass(
        pi0_cand_px[pi0_idcs],
        pi0_cand_py[pi0_idcs],
        pi0_cand_pz[pi0_idcs],
        pi0_cand_e[pi0_idcs],
    )

    # For getting the prefit mass we have to get the photons first. Here we have
    # to do similar work as above. We have to also now even more about how
    # subset collections work internally. We create a FilteredPhotons subset
    # collection that points into the PandoraPFOs. The indices of the photons
    # attached to the pi0 candidates will still be pointing into the
    # PandoraPFOs, because objects in subset collections don't get a new index
    # (nor collectionID).
    #
    # Addtionally we have to know that the indices for related particles are
    # stored in branches of the form _<collection_name>_<relation_name>
    gamma_idcs = events[
        bname("_GammaGammaCandidates_Pi0_New_particles", "index")
    ].array()
    # Finally, we need to know how the individual indices are flattened when
    # written, because we need to grab the photons from the correct position
    gamma_starts = events[
        bname("GammaGammaCandidates_Pi0_New", "particles_begin")
    ].array()
    # Now we are in a position where we can
    # - select the correct start position for the actually selected pi0
    #   candidates
    # - and then select the first and the second photon thereafter
    g1_idcs = gamma_idcs[gamma_starts[pi0_idcs]]
    g2_idcs = gamma_idcs[gamma_starts[pi0_idcs] + 1]

    pfo_e = events[bname("PandoraPFOs", "energy")].array()
    pfo_px = events[bname("PandoraPFOs", "momentum.x")].array()
    pfo_py = events[bname("PandoraPFOs", "momentum.y")].array()
    pfo_pz = events[bname("PandoraPFOs", "momentum.z")].array()

    pi0_mass_prefit = mass(
        pfo_px[g1_idcs] + pfo_px[g2_idcs],
        pfo_py[g1_idcs] + pfo_py[g2_idcs],
        pfo_pz[g1_idcs] + pfo_pz[g2_idcs],
        pfo_e[g1_idcs] + pfo_e[g2_idcs],
    )
    fit_delta_m = pi0_mass_p4 - pi0_mass_prefit

    # # Create histograms
    create_histogram(
        pi0_mass,
        100,
        (0.130, 0.139),
        "Mass [GeV]",
        "Entries",
        r"$M_{\pi^{0}}$",
        "pi0_mass.pdf",
    )
    create_histogram(
        pi0_mass_p4,
        100,
        (0.130, 0.139),
        "Mass [GeV]",
        "Entries",
        r"$M_{\gamma\gamma}$",
        "pi0_mass_p4.pdf",
    )
    create_histogram(
        pi0_mass_prefit,
        100,
        (0.1, 0.19),
        "Mass [GeV]",
        "Entries",
        r"$M_{\gamma\gamma}$ (postfit)",
        "pi0_mass_prefit.pdf",
    )
    create_histogram(
        fit_delta_m,
        100,
        (-0.1, 0.1),
        "Mass Difference [GeV]",
        "Entries",
        r"$M_{\gamma\gamma}$ (postfit) - $M_{\gamma\gamma}$ (prefit)",
        "fit_delta_m.pdf",
    )


if __name__ == "__main__":
    import argparse

    parser = argparse.ArgumentParser(
        description="Script to make histograms using uproot and other python tools"
    )
    parser.add_argument(
        "inputfile", help="The input file from which histograms should be produced"
    )

    args = parser.parse_args()
    main(args)
