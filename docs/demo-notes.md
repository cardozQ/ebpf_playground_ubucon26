# Presenter notes

## Opening the demo

"We have a workload that repeatedly opens, writes, reads and closes a file. Rather than guessing what it does, let's observe it."

## Level 1 — Observe

Question: "Which process is making `openat()` calls, and how many?"

Explain:
- `tracepoint:syscalls:sys_enter_openat` is the event.
- `comm` is the current task's command name.
- `count()` aggregates the number of events.
- `@[comm]` is a bpftrace map.

## Level 2 — Measure

Question: "How long does `openat()` take?"

Explain:
- Save a timestamp at syscall entry, keyed by thread ID.
- At syscall exit, subtract the saved timestamp from `nsecs`.
- Convert nanoseconds to microseconds.
- `hist()` shows the distribution, not just one average.

## Level 3 — Investigate

Question: "For each operation, what file was requested, did it succeed, and how long did it take?"

Explain:
- Entry and exit events are correlated using `tid`.
- The filename is copied from the tracepoint argument at entry.
- `args->ret` is the syscall return value.
- Negative return values indicate errors.
- The histogram summarizes the durations.

## Safety and accuracy notes

- This demo measures syscall duration, not isolated physical disk latency.
- The workload uses a temporary file under `/tmp` and removes it when it finishes.
- The child command is a fixed `/bin/echo`, not arbitrary shell input.
- bpftrace syntax and available tracepoints can vary by version and kernel configuration.
