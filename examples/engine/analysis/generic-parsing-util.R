#
# Copyright (C) 2016-2019 Michele Segata <segata@ccs-labs.org>
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

# determine whether a string contains a parsable number
is.number <- function(string) {
    if (length(grep("^[[:digit:]]*$", string)) == 1)
        return (T)
    else
        return (F)
}

# given a module and the list of names, generates a textual selector to be
# passed to either 'add' or '
get.selector <- function(module, names) {
    sel <- paste('(module(', module, ') AND (',
        paste(names, collapse=" OR "), '))', sep='')
    return (sel)
}

# given the list of names, generates a subset condition to remove NAs
get.subset.condition <- function(names) {
    names <- gsub("name\\(", "!is.na(", names)
    sel <- paste(names, collapse=" & ", sep='')
    return (sel)
}

# gets the list of files with a certain prefix and suffix in a folder
get.data.files <- function(folder, prefix, suffix=".Rdata") {
    if (strsplit(suffix, '')[[1]][1] == '.')
        suffix <- paste('\\', suffix, sep='')
    return(list.files(folder, pattern=paste('^', prefix, '_.*', suffix, sep='')))
}

# given a list of files and the mapping to parameters, returns a dataset with
# all simulation configurations
get.configurations <- function(data.files, fields, suffix=".Rdata") {
    cfg <- data.frame()
    for (d in data.files)
        cfg <- rbind(cfg, data.frame(get.params(d, fields, suffix), filename=d))
    return(cfg)
}

# given the .vec filename and the map of field names, returns a single row
# data.frame where columns have the name of field names and the value is the
# value of the parameter for the simulation
get.params <- function(filename, fields, suffix=".vec") {
    p <- strsplit(gsub(suffix, "", basename(filename)), "_")[[1]]
    #to add a column, we need to have something in the dataframe, so we add a
    #fake column which we remove at the end
    d <- data.frame(todelete=1)
    for (f in names(fields)) {
        d[[fields[[f]]]] <- as.numeric(p[as.integer(f)])
    }
    d$todelete <- NULL
    return (d)
}

# load and parses a vector file keeping only fields indicated by the selector
prepare.vector <- function(vecFile, selector) {
    ds1 <- loadVectorsShaped(vecFile, add('vector', select=selector))
    ds1$eventno <- NULL
    ds1$resultkey <- NULL
    return(ds1)
}

# parses a map file
parse.map <- function(mapfile) {

    configs <- list()
    a <- readLines(mapfile)

    setup <- ''

    for (r in 1:length(a)) {
        #remove blank spaces first
        l <- gsub(" ", "", a[r], fixed=T)
        l <- gsub("\t", "", l, fixed=T)
        #is this a comment or an empty line? if so skip the line
        if (substr(l, 1, 1) == "#" || l == "")
            next

        #check if this is the beginning of a section
        if (substr(l, 1, 1) == "[" && substr(l, nchar(l), nchar(l)) == "]") {
            #save setup
            if (setup != '') {
                configs[[setup]] <- list(module=module, names=names, fields=fields)
            }
            #begin new setup
            setup <- substr(l, 2, nchar(l) - 1)
            module <- ''
            names <- ''
            fields <- list()
        } else {
            #split variable and value
            e <- strsplit(l, "=")[[1]]
            if (e[1] == "inherit") {
                if (is.null(configs[[e[2]]])) {
                    stop(paste("config", setup, "tries to inherit from", e[2], "which is undefined"))
                }
                #copy values from super config
                module <- configs[[e[2]]]$module
                names <- configs[[e[2]]]$names
                fields <- configs[[e[2]]]$fields
            } else if (e[1] == "module") {
                module <- e[2]
            } else if (e[1] == "names") {
                names <- paste('name(', strsplit(e[2], ",")[[1]], ')', sep='')
            } else if (is.number(e[1])) {
                idx <- e[1]
                fd <- e[2]
                fields[[idx]] <- fd
            } else {
                stop(paste("invalid variable in", mapfile))
            }
        }

    }

    configs[[setup]] <- list(module=module, names=names, fields=fields)

    return (configs)

}
#save data to file in the results dir
ssave <- function(data, filename, resultsLocation='', envir=parent.frame()) {
    if (resultsLocation == '')
        fn <- filename
    else
        fn <- paste(resultsLocation, filename, sep='/')
    save(list=as.character(substitute(data)), envir=envir, file=fn)
}
