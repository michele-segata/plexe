#!/usr/bin/env python2
#
# Copyright (C) 2017-2019 Luca Terruzzi <luca.terruzzi@studenti.unitn.it>
# Copyright (C) 2017-2019 Riccardo Colombo <riccardo.colombo@studenti.unitn.it>
# Successive modifications by Michele Segata <segata@ccs-labs.org>
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

import argparse
import math
import subprocess
import sys
from elementtree.ElementTree import parse


def get_segment_length(r, edges):
    return 2 * r * math.sin(math.pi / edges)


def get_half_circle_lane_length(r, edges, n_lanes, lane, lane_width):
    center = n_lanes - 1 + 0.5
    add_radius = (lane - center) * lane_width
    segment_length = get_segment_length(r + add_radius, edges)
    return segment_length * edges / 2


def write_edge(f, index, node_from, node_to, lanes, speed, shape=None):
    shape_str = ""
    if shape is not None:
        shape_str = " shape=\"%s\"" % shape
    f.write("    <edge id=\"edg%d\" from=\"%s\" to=\"%s\" numLanes=\"%d\""
            " speed=\"%f\"%s>\n" %
            (index, node_from, node_to, lanes, speed, shape_str))


def end_edge(f):
    f.write("    </edge>\n")


def write_node(f, index, x, y):
    f.write("    <node id=\"node%d\" x=\"%f\" y=\"%f\"/>\n" % (index, x, y))


def offset(v, x, y):
    if type(v) == list:
        v = [(vx + x, vy + y) for (vx, vy) in v]
    else:
        v[0] += x
        v[1] += y
    return v


def zero_offset(v):
    xv = [x for (x, y) in v]
    yv = [y for (x, y) in v]
    return -min(xv), -min(yv)


def fix_net_file(net_file, r, edges, n_lanes, lane_width):
    xml = parse(net_file)
    lanes = xml.findall("edge/lane")
    for l in lanes:
        if l.get("id")[:4] in ["edg2", "edg4"]:
            edge, lane = l.get("id").split("_")
            lane = int(lane)
            length = get_half_circle_lane_length(r, edges, n_lanes, lane,
                                                 lane_width)
            l.set("length", str(length))
    with open(net_file, "w") as out_file:
        xml.write(out_file)


# Parser for retrieving command arguments
parser = argparse.ArgumentParser(description="Create a circular road to be "
                                             "used in SUMO.")
parser.add_argument('-r', '--radius', type=float, default=100,
                    help='Radius of the circle in metres (min 10). Default'
                    '100m.')
parser.add_argument('-n', '--name', help='Name of the output files.',
                    default="circle")
parser.add_argument('-e', '--edges', help='Number of edges. Default 10.',
                    type=int, default=10)
parser.add_argument('-l', '--lanes', help='Number of lanes. Default 1.',
                    type=int, default=1)
parser.add_argument('-s', '--speed', type=float, default=13.9,
                    help='Maximum speed allowed in m/s. Default 13.9 m/s.')
parser.add_argument('-d', '--dimension', type=float,
                    help='Total length of the ring. If present overrides '
                         'radius.')
parser.add_argument('-S', '--straight', type=float,
                    help='Length of the straight', default=1000)
args = parser.parse_args()


# Arguments validity check
if args.dimension is not None:
    if args.dimension <= 0:
        print("Ring dimension must be positive")
        print("Exiting on error")
        exit(1)
    args.radius = (args.dimension / args.edges) / (2 * math.sin(math.pi /
                                                                args.edges))

if args.radius < 10 and args.dimension is not None:
    print("Radius is too small")
    print("Exiting on error")
    exit(1)

if args.edges < 3:
    print("Too few edges to construct a circle")
    print("Exiting on error")
    exit(1)

if args.lanes < 1:
    print("There must be at least one lane")
    print("Exiting on error")
    exit(1)

if args.speed < 0:
    print("Speed must be positive")
    print("Exiting on error")
    exit(1)

side = get_segment_length(args.radius, args.edges)
if side < 1:
    print("Edge length is too small!. Choose a bigger radius/dimension or "
          "reduce the number of edges.")
    print("Exiting on error")
    exit(1)

delta_x = args.straight / 2

n1 = (-delta_x, args.radius)
n2 = (delta_x, args.radius)
n3 = (delta_x, -args.radius)
n4 = (-delta_x, -args.radius)
nodes = [n1, n2, n3, n4]

c1 = [n2]
c2 = [n4]
for i in range(1, args.edges/2+1):
    angle = float(i) / (args.edges/2) * math.pi + math.pi/2
    c1.append((-math.cos(angle)*args.radius+delta_x,
               math.sin(angle)*args.radius))
    angle = float(args.edges/2-i) / (args.edges/2) * math.pi + math.pi/2
    c2.append((math.cos(angle)*args.radius-delta_x,
               math.sin(angle)*args.radius))

all_points = list(c1)
all_points.extend(c2)
off_x, off_y = zero_offset(all_points)

c1 = offset(c1, off_x, off_y)
c2 = offset(c2, off_x, off_y)

shape1 = c1
shape2 = c2

print("x,y")
for v in shape1:
    print("{},{}".format(v[0], v[1]))
for v in shape2:
    print("{},{}".format(v[0], v[1]))

coord1 = ' '.join(["%f,%f" % x for x in shape1])
coord2 = ' '.join(["%f,%f" % x for x in shape2])

print("Creating node file..."),
gen_string = "<!-- Generated with %s -->\n" % (' '.join(sys.argv))
# Write the two nodes in the node file
with open(args.name + ".nod.xml", "w") as node_file:
    node_file.write(gen_string)
    node_file.write("<nodes>\n")
    nodes = offset(nodes, off_x, off_y)
    for i in range(len(nodes)):
        write_node(node_file, i+1, nodes[i][0], nodes[i][1])
    node_file.write("</nodes>\n")

print("OK")
print("Creating edge file..."),
# Write the two edges in the edge file
with open(args.name + ".edg.xml", "w") as edge_file:
    edge_file.write(gen_string)

    edge_file.write("<edges>\n")

    write_edge(edge_file, 1, "node1", "node2", args.lanes, args.speed)
    end_edge(edge_file)

    write_edge(edge_file, 2, "node2", "node3", args.lanes, args.speed, coord1)
    for i in range(args.lanes):
        length = get_half_circle_lane_length(args.radius, args.edges,
                                             args.lanes, i, 3.2)
        edge_file.write("        <lane index=\"%d\" length=\"%f\"/>\n" %
                        (i, length))
    end_edge(edge_file)

    write_edge(edge_file, 3, "node3", "node4", args.lanes, args.speed)
    end_edge(edge_file)

    write_edge(edge_file, 4, "node4", "node1", args.lanes, args.speed, coord2)
    for i in range(args.lanes):
        length = get_half_circle_lane_length(args.radius, args.edges,
                                             args.lanes, i, 3.2)
        edge_file.write("        <lane index=\"%d\" length=\"%f\"/>\n" %
                        (i, length))
    end_edge(edge_file)

    edge_file.write("</edges>\n")

print("OK")
print("Building net file..."),

# Launch SUMO command netconvert to create the network from nod end edg files
status = 0
net_file = args.name + ".net.xml"
try:
    status = subprocess.call(["netconvert", "-e", args.name + ".edg.xml",
                              "-n", args.name + ".nod.xml", "-o", net_file,
                              "--precision", "6"])
except OSError as e:
    print("Error in launching SUMO netconvert, is SUMO bin folder present in"
          "your system PATH?")
    print("Exiting on error")

# Check for return status of SUMO netconvert
if status != 0:
    print("Error in network creation")
    print("Exiting on error")
    exit(2)

print("OK")
print("Creating additional file..."),
# Write the necessary routes and rerouters in thet additional file
with open(args.name + ".add.xml", "w") as additional_file:
    additional_file.write(gen_string)
    additional_file.write("<additionals>\n")
    additional_file.write("    <route id=\"r1\" edges=\"edg1 edg2 edg3 "
                          "edg4\"/>\n")
    additional_file.write("    <route id=\"r2\" edges=\"edg3 edg4 edg1 "
                          "edg2\"/>\n")
    additional_file.write("    <rerouter id=\"rerouter1\" edges=\"edg1\">\n")
    additional_file.write("        <interval end=\"1e9\">\n")
    additional_file.write("            <routeProbReroute id=\"r1\"/>\n")
    additional_file.write("        </interval>\n")
    additional_file.write("    </rerouter>\n")
    additional_file.write("    <rerouter id=\"rerouter2\" edges=\"edg3\">\n")
    additional_file.write("        <interval end=\"1e9\">\n")
    additional_file.write("            <routeProbReroute id=\"r2\"/>\n")
    additional_file.write("        </interval>\n")
    additional_file.write("    </rerouter>\n")
    additional_file.write("</additionals>\n")

print("OK")

fix_net_file(net_file, args.radius, args.edges, args.lanes, 3.2)
length = get_half_circle_lane_length(args.radius, args.edges, args.lanes, 0,
                                     3.2)

print("Network length -> %f" % (length * 2 + 2 * args.straight))
print("Success.")
