#include <cmath>
#include <ostream>
#include <random>
#include <benchmark/benchmark.h>

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

static void BM_dot_product_easy_asm(benchmark::State& state)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(1, 10);
    size_t n = state.range_x();
    double a[n];
    double b[n];
    for (size_t i = 0; i < n; ++i)
    {
        a[i] = dis(gen);
        b[i] = dis(gen);
    }
    double s = 0;
    while (state.KeepRunning())
    {
        benchmark::DoNotOptimize(s = easy_asm_dot_product(a, b, n));
    }

    std::ostream cnull(nullptr);
    cnull << s;
    state.SetComplexityN(state.range_x());
}

BENCHMARK(BM_dot_product_easy_asm)->RangeMultiplier(2)->Range(8, std::pow(2, 18))->Complexity();

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


template<typename Real>
static void BM_dot_product(benchmark::State& state)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<Real> dis(1, 10);
    size_t n = state.range_x();
    Real a[n];
    Real b[n];
    for(size_t i = 0; i < n; ++i)
    {
        a[i] = dis(gen);
        b[i] = dis(gen);
    }

    Real s = 0;
    while (state.KeepRunning())
    {
        benchmark::DoNotOptimize(s = dot_product<Real>(a, b, n));
    }
    std::ostream cnull(nullptr);
    cnull << s;
    state.SetComplexityN(state.range_x());
}

BENCHMARK_TEMPLATE(BM_dot_product, double)->RangeMultiplier(2)->Range(8, std::pow(2, 18))->Complexity();

BENCHMARK_MAIN();
