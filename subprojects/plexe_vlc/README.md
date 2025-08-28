# Plexe VLC subproject

This Plexe subproject couples the framework with Veins VLC.
The following instructions describe how to compile the framework and how to run the included scenarios.

## Pre-requisites
To build and run the examples you will need OMNeT++ and SUMO as core software.
For one specific example (intersection merge) you will also need Matlab.
The required versions are:
1. OMNeT++: v6.2.0 (Installation guide: [here](https://doc.omnetpp.org/q/InstallGuide.pdf))
2. SUMO: v1.20 (v1.21 and v1.22 should also work) (Installation guide: [here](https://sumo.dlr.de/docs/Installing/index.html))

The OMNeT++ libraries required for Plexe VLC are Veins, Veins VLC, and Plexe.
The detailed instructions about cloning and compiling each library is explained below.

**NOTE:** The directory structure before compiling libraries should be as follows:
```
<root folder>
  |-- omnetpp-6.2.0
  |-- veins
  |-- veins_vlc
  |-- plexe
```

## Instructions for compiling the libraries
Before compiling the libraries, you need to first setup the OMNeT++ environment.
Enter the OMNeT++ root directory and setup the environment with
```bash
cd omnetpp-6.2.0/
source setenv
```
You can now proceed to the instructions for each of the libraries.

### 1. Veins

You can clone the repository from [here](https://github.com/sommer/veins/).
Be sure to checkout use version `5.3.1`.
Enter the veins directory and setup the environment:
```bash
git clone https://github.com/sommer/veins/
cd veins/
git checkout -b veins_v5.3.1 veins-5.3.1
source setenv
```
Generate the `Makefile` and compile the software:
```bash
./configure
make -j <ncores>
```

### 2. Veins VLC

You can clone the repository from [here](https://github.com:michele-segata/veins_vlc)
Enter the Veins VLC directory and setup the environment:
```bash
git clone https://github.com/michele-segata/veins_vlc.git
cd veins_vlc/
./configure
make -j <ncores>
```

### 3. Plexe

You can clone the repository from the [here](https://github.com/michele-segata/plexe), choosing version `3.2`.
```bash
git clone https://github.com/michele-segata/plexe
cd plexe/
git checkout -b plexe_v3.2 plexe-3.2
./configure
make -j <ncores>
```
Once compiled, you will also have to compile the `plexe_vlc` subproject.
```bash
cd subprojects/plexe_vlc
./configure
make -j <ncores>
```

## Running the example

Under the `examples/plexe_vlc` folder of the `plexe_vlc` subproject, you find a simple platooning simulation where messages are exchanged using VLC.
You can run it with
```bash
./run -u Cmdenv -c Sinusoidal -r 0
```
