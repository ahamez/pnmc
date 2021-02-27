# Presentation

Pnmc is a symbolic model checker for Petri nets.
The symbolic part is handled by the [libsdd](https://github.com/ahamez/libsdd) library which implements Hierarchical Set Decision Diagrams.
It aims to be as efficient as possible, memory and CPU-wise. In this regard, it uses a very efficient technique called saturation
to optimize the generation of state spaces.


## Features

- Symbolic generation of state spaces
- Verification of dead states (deadlocks)
- Verification of dead transitions
- Additionally, it can generate the state space of a Time Petri nets using discrete time semantics.

## Installation

- Instructions to compile and install pnmc are given in the INSTALL file.
