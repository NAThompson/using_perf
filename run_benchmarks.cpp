#include <cmath>
#include <ostream>
#include <random>
#include <benchmark/benchmark.h>

template<typename Real>
Real dot_product(const Real * const a, const Real * const b, size_t n)
{
    Real s = 0;
    for(size_t i = 0; i < n; ++i)
    {
        s += a[i]*b[i];
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

BENCHMARK_TEMPLATE(BM_dot_product, double)->RangeMultiplier(2)->Range(8, 16384)->Complexity();

BENCHMARK_MAIN();
