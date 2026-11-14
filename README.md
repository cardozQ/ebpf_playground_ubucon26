# eBPF Performance Playground

A small, beginner-friendly Linux tracing playground for a live bpftrace demonstration.

The project uses one C workload and three bpftrace scripts:

1. **Level 1 — Observe:** count `openat()` calls by process.
2. **Level 2 — Measure:** measure `openat()` duration and build a latency histogram.
3. **Level 3 — Investigate:** correlate syscall entry/exit, filename, return value and latency.

> This playground targets **Linux**. bpftrace/eBPF scripts do not run on FreeBSD as-is.

## Requirements

- Linux kernel with eBPF and tracing support
- `bpftrace`
- GCC or Clang
- `make`
- Root privileges (normally `sudo`) for tracing

On Debian/Ubuntu/Parrot:

```bash
sudo apt update
sudo apt install bpftrace build-essential
```

Check the environment:

```bash
uname -r
bpftrace --version
sudo bpftrace -l 'tracepoint:syscalls:sys_enter_openat'
```

If the last command returns no probe, your kernel or tracing setup may not expose that tracepoint.

## Build the workload

From the project root:

```bash
make
```

This creates `build/ebpf_workload`.

Run it once:

```bash
./build/ebpf_workload
```

The program repeatedly creates, writes, reads and removes a temporary file. It also runs a tiny CPU loop and, by default, starts `/bin/echo` as a child process.

Useful options:

```bash
./build/ebpf_workload --iterations 10
./build/ebpf_workload --no-child
./build/ebpf_workload --help
```

## Run the demos

Use **two terminals**. Start the bpftrace script in Terminal A, then run the workload in Terminal B.

### Level 1 — Count file opens

Terminal A:

```bash
sudo bpftrace scripts/01_count_openat.bt
```

Terminal B:

```bash
./build/ebpf_workload
```

Stop the tracer with `Ctrl+C` to print the aggregation.

### Level 2 — Measure syscall latency

Terminal A:

```bash
sudo bpftrace scripts/02_openat_latency.bt
```

Terminal B:

```bash
./build/ebpf_workload
```

Stop with `Ctrl+C` to print the histogram.

### Level 3 — Investigate individual operations

Terminal A:

```bash
sudo bpftrace scripts/03_openat_investigation.bt
```

Terminal B:

```bash
./build/ebpf_workload --iterations 5
```

This prints one line per matching `openat()` call, including the process, filename, return value and measured duration.

## What the output means

- `openat()` is a Linux system call used to open a file relative to a directory file descriptor.
- `sys_enter_openat` fires when execution enters the kernel for this syscall.
- `sys_exit_openat` fires when the syscall returns to user space.
- `nsecs` is a bpftrace built-in timestamp in nanoseconds.
- `hist()` creates a logarithmic histogram of values.
- A non-negative return value from `openat()` is a file descriptor; a negative return value indicates an error.

The measured interval is the duration of the syscall from the kernel tracepoint's entry to exit. It is **not automatically equal to physical disk latency**: page cache, filesystem metadata, scheduling and other effects can contribute.

## Important demo notes

- Run only one level at a time to keep the output understandable.
- The scripts filter on `comm == "ebpf_workload"`. Linux task command names are limited in length; this executable name fits.
- The child `/bin/echo` process has a different `comm`, so it is not included in the workload-only syscall counts.
- If you want to trace all processes, remove the `comm` filter from the scripts.
- Tracing has overhead. Keep probes targeted and avoid claiming that tracing is free.
- If your machine blocks BPF due to kernel configuration, lockdown, containers, or security policy, use the saved output examples in `docs/demo-notes.md` as your verbal fallback.

## Project layout

```text
ebpf-performance-playground/
├── README.md
├── Makefile
├── src/
│   └── workload.c
├── scripts/
│   ├── 01_count_openat.bt
│   ├── 02_openat_latency.bt
│   └── 03_openat_investigation.bt
└── docs/
    └── demo-notes.md
```

## Suggested GitHub description

A hands-on Linux performance tracing playground: learn bpftrace through a small C workload, from syscall counting to latency histograms and per-operation investigation.
