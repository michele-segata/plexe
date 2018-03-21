Plexe-Veins
===========

[Plexe](http://plexe.car2x.org) is an extension of the popular [Veins](http://veins.car2x.org) vehicular network simulator which permits the realistic simulation of platooning (i.e., automated car-following) systems. It features realistic vehicle dynamics and several cruise control models, permitting the analysis of control systems, large-scale and mixed scenario, as well as networking protocols and cooperative maneuvers. It is free to download and easy to extend.

This is the main repository for the Veins part of the Plexe simulator. Please see the official [website](http://plexe.car2x.org) for the documentation.

Code structure
--------------

Code is organized into branches:

* `master`: the master branch is the development branch. It includes commits that are considered relatively stable. This somehow replaces the old `plexe-dev` branch, but the commits should be more stable and tested.
* `plexe-x.y`: branches of this form tag stable releases. If you want to switch to version 2.1, simply checkout the `plexe-2.1` branch.
