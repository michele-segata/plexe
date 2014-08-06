#load omnet helper function read .vec files
source('omnet_helpers.R')
#load ggplot for quick and dirty plotting
library(ggplot2)


#function to load data needed for plotting
prepare.vector <- function(vecFile) {
	ds1 <- loadVectorsShaped(vecFile,
		add('vector', select='
(
module(scenario.node[*].prot)
			AND (
				name(nodeId)
				OR name(distance)
				OR name(relativeSpeed)
				OR name(speed)
				OR name(posx)
				OR name(posy)
				OR name(acceleration)
			)
)
'))
}

#load simulation output files
test1.data <- prepare.vector('../results/JoinManeuver_1_0.2_0.vec')
test2.data <- prepare.vector('../results/JoinManeuver_2_1_0.vec')

#add a column to distinguish them before merging
test1.data$xi <- 1
test1.data$omegan <- 0.2
test2.data$xi <- 2
test2.data$omegan <- 1

#merge all data together
allData <- rbind(test1.data, test2.data)

#plot speed as function of time for different controller parameters
p1 <-	ggplot(allData, aes(x=time, y=speed*3.6, col=factor(nodeId))) +
		geom_line() +
		xlim(c(0, 100)) +
		ylim(c(90, 150)) +
		facet_grid(xi~.)
#print(p1)
ggsave('speed.pdf', p1, width=16, height=9)

#plot distance as function of time for different controller parameters
ss <-	subset(allData, nodeId != 0)
p2 <-	ggplot(ss, aes(x=time, y=distance, col=factor(nodeId))) +
		geom_line() +
		xlim(c(0, 100)) +
		ylim(c(0, 100)) +
		facet_grid(xi~.)
#print(p2)
ggsave('distance.pdf', p2, width=16, height=9)

#plot acceleration as function of time for different controller parameters
p3 <-	ggplot(allData, aes(x=time, y=acceleration, col=factor(nodeId))) +
		geom_line() +
		xlim(c(0, 100)) +
		ylim(c(-4, 3)) +
		facet_grid(xi~.)
#print(p3)
ggsave('acceleration.pdf', p1, width=16, height=9)
