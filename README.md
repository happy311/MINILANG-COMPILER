# MiniLang Compiler (minicc)

A tiny educational compiler written in C++17 (~600 lines). It compiles a small
language to stack-machine bytecode and runs it on a built-in VM.

## Pipeline

```
source --> Lexer --> Parser --> IR generator --> Optimizer --> Code generator --> VM
          tokens     AST      3-address code    better IR      bytecode        output
```

| File                | Stage | What it does                                              |
|---------------------|-------|-----------------------------------------------------------|
| `src/lexer.*`       | 1     | Splits text into tokens (numbers, names, keywords, ...)   |
| `src/parser.*`      | 2     | Recursive-descent parser, builds the AST (`src/ast.*`)    |
| `src/ir.*`          | 3     | Flattens the AST into three-address code, checks variables |
| `src/optimizer.*`   | 4     | Constant propagation/folding + dead temporary removal     |
| `src/codegen.*`     | 5     | IR -> stack-machine bytecode, resolves jump addresses     |
| `src/vm.*`          | 6     | Executes the bytecode                                     |
| `src/main.cpp`      | -     | Command-line driver                                       |

## Build and run

```
make                                  # or: g++ -std=c++17 src/*.cpp -o minicc
./minicc examples/sum.ml              # compile + run
./minicc examples/constants.ml --ir --asm
make run-examples
```

Options: `--ir` (show IR before/after optimization), `--asm` (show bytecode),
`--no-opt` (skip optimizer), `--no-run` (compile only).

## The language

```
let x = 5;                  // declare a variable (integers only)
x = x * 2 + 1;              // assign
print x;                    // output a value
if (x > 3) { ... } else { ... }
while (x < 100) { ... }
```

Operators: `+ - * /` and comparisons `< > <= >= == !=` (result is 1 or 0).
Unary minus (`-x`) and parentheses work. `//` starts a comment.
Variables are global (no block scopes) and must be declared with `let` once.

## Example: what the optimizer does

`examples/constants.ml`:

```
Before                    After
%t1 = 3 * 4               a = 14
%t2 = 2 + %t1             b = 28
a = %t2                   print 28
%t3 = a * 2               print 3
b = %t3
print b
%t4 = 10 - 4
%t5 = %t4 / 2
print %t5
```

How it works:
1. **Constant propagation:** if `a = 14` is known, later uses of `a` become `14`.
2. **Constant folding:** `3 * 4` is computed at compile time -> `12`.
3. **Dead temporary elimination:** `%t` values nobody reads are deleted.
4. Knowledge is forgotten at labels (loop tops / branch joins) so loops stay correct.

## Ideas to extend it (good for learning)

- Add a `for` loop or `%` operator (touch lexer, parser, IR)
- Add algebraic simplifications (`x * 1 -> x`, `x + 0 -> x`)
- Remove unreachable code after `goto`
- Emit real x86-64 or RISC-V assembly instead of bytecode
- Replace the stack machine with register allocation
