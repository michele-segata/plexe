#load ggplot for quick and dirty plotting
library(ggplot2)

#map controller id to name
car <- function(id) {
    ifelse(id == 0, "Alfa 147",
        ifelse(id == 1, "Audi R8", "Bugatti Veyron")
    )
}

load('../results/EngineTest.Rdata')
allData$car <- car(allData$vehicle)

p.speed <- ggplot(allData, aes(x=time, y=speed*3.6, col=factor(car))) +
           geom_line()
ggsave('engine-speed.pdf', p.speed, width=16, height=9)
#print(p.speed)

p.accel <- ggplot(allData, aes(x=time, y=acceleration, col=factor(car))) +
           geom_line()
ggsave('engine-acceleration.pdf', p.accel, width=16, height=9)
#print(p.accel)
