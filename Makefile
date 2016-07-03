CXX=clang++
CPPFLAGS:= -std=c++14 -g -fno-omit-frame-pointer

ifdef RELEASE
CPPFLAGS+= -O3 -Ofast -march=native
else
CPPFLAGS+= -O0
endif

all: run_benchmarks.x run_benchmarks.s

run_benchmarks.x: run_benchmarks.o
	$(CXX) $(CPPFLAGS) -o $@ $< -lbenchmark -lpthread

run_benchmarks.o: run_benchmarks.cpp
	$(CXX) $(CPPFLAGS) -c $< -o $@

run_benchmarks.s: run_benchmarks.cpp
	$(CXX) $(CPPFLAGS) -S -masm=intel $<

clean:
	rm -f *.x *.s *.o
