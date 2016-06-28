# Using perf

---

# What is it?

 - Performance tools for linux
 - Designed to profile kernel, but can profile userspace apps
 - Sampling based
 - Canonized in linux kernel source code:

```bash
$ git clone https://github.com/torvalds/linux.git
$ cd linux/tools/perf;
```
    
---

# Before running perf

 - Compile your executable with symbols (`-g`)
 - Add the `-fno-omit-frame-pointer` compiler flag, unless you know that it gives you an unacceptable performance hit

---

# Basic Usage:

```bash
$ perf record -g ./run_benchmarks.x
$ perf report -g -M intel
```

