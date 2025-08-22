#!/usr/bin/env python
#
# Copyright (C) 2016-2023 Michele Segata <segata@ccs-labs.org>
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

from os.path import basename, dirname, join
from sys import argv, exit

from pandas import DataFrame, merge

from utils import MODULE, import_omnetpp_python_module, parse_map, get_params, NAMES

NAME = "name"
VECTIME = "vectime"
VECVALUE = "vecvalue"


# omnetpp results module, set by the import_omnetpp_python_module method
results = None


def get_selector(module, names):
    """
    given a module and the list of names, generates a textual selector to be
    passed to either 'add' or '
    """
    # since omnnet 6, the selector format has changed
    # sel = "(module({}) AND ({}))".format(module, all_names)
    all_names = " OR ".join(names)
    sel = "module=~\"{}\" AND ({})".format(module, all_names)
    return sel


def get_vector_names(d):
    return results.get_vectors(d)[NAME].unique()


def load_vectors_shaped(vecFile, selector):
    d = results.read_result_files(vecFile, filter_expression=selector)
    vector_names = get_vector_names(d)
    result = DataFrame()
    merge_columns = [MODULE, VECTIME]
    for v in vector_names:
        vectors = d.loc[d[NAME] == v][[MODULE, VECTIME, VECVALUE]]
        vectors = vectors.explode([VECTIME, VECVALUE], ignore_index=True)
        if all(x.is_integer() for x in vectors[VECVALUE]):
            vectors[VECVALUE] = vectors[VECVALUE].astype(int)
        vectors = vectors.rename(columns={VECVALUE: v})
        if result.empty:
            result = vectors
        else:
            result = merge(result, vectors, left_on=merge_columns,
                           right_on=merge_columns)

    result = result.rename(columns={VECTIME: "time"})
    columns = result.columns
    rename = dict({})
    for c in columns:
        if c.endswith(":vector"):
            rename[c] = c.replace(":vector", "")
    result = result.rename(columns=rename)
    result = result.drop(columns=MODULE)
    return result


def main():

    global results
    results = import_omnetpp_python_module()
    if results is None:
        print("Cannot import the OMNeT++ python library. Check that you are "
              "using OMNeT++ version >= 6 and to have added its bin folder "
              "to your PATH")
        exit(1)

    # parameters of the script:
    # 1: input vector file
    # 2: map config file
    # 3: map config configuration
    # 4: output file prefix
    if len(argv) < 4:
        print("generic-parser.py requires at least 4 parameters")
        exit(1)

    infile = argv[1]
    mapfile = argv[2]
    config = argv[3]
    prefix = argv[4]
    outtype = "csv"
    if len(argv) == 6:
        outtype = argv[5]

    if outtype == "Rdata":
        print("The output type Rdata is not directly supported by this script. "
              "Please use the csv-to-rdata.R script after exporting data to "
              "csv with this script.")
        exit(1)

    outfile = join(dirname(infile), "{}.{}".format(prefix, basename(infile)))
    outfile = outfile.replace(".vec", ".{}".format(outtype))

    # load map file
    map_data = parse_map(mapfile)
    # check whether required config exists
    if config not in map_data.keys():
        print("required config {} does not exist in {}".format(config, mapfile))
        exit(1)
    # get simulation parameters
    params = get_params(infile, map_data[config]["fields"])

    # debug output
    print("infile: {}".format(infile))
    print("outfile: {}".format(outfile))
    print("-------------------------")
    print("run: {}".format(params.iloc[0]["runNumber"]))

    selector = get_selector(map_data[config][MODULE], map_data[config][NAMES])
    data = load_vectors_shaped(infile, selector)
    data.to_csv(outfile, index=False)


if __name__ == "__main__":
    main()
