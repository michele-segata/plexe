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
