In this tutorial, we will be calculating the recoil mass 
```math
M^2_{\text{recoil}} = M^2_H = s + M^2_Z - 2 E_Z \sqrt{s}
```
in
```math
e^+ e^- \rightarrow Z^* \rightarrow ZH \rightarrow H\ell^+ \ell^-
```
events at ILC. 

Before starting off, let's have a look at [the introduction to EDM4hep and podio](./edm4hep_api_intro.md)

## Options:

1. [Docker](#docker)
2. [NAF with Singularity](#naf )


## Docker in your local <a name="docker"></a>
Follow the [key4hep installation](../key4hep_installation.md); no need for x11. You go to the repository and start the juypter-lab

```bash
cd /home/workarea/tutorials
jupyter lab  --port=8888 --ip=0.0.0.0 --no-browser --allow-root
```

You should see 
```console
[I 2022-11-21 14:16:10.398 ServerApp] jupyterlab | extension was successfully loaded.
[I 2022-11-21 14:16:10.400 ServerApp] Serving notebooks from local directory: /home/workarea
[I 2022-11-21 14:16:10.400 ServerApp] Jupyter Server 1.13.5 is running at:
[I 2022-11-21 14:16:10.400 ServerApp] http://5f9c5335290e:8888/lab?token=13992675b1bd934412e6ce6c19c905df30a10dde39784a4f
[I 2022-11-21 14:16:10.401 ServerApp]  or http://127.0.0.1:8888/lab?token=13992675b1bd934412e6ce6c19c905df30a10dde39784a4f
```

copy paste `http://127.0.0.1:8888/lab?token=XXXXXX`into your browser. Then you should see this:

![](../images/jlab.png)

On the left panel, double click `edm4hep_python.ipynb`

## NAF with singularity <a name="naf"></a>

First you need to tunnel to DESY network via `bastion`. Sett the official DESY-IT page [here](https://it.desy.de/services/uco/documentation/external_access/index_eng.html) for more information, in short:

```bash
ssh -fN -D 8090 your-name@bastion.desy.de
```
Then go to your browser and setup a network proxy for it to go through this tunnel.

<details>
<summary>Click here for Firefox instructions</summary>

- Go to settings
- On the `General` page scroll to the bottom to `Network Settings`
- Click on the `Settings` button
- Choose `Manual proxy configuration`
  - In the `SOCKS Host` field enter `localhost`, use the `Port` number you specified in the ssh command
  - Choose `SOCKS v5`
  
![](../images/firefox_proxy.png)
</details>

<details>
<summary>Click here for Chrome instructions</summary>

Chrome requires you to change these settings for your complete system. The
easiest way to get there is in Chrome. These instructions here have been tested
with Ubuntu 20.04
- Go to `Setings` (top right corner, 3 vertical dots, about 2/3 down on the drop down menu)
- Select `System`
- Click on `Open your computer's proxy settings`
  - Depending on you operating system, there should be someting like `Network Proxy`
  - Configure that to `Manual`
  - Choose the `Socks Host`, enter `localhost` and number for the port from the ssh command

![](../images/proxy_system_ub2004.png)

</details>

you can go to any `naf-ilc` with your workarea and checkout the repo.

```bash
cd /nfs/dust/<your-workarea>
git clone https://gitlab.desy.de/ftx-sft-key4hep/tutorials.git
```
Now run our container with `singularity`. (This will take some time just only once!!)
```bash
singularity shell -H $PWD --bind /cvmfs/ilc.desy.de:/cvmfs/ilc.desy.de /nfs/dust/ilc/user/eren/container.repo.d/
```
We are ready for Juypter
```bash
source /cvmfs/ilc.desy.de/key4hep/setup.sh
jupyter lab  --port=8888 --ip=0.0.0.0 --no-browser --allow-root
```

copy paste `naf-ilc11.desy.de:8888/lab?token=77cd65XXXXX`from your console output into your browser.


