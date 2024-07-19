#!/usr/bin/env Rscript
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

to.source <- c("omnet_helpers.R", "generic-parsing-util.R")
plexe.dir <- Sys.getenv("PLEXE_DIR")
if (plexe.dir == "") {
    plexe.bin.dir <- "."
} else {
    plexe.bin.dir <- paste(plexe.dir, "bin", sep="/")
}
for (f in paste(plexe.bin.dir, to.source, sep="/")) {
    if (file.exists(f)) source(f)
    else stop(paste(f, "not found. Did you define PLEXE_DIR by running 'source setenv'?"))
}

args <- commandArgs(trailingOnly = T)

#parameters of the script:
#1: input vector file
#2: map config file
#3: map config configuration
#4: output file prefix

if (length(args) < 4) {
    stop("generic-parse.R requires at least 4 parameters")
}
infile  <- args[1]
mapfile <- args[2]
config  <- args[3]
prefix  <- args[4]
outtype <- "Rdata"
if (length(args) == 5) {
    outtype  <- args[5]
}
if (outtype == "feather" | outtype == "parquet") {
    library("arrow", warn.conflicts = FALSE) #func 'timestamp' wuold conflict with ‘package:utils’
}

outfile <- paste(dirname(infile), '/', prefix, '.', basename(infile), sep='')
outfile <- gsub(".vec", paste(".", outtype, sep=''), outfile)

#load map file
map <- parse.map(mapfile)
#check whether required config exists
if (is.null(map[[config]])) {
    stop("required config", config, "does not exist in", mapfile)
}
#get simulation parameters
params <- get.params(infile, map[[config]]$fields)

# debug output
cat("infile: ", infile, "\n")
cat("outfile; ", outfile, "\n")
cat("-------------------------\n")
cat("run: ", params$runNumber, "\n")

# wastes a lot of storage - but it's easy to handle
selector <- get.selector(map[[config]]$module, map[[config]]$names)
condition <- get.subset.condition(map[[config]]$names)
toclean <- prepare.vector(infile, selector)
names(toclean) <- rename.columns(names(toclean))
toclean <- subset(toclean, eval(parse(text=condition)))
gc()
runData <- type.convert(toclean)

if (outtype == "Rdata") {
    save(runData, file=outfile)
} else if (outtype == "csv") {
    write.csv(runData, file=outfile, row.names=F)
} else if (outtype == "feather") {
    write_feather(runData, outfile)
} else if (outtype == "parquet") {
    write_parquet(runData, outfile)
}

warnings()
