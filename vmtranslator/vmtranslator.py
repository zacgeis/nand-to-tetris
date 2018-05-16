import sys
import argparse
import os

ADDRESSES = {
    "sp": 0,
    "local": 1,
    "argument": 2,
    "this": 3,
    "that": 4,
    "temp": 5,
    "general": 13,
}

def parseLine(line):
    result = line.strip()
    result = result.split("//")[0]
    if len(result) == 0: return []
    result = result.split(" ")
    result = [s for s in result if len(s) > 0]
    return result

def compileVmInst(inst):
    op = inst[0]
    result = []
    if op == "push":
        segment = inst[1]
        value = inst[2]
        result.append("@{}".format(value))
        result.append("D=A")
        result.append("@{}".format(ADDRESSES[segment]))
        result.append("A=M")
        result.append("M=D")
        result.append("@{}".format(ADDRESSES[segment]))
        result.append("M=M+1")
        return result
    if op == "pop":
        segment = inst[1]
        value = inst[2]
        result.append("@{}".format(value))
        result.append("D=A")
        result.append("@{}".format(ADDRESSES[segment]))
        result.append("A=M")
        result.append("M=D")
        result.append("@{}".format(ADDRESSES[segment]))
        result.append("M=M+1")
        return result

def main():
    if len(sys.argv) < 2:
        print("ERROR: Missing single path argument")
        sys.exit(1)

    basePath = sys.argv[1]
    vmFiles = []
    if os.path.isdir(basePath):
        for name in os.listdir(basePath):
            vmFiles.append(os.path.join(basePath, name))
    else:
        vmFiles.append(basePath)

if __name__ == "__main__":
    main()
