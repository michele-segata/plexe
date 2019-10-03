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
