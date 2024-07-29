# Running the WHIZARD event generator

`WHIZARD` is a matrix element generator that is and has been extensively used for 
linear collider physics studies.

For more information, documentation and more advanced tutorials see the `WHIZARD`-homepage at:
[https://whizard.hepforge.org](https://whizard.hepforge.org).

In this tutorial we will show you a very simple example for how to run `Whizard` to generate some
events for the process 
- $e^+e^- \rightarrow ZH$ with $Z\rightarrow \mu^+ \mu^-$ for the ILC at $E_{cms}=250$ GeV.

## Setup
If you haven't done it yet, source a Key4hep software environment via

```bash
source /cvmfs/sw.hsf.org/key4hep/setup.sh
```

and create a working directory for this exercise:

```bash
mkdir work_whizard
cd work_whizard
```

## Run the generator

Download the example steering file [./zhiggs.sin](./zhiggs.sin) or use wget:

```bash
wget https://raw.githubusercontent.com/key4hep/key4hep-tutorials/main/whizard_gen/zhiggs.sin
```
And then you can generate your first events with `WHIZARD`:

```bash
whizard zhiggs.sin
```

## Inspect the output file
If the above has worked there are many files created by `WHIZARD`. We are here only interested in the actual event output file: `zhiggs.slcio`.  This is an [LCIO](https://github.com/iLCSoft/LCIO) file that can be analysed with the usual tools, e.g. we can dump the record of the 3rd event:

```bash
dumpevent zhiggs.slcio 3 | less
```
**Exercise:** use the `dumpevent` tool to look at the MC-truth record.
 - Have we generated the right events for the process ($e^+e^- \rightarrow ZH$ with $Z\rightarrow \mu^+ \mu^-$) ?
 - Have we generated the correct center of mass energy ?

Answering the second question with `dumpevent` might be a bit cumbersome, though.

Take a look at this python script [./lcio_mcparticle.py](./lcio_mcparticle.py) to see how this could be answered in a programmatic way using the LCIO Python bindings.

```bash
python lcio_mcparticle.py zhiggs.slcio
```
   
## Modify the WHIZARD steering file

In the last exercise we have seen that the example steering file does not yet produce the output file that we actually want.
Now it is rather straight forward to fix:

**Exercise:** modify the `zhiggs.sin` file to actually create what we want:
 -  events for the process: $e^+e^- \rightarrow ZH$ with $Z\rightarrow \mu^+ \mu^-$
 -  a correct beam spectrum for the ILC at $E_{cms}=250$ GeV
 -  more than 10 events (e.g. 10k)
 -  output file name `zh_mumu.slcio`

Hints: `WHIZARD` uses its own language `SINDARIN` for the steering files and needs so called circe files for the beam energy spectrum - see [http://whizard.hepforge.org/manual](http://whizard.hepforge.org/manual) and [http://whizard.hepforge.org/circe_files](http://whizard.hepforge.org/circe_files).

If you are done you should be able to see sth. like this:

```bash
python lcio_mcparticle.py zh_mumu.slcio
#  Loading LCIO ROOT dictionaries ...
#  <E_cms> =  247.65331134473922   from  10000  events 
```



