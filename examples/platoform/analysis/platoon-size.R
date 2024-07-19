library(plyr)
library(fixest)
library(ggplot2)
library(doMC)
registerDoMC(cores=detectCores())

# inputfile <- 'platoform.csv'
# inputfile <- 'platoformNOTTE14mar.csv'
inputfile <- 'platoform.parquet.csv'
inputfolder <- 'results'
# outputfile <- 'platoform-binned.csv'
fn <- paste(inputfolder, inputfile, sep='/')
d <- read.csv(fn)

HIGHWAY.LENGTH <- 10300

keep.only.valid <- function(d) {
    # valid <- unique(subset(d, time < max(d$time)-1)$platoonId)
    # return(subset(d, platoonId %in% valid))
    valid <- subset(d, time < max(d$time)-1)
    return(valid)
}

remove.single.vehicles <- function(d) {
    return(subset(d, platoonSize>1))
}

remove.end.of.highway <- function(d) {
    return(subset(d, platoVarX < HIGHWAY.LENGTH))
}

remove.failures <- function(d) {
    ddply(d, .(platoonId, platoonSize, pfvehId), function(x) {
        return(subset(x, time == min(x$time)))
    })
}

compute.platoons <- function(d) {
    ddply(d, .(platoonId, platoonSize), function(x) {
        formation <- x[1,]$platoonId
        time <- x[1,]$time
        platoVarX <- x[1,]$platoVarX
        if (nrow(x) > 1) {
            x <- subset(x, platoleaderFlag==0)
            formation <- c(formation, x[order(x$platoVarX, decreasing=T),]$pfvehId)
        }
        return(data.frame(time=time, platoVarX=platoVarX, formation=paste(formation, collapse=",")))
    })
}

keep.only.max.size <- function(d) {
    ddply(d, .(platoonId), function(x) {
        return(subset(x, platoonSize==max(x$platoonSize)))
    })
}

get.members <- function(formation) {
    as.numeric(strsplit(formation, split=",")[[1]])
}

remove.merged.platoons <- function(d) {
    d <- d[order(d$platoonSize, decreasing=T),]
    # list of indexes to delete
    to.delete <- c()
    for (i in 1:nrow(d)) {
        members <- get.members(d[i,]$formation)[-1]
        for (j in 1:length(members)) {
            del <- which(d$platoonId==members[j])
            if (length(del) > 0) {
                to.delete <- c(to.delete, del)
                # safety check on other members
                if (!all(get.members(d[del,]$formation) %in% members)) {
                    print("ERROR! Removing a record about a platoon but not all members can be found")
                    print(paste("Old platoon:", get.members(d[del,]$formation), collapse=" "))
                    print(d[del,])
                    print(paste("New platoon:", paste(get.members(d[i,]$formation), collapse=" ")))
                    print(d[i,])
                }
            }
        }
    }
    if (length(to.delete) > 0) {
        return(d[-to.delete,])
    } else {
        return(d)
    }
}


# step -1: platoon id da tenere: quelli che hanno loggato al tempo < max time
# step 0: delete all records with platoonSize == 1
# step 1: for all platoonId, for all platoonsize logged, for all vehid, keep only the one with the lower time
# step 2:

# id 1, dim 2, p 1200, v 1 4
# id 1, dim 3, p 5500, v 1 4 0
# id 20, dim 2, p 5600, v 20 25
# id 9, dim 2, p 5700, v 9 19
# id 9, dim 4, p 5800, v 9 19 20 25
# id 1, dim 7, p 7700, v 1 4 0 9 19 20 25
# ordinare platoon id per posizione, ciclare su posizione
# nel passaggio da un bin al successivo, copia il bin precedente, poi pulisci
# pulizia: inserisci gli eventi e poi per ogni platoon id tieni solo dim max
# | 0 1 | 1 2 | 2 3 | 4 5 | 5 6 | 6 7 | 7 8 |
#         2(1)  2(1)  2(1)  2(1)
#                           3(1)
#                           2(20)
#                           2(9)
#                           4(9)
# formato record
# bin -> lista
# ogni elemento lista contiene: platoon id, dimensione, lista veicoli
# altra condizione di pulizia: per tutti i record con platoonid = max
# cicla sui record dei bin ordinati per dimensione inversa
# per ogni platoon member in quel platoon, se esiste un record dove quel membro e' primo, elimina il record
# sanity check: controllare che i veicoli nel record eliminato siano parte del record in analisi
# | 0 1 | 1 2 | 2 3 | 4 5 | 5 6 | 6 7 | 7 8 |
#         2(1)  2(1)  2(1)
#                           3(1)
#                           2(20,25)
#                           4(9,19,20,25)
#
#
#
################################################################################
# TEST ZONE

# a = subset(d, vr==30 & mad==200 & rep==0 & mps==25 & pr==1)
# a = a[order(a$time, a$platoVarX),]
# b = subset(a, platoonId==1)
# print(b)

################################################################################

compute.binned.platoon.size <- function(run, keep.single.vehicles) {

    if (!keep.single.vehicles) {
        run <- remove.single.vehicles(run)
    }
    run <- keep.only.valid(run)
    run <- remove.end.of.highway(run)
    run <- remove.failures(run)
    run <- run[order(run$time, run$platoVarX),]
    platoons <- compute.platoons(run)
    platoons <- platoons[order(platoons$platoVarX, platoons$platoonId),]

    bins <- seq(1000, 11000, by=1000)
    bin.index <- 1
    current <- bins[bin.index]
    binned <- list()
    binned[[paste0(current)]]=data.frame()
    for (i in 1:nrow(platoons)) {
        p <- platoons[i,]
        while (p$platoVarX > current) {
            # we move to the next bin
            # clean current bin
            cb <- binned[[paste0(current)]]
            if (nrow(cb) != 0) {
                cb <- keep.only.max.size(cb)
                cb <- remove.merged.platoons(cb)
                binned[[paste0(current)]] <- cb
            }
            # move to next bin
            bin.index <- bin.index + 1
            current = bins[bin.index]
            # copy current bin into new
            binned[[paste0(current)]] <- cb
        }
        binned[[paste0(current)]] <- rbind(binned[[paste0(current)]], p)
    }
    while (bin.index < length(bins)) {
        # cleanup last bin
        cb <- binned[[paste0(current)]]
        if (nrow(cb) != 0) {
            cb <- keep.only.max.size(cb)
            cb <- remove.merged.platoons(cb)
            binned[[paste0(current)]] <- cb
        }
        # move to next bin
        bin.index <- bin.index + 1
        current = bins[bin.index]
        # copy current bin into new
        binned[[paste0(current)]] <- cb
    }
    binned[[paste0(bins[length(bins)])]] <- NULL
    return(binned)
}

compute.platoon.size.distribution <- function(binned) {
    bins <- seq(1000, 10000, by=1000)
    dataset <- data.frame()
    for (i in 1:length(bins)) {
        if (nrow(binned[[paste0(bins[i])]]) > 0) {
            dataset <- rbind(dataset, data.frame(bin=bins[i], platoonSize=binned[[paste0(bins[i])]]$platoonSize))
        }
    }
    return(dataset)
}

compute.in.platoon.count <- function(binned) {
    bins <- seq(1000, 10000, by=1000)
    dataset <- data.frame()
    for (i in 1:length(bins)) {
        if (nrow(binned[[paste0(bins[i])]]) > 0) {
            d <- binned[[paste0(bins[i])]]
            dataset <- rbind(dataset, ddply(d, .(platoonSize), function(x) {
                return(data.frame(bin=bins[i], cnt=nrow(x)))
            }))
        }
    }
    return(dataset)
}

compute.in.platoon.fraction <- function(binned) {
    return(ddply(binned, .(bin), function(x) {
        total = sum(x$platoonSize * x$cnt)
        singlev = sum(subset(x, platoonSize==1)$cnt)
        return(data.frame(ipr=(total-singlev)/total))
    }))
}

# pvr <- 30
# pmad <- 200
# prep <- 0
# pmps <- 25
# ppr <- 1
# 
# run <- subset(d, vr==pvr & mad==pmad & rep==prep & mps==pmps & pr==ppr)

psd <- ddply(d, .(vr, mad, rep, mps, pr), function(x) {
    print(paste("Processing", paste(x[1,], collapse=" ")))
    run <- compute.binned.platoon.size(x, F)
    binned <- compute.platoon.size.distribution(run)
    return(binned)
}, .parallel=T)

psd.merged <- ddply(psd, .(vr, mad, mps, pr, bin), function(x) {
    return(data.frame(platoonSize=x$platoonSize))
}, .parallel=T)

ipc <- ddply(d, .(vr, mad, rep, mps, pr), function(x) {
    print(paste("Processing", paste(x[1,], collapse=" ")))
    run <- compute.binned.platoon.size(x, T)
    binned <- compute.in.platoon.count(run)
    return(binned)
}, .parallel=T)
ipr <- ddply(ipc, .(vr, mad, mps, pr), function(x) {
    return(compute.in.platoon.fraction(x))
})

psa <- ddply(subset(psd, bin==10000), .(vr, mad, mps, pr), function(x) {
    return(data.frame(psa=mean(x$platoonSize)))
})

write.csv(psd.merged, paste(inputfolder, "platoon-size-distr.csv", sep="/"), row.names=F)

write.csv(ipr, paste(inputfolder, "in-platoon-rate.csv", sep="/"), row.names=F)


# run <- remove.single.vehicles(run)
# run <- keep.only.valid(run)
# run <- remove.end.of.highway(run)
# run <- remove.failures(run)
# run <- run[order(run$time, run$platoVarX),]
# platoons <- compute.platoons(run)
# platoons <- platoons[order(platoons$platoVarX, platoons$platoonId),]

# test.df <- data.frame(
#     platoonId=c(0,1,4,1,1,20,9,9,1),
#     platoonSize=c(1,1,1,2,3,2,2,4,7),
#     time=1:9,
#     platoVarX=c(25,25,25,1200,5500,5600,5700,5800,7700),
#     formation=c("0","1","4","1,4","1,4,0","20,25","9,19","9,19,20,25","1,4,0,9,19,20,25")
# )
# id 0, dim 1, p 25, v 0
# id 1, dim 1, p 25, v 1
# id 4, dim 1, p 25, v 4
# id 1, dim 2, p 1200, v 1 4
# id 1, dim 3, p 5500, v 1 4 0
# id 20, dim 2, p 5600, v 20 25
# id 9, dim 2, p 5700, v 9 19
# id 9, dim 4, p 5800, v 9 19 20 25
# id 1, dim 7, p 7700, v 1 4 0 9 19 20 25
# TEST!
# platoons <- test.df

#bins <- seq(1000, 11000, by=1000)
#bin.index <- 1
#current <- bins[bin.index]
#binned <- list()
#binned[[paste0(current)]]=data.frame()
#for (i in 1:nrow(platoons)) {
#    p <- platoons[i,]
#    while (p$platoVarX > current) {
#        # we move to the next bin
#        # clean current bin
#        cb <- binned[[paste0(current)]]
#        if (nrow(cb) != 0) {
#            cb <- keep.only.max.size(cb)
#            cb <- remove.merged.platoons(cb)
#            binned[[paste0(current)]] <- cb
#        }
#        # move to next bin
#        bin.index <- bin.index + 1
#        current = bins[bin.index]
#        # copy current bin into new
#        binned[[paste0(current)]] <- cb
#    }
#    binned[[paste0(current)]] <- rbind(binned[[paste0(current)]], p)
#}
#while (bin.index < length(bins)) {
#    # cleanup last bin
#    cb <- binned[[paste0(current)]]
#    if (nrow(cb) != 0) {
#        cb <- keep.only.max.size(cb)
#        cb <- remove.merged.platoons(cb)
#        binned[[paste0(current)]] <- cb
#    }
#    # move to next bin
#    bin.index <- bin.index + 1
#    current = bins[bin.index]
#    # copy current bin into new
#    binned[[paste0(current)]] <- cb
#}

# stop()
# 
# binss <- paste0("cut::", paste(bins, collapse=']'), "]")
# 
# d$bin <- bins[as.numeric(bin(d$platoVarX, binss))]
# write.csv(d, file=paste(inputfolder, outputfile, sep='/'), row.names=F)
# 
# p <- ggplot(d, aes(group=bin, y=platoonSize)) + geom_boxplot() + facet_grid(vr~mad)
# 
# ggsave("platoonsize.pdf", plot=p, width=10, height=10)


