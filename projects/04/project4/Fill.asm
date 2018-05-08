// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/04/Fill.asm

// Runs an infinite loop that listens to the keyboard input.
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel;
// the screen should remain fully black as long as the key is pressed.
// When no key is pressed, the program clears the screen, i.e. writes
// "white" in every pixel;
// the screen should remain fully clear as long as no key is pressed.

(LOOP)
  @0 // Adjust this to change the starting point
  D=A
  @0
  M=D

  @24576
  D=M
  @BLACK
  D;JNE
  @WHITE
  0;JMP

(BLACK)
  @1
  M=-1
  @FILL
  0;JMP

(WHITE)
  @1
  M=0
  @FILL
  0;JMP

(FILL)
  @SCREEN
  D=A
  @0
  D=D+M
  @2
  M=D
  @1
  D=M
  @2
  A=M
  M=D

  @0
  D=M
  @8191
  D=A-D

  @0
  M=M+1

  @FILL
  D;JGT

@LOOP
0;JMP
