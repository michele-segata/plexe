#!/usr/bin/env python
#
# Copyright (C) 2016-2025 Michele Segata <segata@ccs-labs.org>
#
# SPDX-License-Identifier: GPL-2.0-or-later
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

from glob import glob
from sys import argv

import pandas as pd
from pandas import concat

from utils import import_omnetpp_python_module, get_params, parse_map


def main():

    results = import_omnetpp_python_module()

    if results is None:
        print("Cannot import the OMNeT++ python library. Check that you are "
              "using OMNeT++ version >= 6 and to have added its bin folder "
              "to your PATH")
        exit(1)

    # parameters of the script:
    # 1: input folder
    # 2: map config file
    # 3: map config configuration
    # 4: input file prefix
    # 5: output file name
    # 6+: list of scalars to extract
    if len(argv) < 6:
        print("process-scalars.py requires at least 6 parameters")
        print("Usage: process-scalars.py <input folder> <map config file> <map config configuration> "
              "<input file prefix> <output file name> <name of scalar 1> [<name of scalar 2> ...]")
        exit(1)

    folder = argv[1]
    mapfile = argv[2]
    config = argv[3]
    pattern = argv[4]
    outfile = argv[5]
    scalars = argv[6:]

    files = glob(f"{folder}/{pattern}_*.sca")

    # load map file
    map_data = parse_map(mapfile)
    # check whether required config exists
    if config not in map_data.keys():
        print("required config {} does not exist in {}".format(config, mapfile))
        exit(1)

    data = pd.DataFrame()
    total_files = len(files)
    current = 1
    for f in files:
        # get simulation parameters
        params = get_params(f, map_data[config]["fields"], suffix=".sca")
        print(f"Processing file {current} out of {total_files} ({f})")
        current = current + 1
        d = results.read_result_files(f)
        all_scalars = d.loc[d["type"] == "scalar"][["module", "name", "value"]]
        scalar_data = all_scalars.loc[all_scalars["name"].isin(scalars)]
        data_with_params = scalar_data.merge(params, how="cross")
        data = concat([data, data_with_params])

    data.to_csv(outfile, index=False)


if __name__ == "__main__":
    main()
