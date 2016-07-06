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
$ sudo apt install linux-tools-common
$ sudo apt install linux-tools-generic
$ sudo apt install linux-tools-`uname -r`
```

Perf is linux-kernel specific; hence the `uname -r`.

---

# Before running perf

 - Compile your executable with symbols (`-g`)
 - Add the `-fno-omit-frame-pointer` compiler flag
 - Decorate interesting functions with

```cpp
__attribute__ ((noinline))
```

---

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

# perf stat: Explanation

- `task-clock`: How long the program ran
- `context-switches`: How many times this program was stopped so another process could progress
- `cpu-migrations`: How many times this program was moved from one core to another

---

# perf stat: Explanation

- `page-faults`: How many times the operating system had to give the process more memory during its course of execution
- `stalled-cycles-frontend`: Cycles idle while instructions are being slowly or branch mispredictions
- `stalled-cycles-backend`: Cycles idle while waiting for data to be fetched from RAM or long-running instructions completing

---

# perf stat: Explanation

- `branches`: Number of `jmp` instructions hit during execution
- `branch-misses`: Number of times speculative execution was incorrect.

---

# perf stat: It's annoying

It's really good at telling you your code is terrible, but can't be bothered to tell you what to do about it.


---

# Branch misprediction

```bash
$ perf stat -B ./run_benchmarks.x
185,665,839,558 branches      #  979.910 M/sec
  1,068,785,447 branch-misses #    0.58% of all branches
```

---

# Available hardware counters

```bash
$ perf list
  branch-instructions OR branches                    [Hardware event]
  branch-misses                                      [Hardware event]
  bus-cycles                                         [Hardware event]
  cache-references                                   [Hardware event]
  cpu-cycles OR cycles                               [Hardware event]
  instructions                                       [Hardware event]
  ref-cycles                                         [Hardware event]
  stalled-cycles-frontend OR idle-cycles-frontend    [Hardware event]

  context-switches OR cs                             [Software event]
  cpu-clock                                          [Software event]
  cpu-migrations OR migrations                       [Software event]
  major-faults                                       [Software event]
  minor-faults                                       [Software event]
  page-faults OR faults                              [Software event]
  task-clock                                         [Software event]

  L1-dcache-load-misses                              [Hardware cache event]
  L1-dcache-loads                                    [Hardware cache event]
  L1-dcache-prefetch-misses                          [Hardware cache event]
```


---

# Counting hardware events

```bash
$ perf stat -e L1-dcache-load-misses,L1-dcache-loads ./run_benchmarks.x
    571,380,771 L1-dcache-load-misses # 0.18% of all L1-dcache hits    (66.67%)
320,099,165,220 L1-dcache-loads                                        (66.66%)
```

---

# Basic Usage perf record:

```bash
$ perf record -g ./run_benchmarks.x
$ perf report -g -M intel
```

![inline](figures/basic_perf.png?raw=true "Title")

---

# perf record -g -M intel

- `-M intel` spits the dissassembled code out in Intel syntax
- `-g` creates a callgraph.

---

# perf record


- `perf record` creates a file called `perf.data`.
- `perf report` reads `perf.data`, and tells you about your program:

```bash
$ perf report -g -U -M intel
```


---

# Self and Children

- The `Self` column says how much time was taken within the function.
- The `Children` column says how much time was spent in functions called by the function.

- If the `Children` column value is very near the `Self` column value, that function isn't your hotspot!


---

# Self and Children

If `Self` and `Children` is confusing, just get rid of it:

```bash
$ perf report -g -U -M intel --no-children
```


---

# perf report disassembly:

![inline](figures/inline_assembly.png?raw=true "Generated Assembly")

---

# perf report commands

- `k`: Show line numbers of source code
- `o`: Show instruction number
- `t`: Switch between percentage and samples
- `J`: Number of jump sources on target; number of places that can jump here.
- `s`: Hide/Show source code


---

# perf gotchas

- perf sometimes attributes the time in a single instruction to the *next* instruction.


---

# perf gotchas

```bash
     │         if (absx < 1)
7.76 │       ucomis xmm1,QWORD PTR [rbp-0x20]
0.95 │     ↓ jbe    a6
1.82 │       movsd  xmm0,QWORD PTR ds:0x46a198
0.01 │       movsd  xmm1,QWORD PTR ds:0x46a1a0
0.01 |       movsd  xmm2,QWORD PTR ds:0x46a100 
```

Hmm, so moving data into `xmm1` and `xmm2` is 182x faster than moving data into `xmm0` . . .

Looks like a misattribution of the `jbe`.


---

# Generating wins with perf:

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

![inline](figures/dot_product_O0.png?raw=true "Dot product at -O0")


---

# Detour: System V ABI

- A floating point return value is placed in register `xmm0`.
- The first integer argument is placed in `rdi`
- The second integer argument is placed in `rsi`
- The third integer argument is placed in `rdx`


---

# Dot product example:

```x86asm
Real dot_product(const Real * const a, const Real * const b, size_t n)
{
push   rbp
mov    rbp, rsp                 ; Establish a stack frame for our function
xorps  xmm0, xmm0               ; Clear out xmm0, as it will be used for our return value
mov    QWORD PTR [rbp-0x8],rdi  ; rdi is first argument; place address of a on stack
mov    QWORD PTR [rbp-0x10],rsi ; rsi is second argument; place address of b on stack
mov    QWORD PTR [rbp-0x18],rdx ; rdx is third argument; place length of array on stack
```

Why copy the integer registers to the stack?-- perhaps a -O0 artefact.


---

# Dot product example:

```x86asm
Real s = 0;
movsd  QWORD PTR [rbp-0x20],xmm0
```

This sets `s` to zero and places it on the stack.

Why put `s` on the stack?--leave it in `xmm0`.

---

# Dot product example

```x86asm
for (size_t i = 0; i < n; ++i)
      mov    QWORD PTR [rbp-0x28],0x0  ; set i = 0
                                       ; but why not use a count register?
20:   mov    rax,QWORD PTR [rbp-0x28]  ; copy i into rax
                                       ; note that this line is the target of a jump
      cmp    rax,QWORD PTR [rbp-0x18]  ; compare i with n
      jae    6d                        ; jump to 6d if i is "after or equal" to n
      ; stuff . . .
6d:   movsd  xmm0,QWORD PTR [rbp-0x20] ; move s to xmm0 (System V abi convention)
      pop    rbp                       ; increment base pointer
      ret                              ; return control to caller
```

---

# Dot product example

```x86asm
auto tmp = a[i]*b[i];
mov    rax,QWORD PTR [rbp-0x28]   ; copy i into rax *again*!
mov    rcx,QWORD PTR [rbp-0x8]    ; move address of a into rcx
                                  ; why didn't we just leave it in rdi?
movsd  xmm0,QWORD PTR [rcx+rax*8] ; move a[i] into xmm0
mov    rax,QWORD PTR [rbp-0x28]   ; copy i into rax *again*!
mov    rcx,QWORD PTR [rbp-0x10]   ; move address of b into rcx,
                                  ; why didn't we just leave it in rsi?
mulsd  xmm0,QWORD PTR [rcx+rax*8] ; a[i]*b[i]
movsd  QWORD PTR [rbp-0x30],xmm0  ; push tmp onto stack
s += tmp;
movsd  xmm0,QWORD PTR [rbp-0x30]  ; move tmp from stack back to where it was
addsd  xmm0,QWORD PTR [rbp-0x20]  ; add tmp to s
movsd  QWORD PTR [rbp-0x20],xmm0  ; mov s back to stack
```

---

# Dot product example

```x86asm
mov    rax,QWORD PTR [rbp-0x28] ; copy i into rax a fourth time?!!
add    rax,0x1                  ; i = i + 1
mov    QWORD PTR [rbp-0x28],rax ; copy rax back into stack
jmp    20                       ; go to top of loop
```

vv---

# Dot product example

- The compiler basically screwed up everything.
- Benchmarking put it the timing a 2.55N nanoseconds, where N is the array length.


---

# Hand-written assembly: 0.8N ns.

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

---

# Dot product example

Interestingly, compiling at -O3 gives identical performance to our handcoded assembly, at 0.8N nanoseconds.

The compiler starts using more `xmm` registers, and stops doing (as many) superfluous writes to the stack.


```nasm
      xorpd  xmm0,xmm0  ; clear out xmm0 register
      test   rdx,rdx    ; if n = 0, return
      je     29
      nop
10:   movsd  xmm1,QWORD PTR [rdi] ; move a[i] to xmm1
      mulsd  xmm1,QWORD PTR [rsi] ; xmm1 = a[i]*b[i]
      addsd  xmm0,xmm1            ; s = s + xmm1 = s + a[i]*b[i]
      add    rdi,0x8              ; move a's pointer offset by 8 bytes
      add    rsi,0x8              ; move b's pointer offset by 8 bytes
      dec    rdx                  ; n -> n-1
      jne    10                   ; jump to beginning of loop if rdx > 0.
29:   ret
```

---

# Dot product example


But even at -O3, we're still only using 64 bits of the 128 `xmm` registers, and the `xmm` registers are the first 128 bits of the `ymm` registers.

Let's see if we can do better.


---

# Dot product example



