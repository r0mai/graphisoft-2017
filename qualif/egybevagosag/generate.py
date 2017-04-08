#!/usr/bin/env python

import sys
import argparse
import random

def generate(vertex_count, edge_count, face_count):
    print vertex_count
    for x in range(0, vertex_count):
        print random.randint(0, 100), random.randint(0, 100), random.randint(0, 100)

    print edge_count
    for x in range(0, edge_count):
        print random.randint(0, vertex_count-1), random.randint(0, vertex_count-1)

    print face_count
    for x in range(0, face_count):
        face_edge_count = min(random.randint(1, edge_count-1), 20)
        print face_edge_count
        for y in range(0, face_edge_count):
            print random.randint(0, edge_count-1)
        print

        print 0 # hole count


def main():
    parser = argparse.ArgumentParser(description="whatever")
    parser.add_argument("-v", "--vertex_count", required=True, type=int)
    parser.add_argument("-e", "--edge_count", required=True, type=int)
    parser.add_argument("-f", "--face_count", required=True, type=int)
    parser.add_argument("-b", "--building_count", required=True, type=int)

    args = parser.parse_args()

    print args.building_count
    for x in range(0, args.building_count):
        generate(args.vertex_count, args.edge_count, args.face_count)

if __name__ == "__main__":
    main()
