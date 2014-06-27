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
			)
)
'))
}

#load simulation output files
accCloseData <- prepare.vector('../results/platoons_Sinusoidal-0-0-0.3-2-200-1-0.1-0.vec')
accFarData <- prepare.vector('../results/platoons_Sinusoidal-1-0-1.2-2-200-1-0.1-0.vec')
caccData <- prepare.vector('../results/platoons_Sinusoidal-2-1-0.3-2-200-1-0.1-0.vec')
ccData <- prepare.vector('../results/platoons_Sinusoidal-3-2-0.3-2-200-1-0.1-0.vec')

#add a column to distinguish them before merging
accCloseData$controller <- "ACC (0.3s)"
accFarData$controller <- "ACC (1.2s)"
caccData$controller <- "CACC"
ccData$controller <- "MYCC"

#merge all data together
allData <- rbind(accCloseData, accFarData, caccData, ccData)

#plot speed as function of time for different controllers
p1 <-	ggplot(allData, aes(x=time, y=speed, col=factor(nodeId))) +
		geom_line() +
		xlim(c(80, 100)) +
		ylim(c(24,31)) +
		facet_grid(controller~.)
#print(p1)
ggsave('speed.pdf', p1, width=16, height=9)

#plot distance as function of time for different controllers
ss <-	subset(allData, nodeId != 0)
p2 <-	ggplot(ss, aes(x=time, y=distance, col=factor(nodeId))) +
		geom_line() +
		xlim(c(80, 100)) +
		facet_grid(controller~., scales="free_y")
#print(p2)
ggsave('distance.pdf', p2, width=16, height=9)
