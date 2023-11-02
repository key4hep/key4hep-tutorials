# Key4hep tutorials

This repository contains instructions and documentation as well as potentially
necessary (code) resources for several Key4hep tutorials. Each tutorial is aimed
at being as standalone as possible, i.e., it should be possible to simply start
any tutorial without having done any other before. Each tutorial is in its own
subfolder in this repository.

## Available tutorials

- [`introduction_key4hep`](https://github.com/key4hep/key4hep-tutorials/blob/main/introduction_key4hep/README.md) Prerequisites, basic introduction and links to further documentation.

- [`whizard_gen`](https://github.com/key4hep/key4hep-tutorials/blob/main/whizard_gen/README.md) - A tutorial that shows a simple example
  for generating events for the process $e^+e^- \rightarrow ZH$ with $Z\rightarrow \mu^+ \mu^-$
  for the ILC at $E_{cms}=250$ GeV.

- [`gaudi_ild_reco`](https://github.com/key4hep/key4hep-tutorials/blob/main/gaudi_ild_reco/README.md) - A tutorial that shows the
  full chain from simulation to reconstruction with the Gaudi based Key4hep
  framework using the ILD as example.

- [`gaudi_alg_higgs`](https://github.com/key4hep/key4hep-tutorials/blob/main/gaudi_alg_higgs/README.md) -
  A tutorial that shows how to create an algorithm to run with Gaudi to obtain
  the mass of the Higgs in a Higgs-recoil process.

- [`edm4hep_analylsis`](https://github.com/key4hep/key4hep-tutorials/blob/main/edm4hep_analysis/README.md) -
  Contains a small exercise that makes you use the basic components from podio
  and EDM4hep for writing small plotting scripts

## The instructions look a bit weird
The instructions are in markdown (`md`) format, but they might contain `sphinx`
specific tags since some of them are also included in the [key4hep
website](https://key4hep.github.io/key4hep-doc/tutorials/README.html), where these allow for
a nicer rendering of additional information or the usage of more advanced
documentation features. These tags usually start with at least three colons
(':') and the most common one is probably

```
:::{dropdown} Some more text
```
