# Plexe 5G subproject

This Plexe subproject couples the framework with Simu5G and implements some basic cooperative driving scenarios supported by 5G communication and MEC services.
The following instructions describe how to compile the framework and how to run the included scenarios.

## Pre-requisites
To build and run the examples you will need OMNeT++ and SUMO as core software.
For one specific example (intersection merge) you will also need Matlab.
The required versions are:
1. OMNeT++: v6.2.0 (Installation guide: [here](https://doc.omnetpp.org/q/InstallGuide.pdf))
2. SUMO: v1.20 (v1.21 and v1.22 should also work, but v1.23 is not currently supported by Veins 5.3.1) (Installation guide: [here](https://sumo.dlr.de/docs/Installing/index.html))
3. **Optional for some simulations:** Matlab (any version after R2020. Required for running the MEC intersection merge example)

The OMNeT++ libraries required for Plexe 5G are INET, Simu5G, Veins, and Plexe.
The detailed instructions about cloning and compiling each library is explained below.

**NOTE:** The directory structure before compiling libraries should be as follows:
```
<root folder>
  |-- omnetpp-6.2.0
  |-- veins
  |-- inet
  |-- simu5g
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

### 1. INET

You can clone INET 4.5.4 from the INET github repository [here](https://github.com/inet-framework/inet).
Once the repository is cloned, checkout the 4.5.4 tag as indicated below.

Enter the `inet` directory and setup the environment:
```bash
git clone https://github.com/inet-framework/inet
cd inet/
git checkout -b inet_v4.5.4 v4.5.4
source setenv
```
Generate the `Makefile` and compile in both `release` and `debug` modes:
```bash
make makefiles
make -j <ncores> MODE=release
make -j <ncores> MODE=debug
```
**NOTE**: `ncores` can be found out using `nproc` command.

### 2. Simu5G

You can clone the repository from [here](https://github.com/michele-segata/Simu5G/).
Be sure to checkout the `plexe_mec` branch.

Enter the `simu5g` directory and setup the environment:
```bash
git clone https://github.com/michele-segata/Simu5G/ simu5g
cd simu5g/
git checkout plexe_mec
source setenv
```
Generate the `Makefile` and compile the framework:
```bash
make makefiles
make -j <ncores> MODE=release
make -j <ncores> MODE=debug
```

### 3. Veins

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

You will also need to compile `veins_inet`:
```bash
cd subprojects/veins_inet/
./configure
make -j <ncores>
```

### 4. Plexe

You can clone the repository from the [here](https://github.com/michele-segata/plexe), choosing version `3.2`.
```bash
git clone https://github.com/michele-segata/plexe
cd plexe/
git checkout -b plexe_v3.2 plexe-3.2
source setenv
```
Then configure and compile `plexe` with:
```bash
./configure
make -j <ncores>
```
Once compiled, you will also have to compile the `plexe_5g` subproject.
```bash
cd subprojects/plexe_5g
```

If you do not have Matlab or you do not need it, you can simply run
```bash
./configure
make -j <ncores>
```
to compile Plexe 5G.
**Notice:** if you have named your frameworks folder differently from the standard, you might need to specify where they are located, e.g.:
```bash
./configure --with-INET=../../../inet4.5
```
Without using Matlab you will not be able to run the *Intersection merge* example, but you will be able to run the other 3 examples using 5G.
If you instead need Matlab, please look at the next section.

#### 4.1 Matlab configuration

In the root folder of the Plexe 5G subproject, open the `configure` file and change the path and version of Matlab:
```
# Add Matlab flags
matlab_path = "<path to MATLAB folder>"
check_fname = os.path.join(matlab_path, 'VersionInfo.xml')
expect_version = '<MatlabVersion>'
```
For Ubuntu and Matlab R2023b, the path and the expected version should look as follows:
```
# Add Matlab flags
matlab_path = "/usr/local/MATLAB/R2023b"
check_fname = os.path.join(matlab_path, 'VersionInfo.xml')
expect_version = 'R2023b'
```
For macOS and Matlab R2024b, it might look as follows instead:
```
# Add Matlab flags
matlab_path = "/Applications/MATLAB_R2024b.app"
check_fname = os.path.join(matlab_path, 'VersionInfo.xml')
expect_version = 'R2024b'
```
**Notice**: to enable compiling `plexe` in debug mode, the build system will search for the Matlab libraries named `libMatlabDataArray_dbg` and `libMatlabEngine_dbg`, which do not exist.
To avoid errors while compiling you can either compile in release mode online or make a copy of the Matlab release library and naming them as above.
For example, in macOS with ARM processors and Matlab R2024b, such libraries are located in `/Applications/MATLAB_R2024b.app/extern/bin/maca64`.
Simply do the following
```bash
cd /Applications/MATLAB_R2024b.app/extern/bin/maca64
cp libMatlabDataArray.dylib libMatlabDataArray_dbg.dylib
cp libMatlabEngine.dylib libMatlabEngine_dbg.dylib
```
This will not give you debug capabilities over Matlab, but enable to compile `plexe` in debug mode.
The procedure is similar for Ubuntu.
Once done, save the file and finally configure and compile the software:
```bash
WITH_MATLAB=1 ./configure
make -j <ncores>
```
**Notice:** if you have named your frameworks folder differently from the standard, you might need to specify where they are located, e.g.:
```bash
./configure --with-INET=../../../inet4.5
```

## Description of Plexe_5G and its example scenarios

In the `plexe/subprojects/plexe_5g` folder, you will find the Plexe_5G subproject, which incorporates 5G and 5G-NR for platooning applications.
This is achieved by federating two frameworks: Plexe and Simu5G.
Additionally, the subproject offers ETSI MEC architecture to support 5G and MEC-enabled cooperative driving applications.

### Scenarios included:

This subproject includes four examples inside the `examples` subfolder (path: `plexe/subprojects/plexe_5g/examples/`)

1. `platooning_5g`: This example ports the `platooning_lte` example which implements a traffic authority that aids two platoons to merge, but using 5G communication.
2. `platooning_mec`: This is same as the example above, however, the traffic authority is implemented as a MEC application.
3. `platooning_mec_overtake`: This example presents the scenario of an overtake maneuver, where a vehicle tries to overtake a platoon from behind. The overtake maneuver is implemented as a MEC application.
4. `platooning_mec_intersection`: This example presents a scenario of a T-shaped intersection, where a platoon traveling south-to-north tries to merge either ahead, in-between, or behind the two platoons traveling west-to-east. Here too, the proposed intersection merge maneuver is implemented as a MEC application.

### Running an example:

After setting up OMNeT++ and all the necessary software, follow the instructions below to run the scenarios.

#### 1. Platooning 5G

Enter the `examples/platooning_5g` folder.
Run the sample simulation with
```bash
./run -u Cmdenv -c CV2XMerge -r 0
```
You should see a ring like circuit in SUMO with two 4-car platoons.
After a while, the platoon behind will get close and merge with the platoon ahead aided by the traffic authority, which is implemented as an application running on an Internet server.

#### 2. Platooning MEC

Enter the `examples/platooning_mec` folder.
Run the sample simulation with
```bash
./run -u Cmdenv -c CV2XMerge -r 0
```
You should exactly the same scenario as in the previous one.
The only difference is that the traffic authority is implemented as a MEC application shared among the two platoon leaders.

#### 3. MEC Overtake Maneuver

After accessing the `plexe_mec_overtake` folder, you can run simulations with
```bash
./run -u Cmdenv -c Overtake -r <runNumber>
```
`runNumber` can go from 0 to 2099, investigating different set of parameters and repetitions.

The example also includes baseline simulations, which can be run with
```bash
./run -u Cmdenv -c Overtake -r <runNumber>
```
`runNumber` goes from 0 to 20.

#### 4. MEC Intersection Merge Maneuver

Before running this example you should open Matlab and run the `init.m` script located inside the `matlab` folder.
This creates a shared Matlab instance that can be invoked by Plexe to run the optimization script.

After accessing the `plexe_mec_intersection` folder, you can run the simulations therein with
```bash
./run -u Cmdenv -c IntersectionMerge -r <runNumber>
```
Run numbers go from 0 to 1439.
As for the overtake maneuver, the intersection scenario also includes set of baseline simulations, which can be run with
```bash
./run -u Cmdenv -c BaselineIntersectionMerge -r <runNumber>
```
with run numbers going from 0 to 143.

## Scientific Documentation

If you use Plexe and its 5G extension for your work, we would appreciate a citation:
* _M. Segata, A. Shastri, A. Piccin, I. G. Duta, and P. Casari,_ **"On the Impact of Communication and Computing Delays on MEC-Enabled Cooperative Maneuvers,"** in 16th IEEE Vehicular Networking Conference (VNC 2025), Porto, Portugal, Jun. 2025
* _M. Segata, R. Lo Cigno, T. Hardes, J. Heinovski, M. Schettler, B. Bloessl, C. Sommer, and F. Dressler,_ **"Multi-Technology Cooperative Driving: An Analysis Based on PLEXE,"** IEEE Transactions on Mobile Computing, vol. 22, no. 8, pp. 4792–4806, Aug. 2023

```bibtex
@inproceedings{segata2025impact,
   author = {Segata, Michele and Shastri, Anish and Piccin, Andrea and
Duta, Ioan Gabriel and Casari, Paolo},
   title = {{On the Impact of Communication and Computing Delays on
MEC-Enabled Cooperative Maneuvers}},
   publisher = {IEEE},
   address = {Porto, Portugal},
   booktitle = {16th IEEE Vehicular Networking Conference (VNC 2025)},
   month = {6},
   year = {2025}
}
@article{segata2023multi-technology,
  author = {Segata, Michele and Lo Cigno, Renato and Hardes, Tobias and Heinovski, Julian and Schettler, Max and Bloessl, Bastian and Sommer, Christoph and Dressler, Falko},
  doi = {10.1109/TMC.2022.3154643},
  title = {{Multi-Technology Cooperative Driving: An Analysis Based on PLEXE}},
  pages = {4792--4806},
  journal = {IEEE Transactions on Mobile Computing},
  issn = {1536-1233},
  publisher = {IEEE},
  month = {8},
  number = {8},
  volume = {22},
  year = {2023}
}
```
