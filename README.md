A compiler for the Elm language written in ANSI C99.


## Usage

The compiler is intended to be used either as a standalone compiler and
virtual machine for executing Elm code, or embedded as a static or dynamic
library in a larger program.


## Features

The compiler is currently targeting Elm 0.19, but is not yet feature complete.


### Language features

The following core language features have been implemented so far:

- [x] Single line comments
- [ ] Multiline comments
- [x] Boolean literals
- [x] Int and Float literals
- [x] String and Char literals
- [ ] Multiline string literals
- [x] Expressions
- [ ] Indentation sensitive parsing
- [ ] Infix operators
- [x] Lists
- [x] Tuples
- [ ] Records
- [x] If-then-else conditionals
- [x] Case-of conditionals
- [ ] Pattern matching
- [x] Function declarations
- [ ] Anonymous functions
- [ ] Structural typing (function arguments)
- [x] Function application
- [ ] Higher-order functions
- [ ] Arithmethic operators with type inference
- [ ] Let expressions
- [ ] Modules
- [ ] Qualified imports
- [ ] Type annotations
- [ ] Type aliases
- [ ] Custom types
- [ ] Ports
- [ ] Effect managers


### Kernel

The kernel contains native implementations of some core functions used by
packages in the `elm` namespace (e.g. `elm/core` and friends). These packages
typically contain native code written in Javascript. The elmc compiler instead
provides versions of these functions written in C or Assembler.


### Optimizations

When compiling Elm code the compiler will generate an Intermediate
Representation (IR) of the source code. This IR is then analysed and optimized
before being passed to the code generator step. The following optimizations
are currently implemented:


### Just-in-Time (JIT) compiler

After the optimization step the IR is transformed to executable machine code
by the JIT compiler.

