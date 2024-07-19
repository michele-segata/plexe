#!/usr/bin/env python3
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

from importlib import import_module
from os.path import basename, dirname, join
from sys import argv, exit, path

from pandas import DataFrame, merge
from shutil import which

NAME = "name"
NAMES = "names"
MODULE = "module"
FIELDS = "fields"
VECTIME = "vectime"
VECVALUE = "vecvalue"


# omnetpp results module, set by the import_omnetpp_python_module method
results = None


def import_omnetpp_python_module():
    global results
    bin_dir = which("opp_run")
    if bin_dir == "":
        return False
    root_dir = dirname(dirname(bin_dir))
    python_dir = join(root_dir, "python")
    path.append(python_dir)
    try:
        results = import_module(".results", "omnetpp.scave")
    except ModuleNotFoundError:
        return False
    return True


def get_name_selector(name, vector=False):
    # since omnnet 6, the selector format has changed
    # return "name({})".format(name)
    if vector:
        return "name=~\"{}:vector\"".format(name)
    else:
        return "name=~\"{}\"".format(name)


def parse_map(mapfile):
    configs = dict()
    with open(mapfile) as file:
        a = [line.rstrip() for line in file]

    setup = ""
    module = ""
    names = ""
    fields = dict()

    for r in range(len(a)):
        # remove blank spaces first
        l = a[r].replace(" ", "")
        l = l.replace("\t", "")
        # is this a comment or an empty line? if so skip the line
        if l == "" or l[0] == "#":
            continue

        # check if this is the beginning of a section
        if l[0] == "[" and l[-1] == "]":
            # save setup
            if not setup == "":
                configs[setup] = dict({
                    MODULE: module,
                    NAMES: names,
                    "fields": fields,
                })
            # begin new setup
            setup = l[1:-1]
            module = ""
            names = ""
            fields = dict()
        else:
            # split variable and value
            e = l.split("=")
            if e[0] == "inherit":
                if e[1] not in configs.keys():
                    m = "config {} tries to inherit from {}, which is undefined"
                    print(m.format(setup, e[1]))
                    exit(1)
                # copy values from super config
                module = configs[e[1]][MODULE]
                names = configs[e[1]][NAMES]
                fields = configs[e[1]]["fields"]
            elif e[0] == MODULE:
                module = e[1]
            elif e[0] == NAMES:
                names = [get_name_selector(x) for x in e[1].split(",")]
                namesv = [get_name_selector(x, True) for x in e[1].split(",")]
                names.extend(namesv)
            elif e[0].isnumeric():
                # -1 because the idx in R was 1-based, in python it is 0 based
                idx = int(e[0]) - 1
                fd = e[1]
                fields[idx] = fd
            else:
                print("invalid variable in {}".format(mapfile))

    configs[setup] = dict({
        MODULE: module,
        NAMES: names,
        FIELDS: fields,
    })

    return configs


def get_params(filename, fields, suffix=".vec"):
    """
    given the .vec filename and the map of field names, returns a single row
    data.frame where columns have the name of field names and the value is the
    value of the parameter for the simulation
    """
    p = basename(filename).replace(suffix, "").split("_")
    # f is the index of the parameter taken from map_file
    # p[f] is the value of the parameter
    # fields[f] is the name of the parameter
    param_names = [f for f in fields.values()]
    values = [[int(p[f]) if p[f].isnumeric() else str(p[f]) for f in fields.keys()]]
    d = DataFrame(values, columns=param_names)
    return d


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
        vectors = d.loc[(d[NAME] == v) & (d[VECVALUE].notnull())][[MODULE, VECTIME, VECVALUE]]
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

    if not import_omnetpp_python_module():
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
    print("run: {}".format(params.iloc[0].to_list()))

    selector = get_selector(map_data[config][MODULE], map_data[config][NAMES])
    data = load_vectors_shaped(infile, selector)
    if outfile.endswith(".csv"):
        data.to_csv(outfile, index=False)
    elif outfile.endswith("feather"):
        data.to_feather(outfile)
    elif outfile.endswith("parquet"):
        data.to_parquet(outfile, index=False)


if __name__ == "__main__":
    main()
