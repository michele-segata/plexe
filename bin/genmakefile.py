#!/usr/bin/env python3
#
# Copyright (C) 2016-2023 Bastian Bloessl <bloessl@ccs-labs.org>
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

import sys
import configparser
from re import search
from subprocess import run, PIPE

CONFIG = 0
MAP = 1
MAPFILE = 2
PREFIX = 3
OUT = 4
MERGE = 5
OUTTYPE = 6


class Multidict(dict):
    _unique = 0

    def __setitem__(self, key, val):
        if isinstance(val, dict):
            self._unique += 1
            key += str(self._unique)
        dict.__setitem__(self, key, val)


def err(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


def get_omnet_version():
    major = 0
    try:
        result = run(['opp_run', '-v'], stdout=PIPE)
        if not result.returncode == 0:
            err("Cannot check for OMNeT++ version. Is opp_run in your PATH?")
            exit(1)
        version = search("Version: (.*?),", result.stdout.decode())
        if version:
            version_string = version.group(1)
            version_parts = version_string.split(".")
            major = int(version_parts[0])
    except FileNotFoundError:
        err("Cannot check for OMNeT++ version. Is opp_run in your PATH?")
        exit(1)
    return major


# get omnet version
opp_version = get_omnet_version()

# instantiate a config parser
cfg = configparser.ConfigParser(None, Multidict, strict=False)

# parse the config file
if len(sys.argv) != 2:
    err("Usage: genmakefile.py <config file>")
    sys.exit(1)

try:
    cfgFile = open(sys.argv[1])
    cfg.read_file(cfgFile)
except Exception as e:
    err(("Unable to parse " + sys.argv[1]))
    err(e)
    sys.exit(1)

# where the results are stored
resultDir = "../results"

# list of configs: config name, parsing script, prefix for output Rdata file
configs = []

for s in cfg.sections():
    if s.startswith("params"):
        if cfg.has_option(s, "resdir"):
            resultDir = cfg.get(s, "resdir")

    if s.startswith("config"):
        if not cfg.has_option(s, "out"):
            err(("section " + s + " misses out option"))
            sys.exit(1)
        if not cfg.has_option(s, "config"):
            err(("section " + s + " misses config option"))
            sys.exit(1)
        if not cfg.has_option(s, "map"):
            err(("section " + s + " misses map option"))
            sys.exit(1)
        if not cfg.has_option(s, "mapFile"):
            err(("section " + s + " misses mapFile option"))
            sys.exit(1)
        if not cfg.has_option(s, "prefix"):
            err(("section " + s + " misses prefix name"))
            sys.exit(1)
        out_type = "Rdata"
        if cfg.has_option(s, "type"):
            out_type = cfg.get(s, "type")
            supported = ['csv', 'Rdata', 'feather', 'parquet']
            if out_type not in supported:
                err("output type can be: " + ",".join(supported))
                sys.exit(1)
        configs.append([cfg.get(s, "config"), cfg.get(s, "map"), cfg.get(s, "mapFile"), cfg.get(s, "prefix"), cfg.get(s, "out"), cfg.get(s, "merge"), out_type])


def get_spaces(string, length):
    return ' ' * (length - len(string))


def get_longer(configs):
    longer = 0
    for c in configs:
        if len(c[OUT]) > longer:
            longer = len(c[OUT])
    # longer + 5 (_DATA)
    return longer + 5


# output safety checks for OMNeT version
print("REQUIRED_VERSION=\"{}\"".format(get_omnet_version()))
print("ifeq (, $(shell which opp_run))")
print("$(error Cannot determine OMNeT++ version. Is opp_run in your PATH?)")
print("endif")
print("OPP_VERSION=\"$(shell opp_run -v | grep Version | sed -e \"s/Version: //\" -e \"s/,.*//\" | cut -d'.' -f 1)\"")
print("ifneq (\"$(OPP_VERSION)\",\"$(REQUIRED_VERSION)\")")
print("$(error This Makefile has been generated for OMNeT++ version $(REQUIRED_VERSION), but version $(OPP_VERSION) has been found. Please re-generate the Makefile using genmakefile.py)")
print("endif\n")

# output some variables
print("# tool for indexing vec files")
print("SCAVETOOL = opp_scavetool")
print("# results location")
print(("RESDIR = " + resultDir))
print("# script for merging")
print("MERGESCRIPT = merge.R")
print("")

longer = get_longer(configs)

for c in configs:
    print(("# match all .vec files for the " + c[CONFIG] + " config"))
    print((c[OUT].upper() + get_spaces(c[OUT], longer) + " = $(wildcard $(RESDIR)/" + c[CONFIG] + "*.vec)"))
    print(("# change suffix from .vec to ." + c[OUTTYPE] + " and add the " + c[PREFIX] + " prefix"))
    print((c[OUT].upper() + "_DATA" + get_spaces(c[OUT] + "_DATA", longer) + " = $(" + c[OUT].upper() + ":$(RESDIR)/%.vec=$(RESDIR)/" + c[PREFIX] + ".%." + c[OUTTYPE] + ")"))

print("")
print("# vector index files and Rdata files")
print("VCI = $(VECTOR:%.vec=%.vci)")
print("RDATA = $(VECTOR:%.vec=%.Rdata)")
print("CSV = $(VECTOR:%.vec=%.csv)")
print("PAR = $(VECTOR:%.vec=%.parquet)")
print("FHT = $(VECTOR:%.vec=%.feather)")

print("")
print("# all make targets")
sys.stdout.write("all: ")
for c in configs:
    sys.stdout.write(c[OUT] + "." + c[OUTTYPE] + " ")
print("")
print("")

for c in configs:
    print(("# to make " + c[OUT] + "." + c[OUTTYPE] + " we need to merge all files starting with " + c[PREFIX] + "." + c[CONFIG]))
    print(("# before this, check that all " + c[OUT].upper() + "_DATA files have been processed"))
    print(("$(RESDIR)/" + c[OUT] + "." + c[OUTTYPE] + ": $(" + c[OUT].upper() + "_DATA)"))
    if c[MERGE] == '1':
        print(("\t$(MERGESCRIPT) $(RESDIR)/ " + c[PREFIX] + "." + c[CONFIG] + " $(notdir $@) " + c[MAPFILE] + " " + c[MAP] + " " + c[OUTTYPE]))
    else:
        print("\t$(MERGESCRIPT)")
    print((c[OUT] + "." + c[OUTTYPE] + ": $(RESDIR)/" + c[OUT] + "." + c[OUTTYPE]))
    print("")

for c in configs:
    print(("# to make all " + c[PREFIX] + ".*." + c[OUTTYPE] + " files we need to run the generic parser"))
    # vec to csv
    if opp_version == 5:
        print((c[PREFIX] + ".%." + c[OUTTYPE] + ": %.vec %.vci"))
        print(("\tgeneric-parser.R $< " + c[MAPFILE] + " " + c[MAP] + " " + c[PREFIX] + " " + c[OUTTYPE]))
    elif opp_version >= 6:
        print((c[PREFIX] + ".%." + c[OUTTYPE] + ": %.vec %.vci"))
        print(("\tgeneric-parser.py $< " + c[MAPFILE] + " " + c[MAP] + " " + c[PREFIX] + " " + c[OUTTYPE]))
        #if c[OUTTYPE] == "Rdata":
        #    # if needed, csv to Rdata
        #    print(
        #        (c[PREFIX] + ".%." + c[OUTTYPE] + ": " + c[PREFIX] + ".%.csv"))
        #    print("\tcsv-to-rdata.R $<")
        #    print("\trm $<")
    else:
        err("OMNeT++ version {} is not supported.".format(opp_version))
        sys.exit(1)

    print("")

print("# if vec files are not indexed, index them")
print("%.vci : %.vec")
print("\t$(SCAVETOOL) index $<")
print("")

print("# helper to print variable values. e.g.: make print-DELAY_DATA")
print("print-%:")
print("\t@echo '$*=$($*)'")

print("")
print("# every intermediate file is kept instead of being automatically deleted.")
print("# .vci files are detected as intermediate and thus cancelled when the make")
print("# command terminates. however, such files can still be needed by other")
print("# targets, so keep them instead of re-doing indexing")
print(".SECONDARY :")
