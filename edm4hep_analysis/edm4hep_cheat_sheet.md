# Cheat Sheet: Reading EDM4hep

This section compares:
 - analysis of EDM4hep file with podio reader
 - analysis of EDM4hep file with plain ROOT or uproot
 - analysis of legacy LCIO file with LCReader

## Event loop

::::::{tab-set}

:::::{tab-item} C++

::::{tab-set}

:::{tab-item} EDM4hep - podio
```cpp
// TODO: write me
```
:::

:::{tab-item} EDM4hep - ROOT
```cpp
// TODO: write me
```
:::

:::{tab-item} LCIO (legacy)
```cpp
// TODO: write me
```
:::
::::

:::::

:::::{tab-item} Python

::::{tab-set}
:::{tab-item} EDM4hep - podio
```python
from podio import root_io

reader = root_io.Reader("my_file.edm4hep.root")

for event in reader.get("events"):
    # do more stuff with this event
```
:::

:::{tab-item} EDM4hep - ROOT
```python
import ROOT

file = ROOT.TFile("my_file.edm4hep.root")

for event in file["events"]:
    # do more stuff with this event```
:::

:::{tab-item} LCIO (legacy)
```python
import pyLCIO

reader = pyLCIO.IOIMPL.LCFactory.getInstance().createLCReader()
reader.open("my_file.slcio")

for event in reader:
    # do more stuff with this event
```
:::
::::

:::::
::::::

## Event metadata

::::::{tab-set}
:::::{tab-item} C++
::::{tab-set}
:::{tab-item} EDM4hep - podio
```cpp
// TODO: write me
```
:::

:::{tab-item} EDM4hep - ROOT
```cpp
// TODO: write me
```
:::

:::{tab-item} LCIO (legacy)
```cpp
// TODO: write me
```
:::
::::
:::::
:::::{tab-item} Python
::::{tab-set}
:::{tab-item} EDM4hep - podio
```python
for event in reader.get("events"):
    event_header = event.get("EventHeader")[0]

    print( event_header.getRunNumber() )
    print( event_header.getEventNumber() )
    print( event_header.getTimeStamp() )

    for par in event.parameters:
        print( par, event.get_parameter(par) )
```
:::
:::{tab-item} EDM4hep - ROOT
```python
for event in file["events"]:
    event_header = event.EventHeader[0]

    print(event_header.runNumber)
    print(event_header.eventNumber)
    print(event_header.timeStamp)

    # TODO: podio::Frame parameters if possible
:::
:::{tab-item} LCIO (legacy)
```python
for event in reader:
    print( event.getRunNumber() )
    print( event.getEventNumber() )
    print( event.getTimeStamp() )

    pars = event.getParameters()

    for key in pars.getIntKeys(pyLCIO.EVENT.StringVec()):
        print( key, pars.getIntVal(key) )

    for key in pars.getFloatKeys(pyLCIO.EVENT.StringVec()):
        print( key, pars.getFloatVal(key) )

    for key in pars.getDoubleKeys(pyLCIO.EVENT.StringVec()):
        print( key, pars.getDoubleVal(key) )

    for key in pars.getStringKeys(pyLCIO.EVENT.StringVec()):
        print( key, pars.getStringVal(key) )

```
:::
::::
:::::
::::::


## Working with collections

::::::{tab-set}
:::::{tab-item} C++
::::{tab-set}
:::{tab-item} EDM4hep - podio
```cpp
// TODO: write me
```
:::

:::{tab-item} EDM4hep - ROOT
```cpp
// TODO: write me
```
:::

:::{tab-item} LCIO (legacy)
```cpp
// TODO: write me
```
:::
::::
:::::
:::::{tab-item} Python
::::{tab-set}
:::{tab-item} EDM4hep - podio
```python
for event in reader.get("events"):
    # list available collection names
    # the set of collections in EDM4hep is the same for all events
    print ( event.getAvailableCollections() )

    # get collections
    mc_col = event.get("MCParticles")
    pfo_col = event.get("PandoraPFOs")
    hit_col = event.get("EcalBarrelCollectionRec")

    # itirate over collection elements
    for mc in mc_col:
        if not mc.isAvailable():
            continue
        # do more stuff with your MCParticles
    
    for pfo in pfo_col:
        if not pfo.isAvailable():
            continue
        # do more stuff with your PFOs

    for hit in hit_col:
        if not hit.isAvailable():
            continue
        # do more stuff with your hits
```
:::
:::{tab-item} EDM4hep - ROOT
```python
# list available collection names
# the set of collections in EDM4hep is the same for all events
for entry in file["podio_metadata"]:
    for col_info in entry.events___CollectionTypeInfo:
        print(col_info.name)
# uproot version
for col_name in uproot_file.get("podio_metadata").arrays("events___CollectionTypeInfo.name")["events___CollectionTypeInfo.name"][0]:
    print(col_name)

for event in reader.get("events"):
    # get collections
    mc_col = event.MCParticles
    pfo_col = event.PandoraPFOs
    hit_col = event.EcalBarrelCollectionRec

    # itirate over collection elements
    for mc in mc_col:
        if not mc:
            continue
        # do more stuff with your MCParticles
    
    for pfo in pfo_col:
        if not pfo:
            continue
        # do more stuff with your PFOs

    for hit in hit_col:
        if not hit:
            continue
        # do more stuff with your hits

:::
:::{tab-item} LCIO (legacy)
```python
for event in reader:
    # list available collection names
    # set of collections in LCIO was not necessarily the same for all events
    print( event.getCollectionNames() )

    mc_col = event.getCollection("MCParticle")
    pfo_col = event.getCollection("PandoraPFOs")
    hit_col = event.getCollection("EcalBarrelCollectionRec")

    # itirate over collection elements
    for mc in mc_col:
        if not mc:
            continue
        # do more stuff with your MCParticles
    
    for pfo in pfo_col:
        if not pfo:
            continue
        # do more stuff with your PFOs

    for hit in hit_col:
        if not hit:
            continue
        # do more stuff with your hits
```
:::
::::
:::::
::::::

## Working with collection elements

:::{note}
All object properties can be checked in the corresponding class API docs:
[https://edm4hep.web.cern.ch/namespaceedm4hep.html](https://edm4hep.web.cern.ch/namespaceedm4hep.html)
:::

::::::{tab-set}
:::::{tab-item} C++
::::{tab-set}
:::{tab-item} EDM4hep - podio
```cpp
// TODO: write me
```
:::

:::{tab-item} EDM4hep - ROOT
```cpp
// TODO: write me
```
:::

:::{tab-item} LCIO (legacy)
```cpp
// TODO: write me
```
:::
::::
:::::
:::::{tab-item} Python
::::{tab-set}
:::{tab-item} EDM4hep - podio
```python
for event in reader.get("events"):
    for pfo in event.get("PandoraPFOs"):
        # storage properties
        print("element index inside the collection:", pfo.id().index)
        print("collection ID where element is stored:", pfo.id().collectionID)
        print("collection name where element is stored:", event.getName( pfo.id().collectionID ) )

        # physics properties
        print("E:", pfo.getEnergy())
        print("px", pfo.getMomentum().x)
        print("py", pfo.getMomentum().y)
        print("pz", pfo.getMomentum().z)

        # related objects
        for cluster in pfo.getClusters():
            # do something with your cluster
            for hit in cluster.getHits():
                # do something with your hit

```
:::
:::{tab-item} EDM4hep - ROOT
```python
# helper map to get the collection names of the related objects
col_id2name = {}
for entry in file["podio_metadata"]:
    for col_info in entry.events___CollectionTypeInfo:
        col_id2name[col_info.collectionID] = str(col_info.name)

for event in file["events"]:
    for pfo in event.PandoraPFOs:
        # storage properties
        # Not directly possible from the object.

        # physics properties
        print("E:", pfo.energy)
        print("px", pfo.momentum.x)
        print("py", pfo.momentum.y)
        print("pz", pfo.momentum.z)

        # related objects
        for cluster_it in range(pfo.clusters_begin, pfo.clusters_end):
            cluster_index = event._PandoraPFOs_clusters[cluster_it].index
            cluster_col_id = event._PandoraPFOs_clusters[cluster_it].collectionID
            cluster_col_name = col_id2name[cluster_col_id]

            cluster = getattr(event, cluster_col_name)[cluster_index]
            # do something with your cluster

            for hit_it in range(cluster.hits_begin, cluster.hits_end):
                hit_index = getattr(event, f"_{cluster_col_name}_hits")[hit_it].index
                hit_col_id = getattr(event, f"_{cluster_col_name}_hits")[hit_it].collectionID
                hit_col_name =  col_id2name[hit_col_id]

                hit = getattr(event, hit_col_name)[hit_index]
                # do something with your hit

:::
:::{tab-item} LCIO (legacy)
```python
for event in reader:
    for pfo in event.getCollection("PandoraPFOs"):
        # storage properties
        # Not directly possible from the object.

        # physics properties
        print("E:", pfo.getEnergy())
        print("px", pfo.getMomentum()[0])
        print("py", pfo.getMomentum()[1])
        print("pz", pfo.getMomentum()[2])

        # related objects
        for cluster in pfo.getClusters():
            # do something with your cluster

            for hit in cluster.getCalorimeterHits():
                # do something with your hit
```
:::
::::
:::::
::::::

## Working with links


::::::{tab-set}
:::::{tab-item} C++
::::{tab-set}
:::{tab-item} EDM4hep - podio
```cpp
// TODO: write me
```
:::

:::{tab-item} EDM4hep - ROOT
```cpp
// TODO: write me
```
:::

:::{tab-item} LCIO (legacy)
```cpp
// TODO: write me
```
:::
::::
:::::
:::::{tab-item} Python
::::{tab-set}
:::{tab-item} EDM4hep - podio
```python
for event in reader.get("events"):
    # itirating over link collections
    for link in event.get("RecoMCTruthLink"):
        print(f"RecoParticle {link.getFrom()} is linked to MCParticle {link.getTo()} with weight {link.getWeight()}")

    for link in event.get("EcalBarrelRelationsSimRec"):
        print(f"CaloHit {link.getFrom()} is linked to SimCaloHit {link.getTo()} with weight {link.getWeight()}")

    # extracting links for specific objects (manual search)
    for pfo in event.get("PandoraPFOs"):
        print(f"RecoParticle {pfo} is linked to: ")
        links = {}
        for link in event.get("RecoMCTruthLink"):
            if link.getFrom() != pfo:
                continue
            links[link.getTo()] = link.getWeight()
        for mc, weight in links.items():
            print(f" --- MCParticle {mc} with weight {weight}")


    # extracting links for specific objects (LinkNavigator)
    # TODO: needs a fix for mutable types https://github.com/AIDASoft/podio/issues/918

```
:::
:::{tab-item} EDM4hep - ROOT
```python
# helper map to get the collection names of the related objects
col_id2name = {}
for entry in file["podio_metadata"]:
    for col_info in entry.events___CollectionTypeInfo:
        col_id2name[col_info.collectionID] = str(col_info.name)

for event in file["events"]:
    # itirating over links
    for i, link in enumerate(event.RecoMCTruthLink):
        from_obj_col_id = event._RecoMCTruthLink_from[i].collectionID
        from_obj_index = event._RecoMCTruthLink_from[i].index
        from_obj = getattr(event, col_id2name[from_obj_col_id])[from_obj_index]

        to_obj_col_id = event._RecoMCTruthLink_from[i].collectionID
        to_obj_index = event._RecoMCTruthLink_from[i].index
        to_obj = getattr(event, col_id2name[to_obj_col_id])[to_obj_index]

        print(f"RecoParticle {from_obj} is linked to MCParticle {to_obj} with weight {link.weight}")

    for link in event.EcalBarrelRelationsSimRec:
        from_obj_col_id = event._EcalBarrelRelationsSimRec_from[i].collectionID
        from_obj_index = event._EcalBarrelRelationsSimRec_from[i].index
        from_obj = getattr(event, col_id2name[from_obj_col_id])[from_obj_index]

        to_obj_col_id = event._EcalBarrelRelationsSimRec_from[i].collectionID
        to_obj_index = event._EcalBarrelRelationsSimRec_from[i].index
        to_obj = getattr(event, col_id2name[to_obj_col_id])[to_obj_index]

        print(f"CaloHit {from_obj} is linked to SimCaloHit {to_obj} with weight {link.weight}")

    # extracting links for specific objects (manual search)
    for pfo in event.PandoraPFOs:
        print(f"RecoParticle {pfo} is linked to: ")
        links = {}
        for link in event.RecoMCTruthLink:
            from_obj_col_id = event._RecoMCTruthLink_from[i].collectionID
            from_obj_index = event._RecoMCTruthLink_from[i].index
            from_obj = getattr(event, col_id2name[from_obj_col_id])[from_obj_index]

            to_obj_col_id = event._RecoMCTruthLink_from[i].collectionID
            to_obj_index = event._RecoMCTruthLink_from[i].index
            to_obj = getattr(event, col_id2name[to_obj_col_id])[to_obj_index]

            if from_obj != pfo:
                continue
            links[to_obj] = link.weight
        for mc, weight in links.items():
            print(f" --- MCParticle {mc} with weight {weight}")


    # extracting links for specific objects (LinkNavigator)
    # TODO: needs a fix for mutable types https://github.com/AIDASoft/podio/issues/918

:::
:::{tab-item} LCIO (legacy)
```python
for event in reader:
    # itirating over links
    for link in event.getCollection("RecoMCTruthLink"):
        print(f"RecoParticle {link.getFrom()} is linked to MCParticle {link.getTo()} with weight {link.getWeight()}")

    for link in event.getCollection("EcalBarrelRelationsSimRec"):
        print(f"CaloHit {link.getFrom()} is linked to SimCaloHit {link.getTo()} with weight {link.getWeight()}")

    # extracting links for specific objects (manual search)
    for pfo in event.getCollection("PandoraPFOs"):
        print(f"RecoParticle {pfo} is linked to: ")
        links = {}
        for link in event.getCollection("RecoMCTruthLink"):
            if link.getFrom() != pfo:
                continue
            links[link.getTo()] = link.getWeight()
        for mc, weight in links.items():
            print(f" --- MCParticle {mc} with weight {weight}")

    # extracting links for specific objects (LCRelationNavigator)
    nav = pyLCIO.UTIL.LCRelationNavigator( event.getCollection("RecoMCTruthLink") )
    for pfo in event.getCollection("PandoraPFOs"):
        objects = nav.getRelatedToObjects(pfo)
        weights = nav.getRelatedToObjects(pfo)
        for mc, weight in zip(objects, weights):
            print(f" --- MCParticle {mc} with weight {weight}")


```
:::
::::
:::::
::::::

## Working with vertices

TBD