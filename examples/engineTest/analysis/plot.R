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
				name(speed)
				OR name(acceleration)
			)
)
'))
}


#load simulation output files
alfa    <- prepare.vector('../results/EngineTest_0_0.vec')
audi    <- prepare.vector('../results/EngineTest_1_0.vec')
bugatti <- prepare.vector('../results/EngineTest_2_0.vec')

#add a column to distinguish them before merging
alfa$car    <- 'Alfa 147'
audi$car    <- 'Audi R8'
bugatti$car <- 'Bugatti Veyron'

#merge all data together
allData <- rbind(alfa, audi, bugatti)
allData <- subset(allData, time <= 75)

#plot speed as function of time for different cars
p1 <-	ggplot(allData, aes(x=time, y=speed*3.6, col=factor(car))) +
		geom_line() +
		facet_grid(car~.)
#print(p1)
ggsave('speed.pdf', p1, width=16, height=9)

#plot acceleration as function of time for different cars
p2 <-	ggplot(allData, aes(x=time, y=acceleration, col=factor(car))) +
		geom_line() +
		facet_grid(car~.)
#print(p2)
ggsave('acceleration.pdf', p2, width=16, height=9)
