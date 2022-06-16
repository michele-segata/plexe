#!/usr/bin/env Rscript
#
# Copyright (C) 2016-2022 Michele Segata <segata@ccs-labs.org>
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


args <- commandArgs(trailingOnly = T)

if (length(args) != 1) {
    stop("Usage: Rscript csv-to-rdata.R </path/to/file.csv>")
}

infile <- args[1]
outfile <- gsub(".csv", ".Rdata", infile)
runData <- read.csv(infile)
save(runData, file=outfile)
