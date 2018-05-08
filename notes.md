# The Elements of Computing Systems

## Chapter 1: Boolean Logic

## Chapter 4: Machine Language

- Processors can natively support a number of different data types - float,
int8, int32, int64, etc. If specific data types aren't supported, they can be
supported at the software level.
- Memory Hierarchy: Registers, Caches, Main Memory (RAM), Disk.
- Registers reside within the CPU.
- Busses connect things like CPU and Memory.
- Hack has three registers D, A, and M.
  - D is for data values.
  - A is for addresses.
  - M holds the value that A points to.
- `@21` will load 21 into the A register and also point M to A's value in RAM.
- Hack words are 16 bits.
- Keyboard and screen are memory mapped into RAM.
- Use the following approach to manipulate bits: `Screen[32 * row + col / 16]`.

## Chapter 6: Assembler

- The goal of the assembler is to translate the symbolic assembly code into
binary code for the actual hardware.
- The assembler also translates variables and labels to their respective
locations in memory.
- See [assembler/src/assembler.c](assembler/src/assembler.c)
