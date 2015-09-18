library(omnetpp)
library(reshape2, warn.conflicts = F)

getScalars <- function(scaFiles, module, scalarName, attributeName) {

    selectString <- paste('module(', module, ') AND name("', scalarName,'")', sep='')
    dataset <- loadDataset(scaFiles, add('scalar', select=selectString))

    # attrvalues is a factor with levels
    attrvalues <- unique(subset(dataset$runattrs$attrvalue, dataset$runattrs$attrname == attributeName))
    attrvalues <- sort(as.integer(levels(attrvalues)[as.integer(attrvalues)]))

    res <- matrix(0, nrow=0, ncol=2)

    # iterate through distances
    for (i in 1:length(attrvalues)) {
        a <- attrvalues[i]
        runids <- subset(dataset$runattrs$runid, dataset$runattrs$attrname == attributeName & dataset$runattrs$attrvalue == a)
        p <- subset(dataset$scalars$value, is.element(dataset$scalars$runid, runids))
        res <- rbind(res, cbind(a, p))
    }

    return(res)
}

loadVectorsShaped <- function(vecFiles, ...){
	d <- loadDataset(vecFiles, ...)

	vectornames <- levels(d$vectors$name)
	if(length(vectornames) == 0) {
		warning("no vectors loaded!")
		return
	}
	if(length(d$scalars$name)){
		warning("scalars loaded, but will not be considered in this function")
	}

	#preprocess first vector
	reskeys <- subset(d$vectors$resultkey, d$vectors$name == vectornames[1])
	v <- loadVectors(d,reskeys)
	result <- v$vectordata

	#process remaining vectors
	if(length(vectornames) > 1) {
		for(name in vectornames[2:length(vectornames)]) {
			reskeys <- subset(d$vectors$resultkey, d$vectors$name == name)
			v <- loadVectors(d,reskeys)
			result <- merge(result, v$vectordata, by=c("eventno", "resultkey", "x"), all.x = TRUE, all.y = TRUE, suffixes = c("",name))
		}
	}
	names(result) <- c(names(result)[1:2], "time", vectornames)
	return (result)
}

listVectors <- function(vecFiles, ...){
	d <- loadDataset(vecFiles, ...)
	return (levels(d$vectors$name))
}

print.runs <- function(vecFiles) {
	d <- loadDataset(vecFiles)
	print(cast(d$runattrs, runid~attrname, value='attrvalue'))
}
