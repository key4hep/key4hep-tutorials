# Generate Gaudi Functionals

In this tutorial we use `generateFunctional`, a small command-line helper shipped
with [`k4FWCore`](https://github.com/key4hep/k4FWCore), to generate framework-correct
C++ scaffolding for Gaudi Functional algorithms. A Functional algorithm needs several
pieces to agree with each other: the framework base class, the C++ input and output
types, the ordering of the constructor `KeyValue`s, the `operator()` signature and
return type, the EDM4hep/podio includes and the component declaration. `generateFunctional`
writes all of this from a compact description of the collections and properties, so that
you can focus on the physics logic in `operator()`.

The generator does not replace understanding the framework, and it does not write the
algorithm for you: you still supply the collection types and names, and you still
implement the event loop. What it gives you is a clean, compilable starting point.

## Setup

The tutorial needs a Key4hep environment that contains `generateFunctional` (k4FWCore
with [PR #372](https://github.com/key4hep/k4FWCore/pull/372) or later). On any machine
with CVMFS, source a recent stack:

```bash
source /cvmfs/sw.hsf.org/key4hep/setup.sh
```

Once k4FWCore is installed, the command is on your `PATH`:

```bash
generateFunctional --help
```

From a source checkout it can also be run directly (it is executable) or through Python:

```bash
cd k4FWCore/helpers
./generateFunctional --help            # executable
```

Plain Python execution requires `jinja2`; running through `uv` (`uv run --script generateFunctional ...`)
is an optional alternative that provisions it automatically.

# Choosing a Functional

The kind of Functional you need is determined by how many collections it reads and
writes. `generateFunctional` infers it from the number of `--inputs` and `--outputs`:

| Inputs      | Outputs           | Generated Functional |
|-------------|-------------------|----------------------|
| one or more | none              | `Consumer`           |
| none        | one or more       | `Producer`           |
| one or more | one               | `Transformer`        |
| one or more | two or more       | `MultiTransformer`   |
| one or more | a Boolean decision | `FilterPredicate`   |

Only the filter must be requested explicitly, because it cannot be told apart from a
consumer by counting collections:

```bash
generateFunctional MyFilter filter \
  -i 'edm4hep::MCParticleCollection:MCParticles'
```

A `Producer` with more than one output stays a `Producer` and returns a `std::tuple`;
a `Transformer` with more than one output is promoted to a `MultiTransformer`.

# Describing collections

Each input and output is a `TYPE:KEY` pair:

```
C++CollectionType:SteeringPropertyName
```

* `TYPE` becomes the C++ collection type (templated types such as
  `podio::UserDataCollection<float>` are supported).
* `KEY` becomes the configurable collection property that you set in the Python steering
  file. It is optional; if omitted the generator derives one from the type by stripping
  the namespace and the `Collection` suffix.

```bash
-o 'edm4hep::MCParticleCollection'
# default key: MCParticles
```

Gaudi properties use a similar `TYPE:NAME:DEFAULT[:DESCRIPTION]` form:

```bash
-p 'double:NoiseMean:0.0:Gaussian noise mean in GeV' \
   'double:NoiseSigma:0.001:Gaussian noise width in GeV'
```

# A first Consumer

A `Consumer` reads one or more collections and produces nothing — ideal for monitoring,
validation, or filling histograms. Generate one that reads calorimeter simulation hits:

```bash
generateFunctional EventStats \
  -i 'edm4hep::SimCalorimeterHitCollection:SimCaloHits' \
  -p 'double:EnergyThreshold:0.0:Minimum hit energy in GeV'
```

With one input and no output the generator infers a `Consumer`:

```cpp
struct EventStats final : k4FWCore::Consumer<void(const edm4hep::SimCalorimeterHitCollection&)> {
  EventStats(const std::string& name, ISvcLocator* svcLoc)
      : Consumer(name, svcLoc, KeyValue("SimCaloHits", "SimCaloHits")) {}

  void operator()(const edm4hep::SimCalorimeterHitCollection& SimCaloHits) const override {
    debug() << "Received SimCaloHits with " << SimCaloHits.size() << " elements" << endmsg;
    for (const auto& elem : SimCaloHits) {
      // TODO: process elem
    }
  }

  Gaudi::Property<double> m_energyThreshold{
      this, "EnergyThreshold", 0.0, "Minimum hit energy in GeV"};
};
```

# A Transformer with properties

A `Transformer` reads collections and returns exactly one. Generate the scaffold for a
simple calorimeter digitizer, place its properties under `private:`, and also emit a
standalone `CMakeLists.txt`:

```bash
generateFunctional RandomNoiseDigitizer \
  -i 'edm4hep::SimCalorimeterHitCollection:InputCollection' \
  -o 'edm4hep::CalorimeterHitCollection:OutputCollection' \
  -p 'double:NoiseMean:0.0:Gaussian noise mean in GeV' \
     'double:NoiseSigma:0.001:Gaussian noise width in GeV' \
  --private-properties \
  --cmake
```

This writes `RandomNoiseDigitizer.cpp` and `CMakeLists.txt`.

:::{dropdown} The generated `RandomNoiseDigitizer.cpp`
```cpp
#include "k4FWCore/Transformer.h"
#include "Gaudi/Property.h"
#include "edm4hep/CalorimeterHitCollection.h"
#include "edm4hep/SimCalorimeterHitCollection.h"
#include <string>

struct RandomNoiseDigitizer final
    : k4FWCore::Transformer<edm4hep::CalorimeterHitCollection(const edm4hep::SimCalorimeterHitCollection&)> {

  RandomNoiseDigitizer(const std::string& name, ISvcLocator* svcLoc)
      : Transformer(name, svcLoc,
                    {KeyValue("InputCollection", "InputCollection")},
                    {KeyValue("OutputCollection", "OutputCollection")}) {}

  edm4hep::CalorimeterHitCollection operator()(
      const edm4hep::SimCalorimeterHitCollection& InputCollection) const override {
    // TODO: implement
    return edm4hep::CalorimeterHitCollection{};
  }

private:
  Gaudi::Property<double> m_noiseMean{
      this, "NoiseMean", 0.0, "Gaussian noise mean in GeV"};
  Gaudi::Property<double> m_noiseSigma{
      this, "NoiseSigma", 0.001, "Gaussian noise width in GeV"};
};

DECLARE_COMPONENT(RandomNoiseDigitizer)
```
:::

The generated class is a valid scaffold: correct includes, the right base-class
signature, ordered constructor `KeyValue`s, a type-safe `operator()`, the requested
`Gaudi::Property` members, and the `DECLARE_COMPONENT` line that lets Gaudi load the
algorithm. The one part you are expected to edit is `operator()`.

# Implementing `operator()`

Replace the generated `TODO` with the algorithm logic. `operator()` runs once per event
and is `const` to support thread-safe scheduling, so avoid mutable event state in
members.

```cpp
edm4hep::CalorimeterHitCollection operator()(
    const edm4hep::SimCalorimeterHitCollection& input) const override {
  edm4hep::CalorimeterHitCollection output;

  for (const auto& simHit : input) {
    auto digiHit = output.create();

    // Exercise: draw Gaussian noise from the Key4hep/Gaudi random-number service.
    const double noise = 0.0;

    digiHit.setCellID(simHit.getCellID());
    digiHit.setEnergy(simHit.getEnergy() + noise);
  }

  return output;
}
```

# Configuring it in a steering file

The `KEY` values you passed on the command line become the configurable collection
properties. A minimal steering file for the digitizer:

```python
from Gaudi.Configuration import INFO
from Configurables import RandomNoiseDigitizer, EventDataSvc
from k4FWCore import ApplicationMgr, IOSvc

svc = IOSvc("IOSvc")
svc.Input = "sim_edm4hep.root"
svc.Output = "digi_edm4hep.root"

digitizer = RandomNoiseDigitizer(
    "RandomNoiseDigitizer",
    InputCollection="ECalBarrelCollection",
    OutputCollection="ECalBarrelDigiHits",
)
digitizer.NoiseMean = 0.0
digitizer.NoiseSigma = 0.001

ApplicationMgr(
    TopAlg=[digitizer],
    EvtSel="NONE",
    EvtMax=10,
    ExtSvc=[EventDataSvc("EventDataSvc")],
    OutputLevel=INFO,
)
```

# Building and running

For the standalone scaffold created with `--cmake`:

```bash
cmake -S . -B build
cmake --build build -j
```

To add the algorithm to an existing Key4hep package instead, move the generated `.cpp`
into the package's component source directory, add it to the package's
`gaudi_add_module(...)` sources, and rebuild. Then run the steering file:

```bash
k4run runRandomNoiseDigitizer.py
```

# Multiple outputs

Give a transformer more than one output and the generator produces a `MultiTransformer`
that returns a tuple:

```bash
generateFunctional SplitParticles \
  -i 'edm4hep::MCParticleCollection:MCParticles' \
  -o 'edm4hep::MCParticleCollection:ChargedParticles' \
     'edm4hep::MCParticleCollection:NeutralParticles'
```

The return type is emitted as a `retType` alias and the stub returns placeholder
collections that you rename as you implement:

```cpp
using retType = std::tuple<
    edm4hep::MCParticleCollection,
    edm4hep::MCParticleCollection>;

// ... inside operator() ...
auto output1 = edm4hep::MCParticleCollection();
auto output2 = edm4hep::MCParticleCollection();

// TODO: fill output collections

return std::make_tuple(std::move(output1), std::move(output2)); // NOLINT
```

The ordering of output types, constructor keys, and returned collections must stay
consistent.

# Variable-length (runtime) collections

k4FWCore Functionals can take a variable number of input collections. Promote an input
to a `KeyValues` vector with `--runtime-inputs`:

```bash
generateFunctional MergeHits \
  -i 'edm4hep::MCParticleCollection:Inputs' \
  --runtime-inputs 'edm4hep::MCParticleCollection:Inputs:MCParticles0,MCParticles1'
```

The input then arrives as a `std::vector<const edm4hep::MCParticleCollection*>&`, with a
`KeyValues("Inputs", {"MCParticles0", "MCParticles1"})` in the constructor.

Fixed and variable-length inputs can be mixed in the same algorithm — give them distinct
keys, listing only the variable-length one in `--runtime-inputs`:

```bash
generateFunctional MixInputs \
  -i 'edm4hep::MCParticleCollection:Fixed' 'edm4hep::TrackCollection:Tracks' \
  --runtime-inputs 'edm4hep::TrackCollection:Tracks:t0,t1' \
  -o 'edm4hep::MCParticleCollection:Output'
# operator()(const edm4hep::MCParticleCollection& Fixed,
#            const std::vector<const edm4hep::TrackCollection*>& Tracks)
```

Use `--runtime-outputs TYPE` for a variable-length vector of outputs. Runtime collections
are a k4FWCore-specific feature.

# Useful options

| Option                  | Purpose                                                     |
|-------------------------|-------------------------------------------------------------|
| `--cmake`               | also generate a minimal `CMakeLists.txt`                    |
| `--private-properties`  | place properties under `private:`                           |
| `--type-aliases`        | emit `using XxxColl = ...;` aliases for input types          |
| `--namespace NAME`      | wrap the class in a C++ namespace                           |
| `--use-class`           | generate `class` instead of `struct`                        |
| `--framework gaudi`     | target native `Gaudi::Functional` instead of k4FWCore      |
| `--event-context`       | add `const EventContext&` and a `finalize()` scaffold       |
| `--runtime-inputs`      | promote named inputs to variable-length `KeyValues`         |
| `--runtime-outputs`     | generate a variable-length vector of outputs               |
| `--keyvalues-inputs`    | promote specific inputs to `KeyValues` vectors              |
| `--all-keyvalues`       | treat every input as a `KeyValues` vector                  |
| `-f`, `--output-file`   | write to a specific path (default `<ClassName>.cpp`)        |
| `--force`               | overwrite an existing generated file                        |

# Safe generation

By default the generator refuses to clobber an existing file:

```console
Refusing to overwrite existing C++ source at 'RandomNoiseDigitizer.cpp'.
  Re-run with --force (or remove the file) if you really want to replace it.
```

Once you have edited a generated file, treat it as ordinary C++ and edit it directly;
only regenerate when the command line is the source of truth.

# Exercises

**1. Event-statistics Consumer.** Generate `EventStats` with a
`edm4hep::SimCalorimeterHitCollection` input keyed `SimCaloHits` and an `EnergyThreshold`
property. Implement it to count hits above threshold, sum their energy, and print one
`info()` message per event.

**2. Random-noise Transformer.** Generate `RandomNoiseDigitizer` as above, complete
`operator()` (draw Gaussian noise from the Gaudi random-number service), add it to a
steering file, build, and run ten events.

**3. Splitter (challenge).** Generate a Functional with one `MCParticleCollection` input,
two `MCParticleCollection` outputs, a `MinMomentum` property, and `--type-aliases
--private-properties`. Split the input into particles with `p >= MinMomentum` and the
rest, and check that the order of the returned tuple matches the order of the `-o`
options.

# References

- k4FWCore pull request [#372](https://github.com/key4hep/k4FWCore/pull/372) — the Gaudi
  Functional C++ class generator.
- [`gaudi_alg_higgs`](https://github.com/key4hep/key4hep-tutorials/blob/main/gaudi_alg_higgs/README.md)
  — writing a Gaudi algorithm by hand, for comparison.
- Key4hep documentation: [Writing Gaudi (Functional) algorithms](https://key4hep.github.io/key4hep-doc/).
