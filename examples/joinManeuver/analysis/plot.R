#load omnet helper function read .vec files
source('omnet_helpers.R')
#load ggplot for quick and dirty plotting
library(ggplot2)

#load simulation output file
load('../results/JoinManeuver.Rdata')

#plot speed as function of time for different controller parameters
p1 <-    ggplot(allData, aes(x=time, y=speed*3.6, col=factor(nodeId))) +
        geom_line() +
        xlim(c(0, 100)) +
        ylim(c(90, 150)) +
        facet_grid(caccXi~.)
#print(p1)
ggsave('speed.pdf', p1, width=16, height=9)

#plot distance as function of time for different controller parameters
ss <-    subset(allData, nodeId != 0)
p2 <-    ggplot(ss, aes(x=time, y=distance, col=factor(nodeId))) +
        geom_line() +
        xlim(c(0, 100)) +
        ylim(c(0, 100)) +
        facet_grid(caccXi~.)
#print(p2)
ggsave('distance.pdf', p2, width=16, height=9)

#plot acceleration as function of time for different controller parameters
p3 <-    ggplot(allData, aes(x=time, y=acceleration, col=factor(nodeId))) +
        geom_line() +
        xlim(c(0, 100)) +
        ylim(c(-4, 3)) +
        facet_grid(caccXi~.)
#print(p3)
ggsave('acceleration.pdf', p3, width=16, height=9)
