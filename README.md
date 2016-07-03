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


---

# perf gotchas

perf sometimes attributes the time in a single instruction to the *next* instruction.

    Example:

```bash
     │         if (absx < 1)
7.76 │       ucomis xmm1,QWORD PTR [rbp-0x20]
0.95 │     ↓ jbe    a6
1.82 │       movsd  xmm0,QWORD PTR ds:0x46a198
0.01 │       movsd  xmm1,QWORD PTR ds:0x46a1a0
0.01 |       movsd  xmm2,QWORD PTR ds:0x46a100 
```

Hmm, so one the first move into the register is 182x slower than the second, and it's right after a jump instruction . . .

Looks like a misattribution of time.


---

# Generating wins with perf: dot product example

Let's first compile the following dot product at -O0 and see what the compiler does with it:

```cpp
template<typename Real>
Real dot_product(const Real * const a, const Real * const b, size_t n)
{
    Real s = 0;
    for(size_t i = 0; i < n; ++i)
    {
        auto tmp = a[i]*b[i];
        s += tmp;
    }
    return s;
}
```

---

# Dot product example: O0

![Basic usage of perf](figures/dot_product_O0.png?raw=true "Dot product at -O0")


---

# Dot product example:

Quick note about Linux's System V ABI:

- A floating point return value is placed in register `xmm0`.
- The first integer argument is placed in `rdi`
- The second integer argument is placed in `rsi`
- The third integer argument is placed in `rdx`


---

# Dot product example:

Some comments about our assembly:

```nasm
Real dot_product(const Real * const a, const Real * const b, size_t n)
{
push   rbp
mov    rbp, rsp                 ; Establish a stack frame for our function
xorps  xmm0, xmm0               ; Clear out xmm0, as it will be used for our return value
mov    QWORD PTR [rbp-0x8],rdi  ; rdi is first argument; place address of a on stack
mov    QWORD PTR [rbp-0x10],rsi ; rsi is second argument; place address of b on stack
mov    QWORD PTR [rbp-0x18],rdx ; rdx is third argument; place length of array on stack
```

I personally see no good reason to copy the integer registers to the stack, but perhaps this is a -O0 artefact.


---

# Dot product example:

```nasm
Real s = 0;
movsd  QWORD PTR [rbp-0x20],xmm0
```

This sets `s` to zero and places it on the stack.

Again I see no reason for `s` to ever leave the `xmm0` register.

---

# Dot product example

```nasm
for (size_t i = 0; i < n; ++i)
      mov    QWORD PTR [rbp-0x28],0x0            ; set i = 0, but why not use a count register?
20:   mov    rax,QWORD PTR [rbp-0x28]            ; copy i into rax, note that this line is the target of a jump
      cmp    rax,QWORD PTR [rbp-0x18]            ; compare i with n
      jae    6d                                  ; jump to 6d if i is "after or equal" to n
      ; stuff . . .
6d:   movsd  xmm0,QWORD PTR [rbp-0x20]           ; move s to xmm0 (System V abi convention)
      pop    rbp                                 ; increment base pointer
      ret                                        ; return control to caller
```


# Dot product example

```nasm
auto tmp = a[i]*b[i];
mov    rax,QWORD PTR [rbp-0x28]          ; copy i into rax *again*!
mov    rcx,QWORD PTR [rbp-0x8]           ; move address of a into rcx, why didn't we just leave it in rdi?
movsd  xmm0,QWORD PTR [rcx+rax*8]        ; move a[i] into xmm0
mov    rax,QWORD PTR [rbp-0x28]          ; copy i into rax *again*!
mov    rcx,QWORD PTR [rbp-0x10]          ; move address of b into rcx, why didn't we just leave it in rsi?
mulsd  xmm0,QWORD PTR [rcx+rax*8]        ; a[i]*b[i]
movsd  QWORD PTR [rbp-0x30],xmm0         ; push tmp onto stack
s += tmp;
movsd  xmm0,QWORD PTR [rbp-0x30]         ; move tmp from stack back to where it was
addsd  xmm0,QWORD PTR [rbp-0x20]         ; add tmp to s
movsd  QWORD PTR [rbp-0x20],xmm0         ; mov s back to stack
```

---

# Dot product example

```nasm
mov    rax,QWORD PTR [rbp-0x28]         ; copy i into rax a fourth time?!!
add    rax,0x1                          ; i = i + 1
mov    QWORD PTR [rbp-0x28],rax         ; copy rax back into stack
jmp    20                               ; go to top of loop
```

---

# Dot product example

The compiler basically screwed up everything. Benchmarking put it the timing a 2.55N nanoseconds, where N is the array length.


---

# Dot product example

The following uses more registers, and benchmarks at 0.8N nanoseconds:


```cpp
double easy_asm_dot_product(const double * const a, const double * const b, size_t n)
{
    double s = 0;
    asm volatile(".intel_syntax noprefix;"
                 "mov rdx, QWORD PTR [rbp - 0x18];"
                 "xorps xmm0, xmm0;"
                 "xor rax, rax;" // set i = 0
                 "begin: cmp rax, rdx;" // compare i to n
                 "jae end;"
                 "movsd xmm1, QWORD PTR [rdi + 8*rax];" // move a[i] into xmm1
                 "movsd xmm2, QWORD PTR [rsi + 8*rax];" // move b[i] into xmm2
                 "mulsd xmm2, xmm1;" // a[i]*b[i] in xmm2
                 "addsd xmm0, xmm2;" // s += a[i]*b[i]
                 "inc rax;"          // i = i + 1
                 "jmp begin;"        // jump to top of loop
                 "end: nop;"
                 :
                 : "r" (&s)
                 );
    return s;
}
```
