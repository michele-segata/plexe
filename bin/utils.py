from importlib import import_module
from os.path import dirname, join, basename
from shutil import which
from sys import path

from pandas import DataFrame

NAMES = "names"
MODULE = "module"
FIELDS = "fields"


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
    values = [[int(p[f]) if p[f].isnumeric() else float(p[f]) for f in fields.keys()]]
    d = DataFrame(values, columns=param_names)
    return d


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


def import_omnetpp_python_module():
    bin_dir = which("opp_run")
    if bin_dir == "":
        return False
    root_dir = dirname(dirname(bin_dir))
    python_dir = join(root_dir, "python")
    path.append(python_dir)
    try:
        results = import_module(".results", "omnetpp.scave")
    except ModuleNotFoundError:
        return None
    return results


