CXX=clang++
CPPFLAGS:= -std=c++14 -O3 -g -fno-omit-frame-pointer

all: run_benchmarks.x run_benchmarks.s

run_benchmarks.x: run_benchmarks.o
	$(CXX) $(CPPFLAGS) -o $@ $< -lbenchmark -lpthread

run_benchmarks.o: run_benchmarks.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

run_benchmarks.s: run_benchmarks.cpp
	$(CXX) $(CPPFLAGS) -S -masm=intel $<

clean:
	rm -f *.x *.s *.o
