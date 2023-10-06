# Running the WHIZARD event generator

`WHIZARD` is a matrix element generator that is and has been extensively used for 
linear collider physics studies.

For more information, documentation and more advanced tutorials see the `WHIZARD`-homepage at:
[https://whizard.hepforge.org](https://whizard.hepforge.org).

In this tutorial we will show you a very simple example for how to run `Whizard` to generate some
events for the process $e^+e^- \rightarrow ZH$ with $Z\rightarrow \mu^+ \mu^-$.

## Setup
If you haven't done it yet, source a Key4hep software environment via

```bash
source /cvmfs/sw-nightlies.hsf.org/key4hep/setup.sh
```

and create a working directory for this exercise:

```bash
mkdir whizard_work
cd whizard_work
```

## Run the generator

Download the example steering file [./zh_mumu0.sin](./zh_mumu0.sin) or use wget:

```bash
wget https://raw.githubusercontent.com/key4hep/key4hep-tutorials/main/whizard_gen/zh_mumu0.sin
```
And then you can generate your first 10 events with `WHIZARD`:

```bash
whizard zh_mumu0.sin
```



