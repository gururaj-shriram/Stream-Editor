Stream Editor (svi)

This was the final project of the csc322 class from Fall 2015

svi.c takes a file with a list of edit commands as an argument in the command line. The standard input is then edited (unless an input file is redirected. svi is similar yet basic compared to the Unix command sed.

A command in the edit file contains
- a line range specification
- a type of edit
- supplemental data

The line range specification can be 
- line range
- text
- no specification

The type of edits are
- (A)ppend text to the line
- (I)nsert at the beginning of the line (prepend)
- (O): Inserting text on the line prior
- (d)eleting the specified lines
- (s): s/OLD/NEW replaces OLD with NEW

A sample edit file and input file is attached

Running the svi is done by first compiling svi and supplying the command file as a command line argument
E.g.

```
gcc svi.c -o svi
./svi command1.txt < input1.txt
```
