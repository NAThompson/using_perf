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

# Installing perf:

```bash
$ sudo apt install linux-tools-common linux-tools-generic linux-tools-`uname -r`
```

Perf is linux-kernel specific; hence the `uname -r`.

---

# Before running perf

 - Compile your executable with symbols (`-g`)
 - Add the `-fno-omit-frame-pointer` compiler flag, unless you know that it gives you an unacceptable performance hit

---

# Basic Usage: `perf stat`:

```bash
$ perf stat ./run_benchmarks.x
Run on (8 X 2054.61 MHz CPU s)
2016-06-28 08:31:07
Benchmark                            Time           CPU Iterations
------------------------------------------------------------------
BM_dot_product<double>/8          1699 ns       1253 ns     603448

 Performance counter stats for './run_benchmarks.x':

      10876.560045      task-clock (msec)         #    1.000 CPUs utilized          
                12      context-switches          #    0.001 K/sec                  
                 0      cpu-migrations            #    0.000 K/sec                  
               195      page-faults               #    0.018 K/sec                  
    40,611,101,450      cycles                    #    3.734 GHz                    
    27,928,382,315      stalled-cycles-frontend   #   68.77% frontend cycles idle   
   <not supported>      stalled-cycles-backend   
    11,752,988,740      instructions              #    0.29  insns per cycle        
                                                  #    2.38  stalled cycles per insn
     1,092,239,732      branches                  #  100.421 M/sec                  
        30,731,372      branch-misses             #    2.81% of all branches        

      10.877577427 seconds time elapsed

```

--- 

# `perf stat`: Explanation

- `task-clock`: How long the program ran
- `context-switches`: How many times this program was stopped so another process could progress
- `cpu-migrations`: How many times this program was moved from one core to another
- `page-faults`: How many times the operating system had to give the process more memory during its course of execution
- `stalled-cycles-frontend`: Cycles idle while instructions are being slowly or branch mispredictions
- `stalled-cycles-backend`: Cycles idle while waiting for data to be fetched from RAM or long-running instructions completing
- `branches`: Number of `jmp` instructions hit during execution
- `branch-misses`: Number of times speculative execution was incorrect.

---

# `perf stat`: It's annoying

It's really good at telling you your code is terrible, but can't be bothered to tell you what to do about it.


---

# Branch misprediction with `perf stat`

```bash
$ perf stat -B run_benchmarks.x
185,665,839,558      branches                  #  979.910 M/sec                  
  1,068,785,447      branch-misses             #    0.58% of all branches
```

---

# `perf stat`

You can get a list of all hardware counters perf can record:

```bash
$ perf list
List of pre-defined events (to be used in -e):

  branch-instructions OR branches                    [Hardware event]
  branch-misses                                      [Hardware event]
  bus-cycles                                         [Hardware event]
  cache-misses                                       [Hardware event]
  cache-references                                   [Hardware event]
  cpu-cycles OR cycles                               [Hardware event]
  instructions                                       [Hardware event]
  ref-cycles                                         [Hardware event]
  stalled-cycles-frontend OR idle-cycles-frontend    [Hardware event]

  alignment-faults                                   [Software event]
  bpf-output                                         [Software event]
  context-switches OR cs                             [Software event]
  cpu-clock                                          [Software event]
  cpu-migrations OR migrations                       [Software event]
  dummy                                              [Software event]
  emulation-faults                                   [Software event]
  major-faults                                       [Software event]
  minor-faults                                       [Software event]
  page-faults OR faults                              [Software event]
  task-clock                                         [Software event]

  L1-dcache-load-misses                              [Hardware cache event]
  L1-dcache-loads                                    [Hardware cache event]
  L1-dcache-prefetch-misses                          [Hardware cache event]
  L1-dcache-store-misses                             [Hardware cache event]
  L1-dcache-stores                                   [Hardware cache event]
  L1-icache-load-misses                              [Hardware cache event]
  LLC-loads                                          [Hardware cache event]
  LLC-prefetches                                     [Hardware cache event]
  LLC-stores                                         [Hardware cache event]
  branch-load-misses                                 [Hardware cache event]
  branch-loads                                       [Hardware cache event]
  dTLB-load-misses                                   [Hardware cache event]
  dTLB-loads                                         [Hardware cache event]
  dTLB-store-misses                                  [Hardware cache event]
  dTLB-stores                                        [Hardware cache event]
  iTLB-load-misses                                   [Hardware cache event]
  iTLB-loads                                         [Hardware cache event]
```


---

# `perf stat`

Any of the events in `perf list` can be counted via

```bash
$ perf stat -e cache-references,cache-misses,L1-dcache-load-misses,L1-dcache-loads ./run_benchmarks.x
        72,239,195      cache-references                                              (50.00%)
         5,526,441      cache-misses              #    7.650 % of all cache refs      (66.67%)
       571,380,771      L1-dcache-load-misses     #    0.18% of all L1-dcache hits    (66.67%)
   320,099,165,220      L1-dcache-loads                                               (66.66%)
```

---

# Basic Usage `perf record`:

```bash
$ perf record -g ./run_benchmarks.x
$ perf report -g -M intel
```

![Basic usage of perf](figures/basic_perf.png?raw=true "Title")


`-M intel` spits the dissassembled code out in Intel syntax (GNU syntax is grotesque), `-g` creates a callgraph.

---

# Basic Usage: `perf record`:


`perf record` creates a file called `perf.data`. `perf report` reads this file, and tells you about your program:

```bash
$ perf report -g -U -M intel
```


---

# Self and Children

The `Self` column says how much time was taken within the function. The `Children` column says how much time was spent in functions called by the function.

If the `Children` column is very near the `Self` column, that function isn't your hotspot!

If `Self` and `Children` is confusing, just get rid of it:

```bash
$ perf report -g -U -M intel --no-children
```


---

# Viewing assembly:

Navigate to a function (say, `foo`) you want to analyze, and hit `Enter`. Then highlight `Annotate foo`, and hit enter again.

You will see the generated assembly:

![Generated Assembly](figures/inline_assembly.png?raw=true "Generated Assembly")

---

# Keyboard Commands for working with disassembler output:

- `k`: Show line numbers of source code
- `o`: Show instruction number
- `t`: Switch between percentage and samples
- `J`: Number of jump sources on target; number of places that can jump here.
- `s`: Hide/Show source code
