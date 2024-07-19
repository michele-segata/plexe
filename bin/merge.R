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

library(data.table)
to.source <- c("generic-parsing-util.R")
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

if (length(args) != 0) {

    res.folder <- args[1]

    outfile <- paste(res.folder, args[3], sep="")
    mapfile <- args[4]
    config <- args[5]
    outtype = "Rdata"
    if (length(args) == 6) {
        outtype = args[6]
    }
    if (outtype == "feather" | outtype == "parquet") {
        library("arrow")
    }

    files <- list.files(res.folder, pattern=paste('^', args[2], "_.*\\.", outtype, sep=""))

    #get maps from mapfile
    map <- parse.map(mapfile)

    files <- paste(res.folder, files, sep="")

    if(1) {
        print("merging data")
        cat("outfile: ", outfile, "\n")
        print("files:")
        print(files)
    }

    allData <- data.frame()

    multmerge = function(files) {
        i <- 1
        datalist = lapply(files, function(x) {
            cat("[", i, "/", length(files), "] current file", x, "\n")
            i <<- i + 1
            if (outtype == "Rdata") {
                load(x)
            } else if (outtype == "csv") {
                runData <- read.csv(x)
            } else if (outtype == "feather") {
                runData <- read_feather(x)
            }
            else if (outtype == "parquet") {
                runData <- read_parquet(x)
            }

            params <- get.params(x, map[[config]]$fields, suffix=paste(".", outtype, sep=""))
            runData <- cbind(runData, params)
            runData
        })
        cat("Merging...\n")
        rbindlist(datalist)
    }
    allData = multmerge(files)
    allData <- type.convert(allData, as.is = TRUE)
    cat("Saving to", outfile, "...\n")
    if (outtype == "Rdata") {
        save(allData, file=outfile)
    } else if (outtype == "csv") {
        write.csv(allData, file=outfile, row.names=F)
    } else if (outtype == "feather") {
        write_feather(allData, outfile)
    }
    else if (outtype == "parquet") {
        write_parquet(allData, outfile)
    }

}
