# Operating Systems

Homework assignments from an Operating Systems course. Each `ExN` folder is a separate assignment.

## Ex1 - Bash and Python scripting

A chess game viewer for PGN files, written once in Bash (`chess_sim.sh`) and once in Python (`chess_sim.py`). It loads a game and lets you step forward and backward through the moves on a text board. Includes a script that splits a multi-game PGN file into separate files, plus a tester and sample games.

## Ex2 - Processes and file I/O in C

- `part1.c`, `part2.c` - working with `fork()` and `wait()`: several child processes write to a shared output in a controlled order.
- `buffered_open.c` - a wrapper around `open`/`read`/`write` that adds read and write buffering, and supports a custom `O_PREAPPEND` flag for writing at the start of a file.

## Ex3 - Concurrency and synchronization (C++)

A news broadcast simulation built on producer-consumer: producers, a dispatcher, co-editors and a screen manager that communicate through bounded buffers, synchronized with a counting semaphore implemented from scratch. The assignment spec is in `Ex3/README.md`.

## Ex4 - xv6 kernel changes

- `Part1-xv6` - a priority-based scheduler with a new `set_ps_priority` system call, and test programs that run processes with different priorities.
- `Part2-xv6` - kernel semaphores (`sem_create`, `sem_free`, `sem_down`, `sem_up`) with user-space tests.

## Ex5 - File systems

- `copytree.c`, `part3.c` - recursive directory copy in C, with options to copy symbolic links and file permissions.
- `xv6-ex5` - file permissions in xv6, with new `chmod` and `getperm` system calls and tests.
- `tests` - Python test scripts for the assignment.

## Running

The xv6 assignments run on QEMU (RISC-V). From inside the relevant folder:

```
make qemu
```

Each xv6 folder also has a `.devcontainer` with a ready-to-use environment.
