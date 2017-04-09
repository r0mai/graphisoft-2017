#!/usr/bin/env python3
from __future__ import print_function
import sys
import argparse


class Input:
    def __init__(self, villages, distances, jumps, timeBudget):
        self.villages = villages
        self.distances = distances
        self.jumps = jumps
        self.timeBudget = timeBudget


class Output:
    def __init__(self, jumps):
        self.jumps = jumps


def parseInput(input):
    N = int(input.readline())
    villages = []
    for i in range(N):
        villages.append(input.readline().rstrip())

    distances = list(map(int, input.readline().split()))

    jumpCount = int(input.readline())

    jumps = []
    for i in range(jumpCount):
        line = input.readline().split()
        jumps.append((line[0], line[1], int(line[2])))

    timeBudget = int(input.readline())

    return Input(villages, distances, jumps, timeBudget)


def parseOutput(output):
    N = int(output.readline())
    jumps = []
    for i in range(N):
        line = output.readline().split()
        jumps.append((line[0], line[1]))

    return Output(jumps)


def checkAllJumpsAreValid(jumps, registeredJumps):
    for jump in jumps:
        for registeredJump in registeredJumps:
            if registeredJump[0] == jump[0] and registeredJump[1] == jump[1]:
                break
        else:
            assert False, (jump, registeredJumps)


def getCostOfJump(input, jump):
    for jump_ in input.jumps:
        if jump_[0] == jump[0] and jump_[1] == jump[1]:
            return jump_[2]
    assert False


def calculateRouteCost(input, output):
    N = len(input.villages)

    currentVillage = input.villages[0]
    currentIndex = 0
    routeCost = 0
    bikeCost = 0
    while True:
        def _findJump():
            for jump in output.jumps:
                if currentVillage == jump[0]:
                    return jump
            return None

        jump = _findJump()
        if jump is None:
            bikeCost += input.distances[currentIndex]
            routeCost += input.distances[currentIndex]
            currentIndex += 1
            currentVillage = input.villages[currentIndex % N]
        else:
            routeCost += getCostOfJump(input, jump)
            currentVillage = jump[1]
            currentIndex = input.villages.index(currentVillage)

        if currentVillage == input.villages[0]:
            return routeCost, bikeCost


def main():
    parser = argparse.ArgumentParser(description="Validation of topart")
    parser.add_argument(
        "-i", "--input", metavar="sample.in", required=True,
        type=argparse.FileType('r'))
    parser.add_argument(
        "-o", "--output", metavar="sample.out", required=True,
        type=argparse.FileType('r'))
    args = parser.parse_args()
    parsedInput = parseInput(args.input)
    parsedOutput = parseOutput(args.output)

    checkAllJumpsAreValid(parsedOutput.jumps, parsedInput.jumps)
    routeCost, bikeCost = calculateRouteCost(parsedInput, parsedOutput)
    assert routeCost <= parsedInput.timeBudget, routeCost
    print('Total: {}'.format(routeCost))
    print('Biked: {}'.format(bikeCost))


if __name__ == "__main__":
    main()
