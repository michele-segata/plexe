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

#load ggplot for quick and dirty plotting
library(ggplot2)

cntr = c(
   "ACC (0.3 s)",
   "ACC (1.2 s)",
   "CACC",
   "PLOEG",
   "CONSENSUS",
   "FLATBED"
)
#map controller id to name
controller <- function(id, headway) {
    ifelse(id == 0,
        ifelse(headway < 1, cntr[1], cntr[2]),
        cntr[id+2]
    )
}

load('../results/Braking.Rdata')
allData$controllerName <- controller(allData$controller, allData$headway)

p.speed <- ggplot(allData, aes(x=time, y=speed*3.6, col=factor(nodeId))) +
           geom_line() +
           facet_grid(controllerName~., scales='free_y')
ggsave('braking-speed.pdf', p.speed, width=16, height=9)
#print(p.speed)

p.distance <- ggplot(subset(allData, distance != -1), aes(x=time, y=distance, col=factor(nodeId))) +
              geom_line() +
              facet_grid(controllerName~., scales='free_y')
ggsave('braking-distance.pdf', p.distance, width=16, height=9)
#print(p.distance)

p.accel <- ggplot(allData, aes(x=time, y=acceleration, col=factor(nodeId))) +
           geom_line() +
           facet_grid(controllerName~., scales='free_y')
ggsave('braking-acceleration.pdf', p.accel, width=16, height=9)
#print(p.accel)

p.caccel <- ggplot(allData, aes(x=time, y=controllerAcceleration, col=factor(nodeId))) +
            geom_line() +
            facet_grid(controllerName~., scales='free_y')
ggsave('braking-controller-acceleration.pdf', p.caccel, width=16, height=9)
#print(p.caccel)
