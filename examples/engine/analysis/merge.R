#!/usr/bin/env Rscript
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Copyright (C) 2016 Bastian Bloessl <bloessl@ccs-labs.org>
# Copyright (C) 2016 Michele Segata <segata@ccs-labs.org>

source('generic-parsing-util.R')

args <- commandArgs(trailingOnly = T)

if (length(args) != 0) {

	res.folder <- args[1]

	files <- list.files(res.folder, pattern=paste('^', args[2], "_.*\\.Rdata", sep=""))
	outfile <- paste(res.folder, args[3], sep="")
	mapfile <- args[4]
	config <- args[5]

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

	for(f in 1:length(files)) {
		cat("[", f, "/", length(files), "] current file", files[f], "\n")
		load(files[f])
		params <- get.params(files[f], map[[config]]$fields, suffix=".Rdata")
		runData <- cbind(runData, params)
		allData <- rbind(allData, runData)
		rm(runData)
		gc()
	}

	save(allData, file=outfile)

}
