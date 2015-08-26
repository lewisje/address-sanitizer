# Introduction #

We want to get a complete trace of program execution as a sequence of basic block IDs.
In general this task is quite challenging due to performance, IO, threads, memory consumption and what not.
Here we describe a very simple and limited prototype of basic-block
tracer built into AsanCoverage (you will need fresh clang).

Usage:

```
clang++ -O1 -g -fsanitize=address -fsanitize-coverage=3 -mllvm -sanitizer-coverage-experimental-tracing your-app.cc
ASAN_OPTIONS=coverage=1 ./a.out
```

This will produce two files
```
trace-points.PID.sancov
trace-events.PID.sancov
```
The first file contains unique PCs executed by the application as text lines of the form "module\_name offset".
The second file is binary and contains the trace represented as a set of 4-byte indexes into the first file.

# Limitations #
As we've said, the current implementation is a toy:
  * not thread-safe
  * not tuned for performance
  * supports only relatively small traces
  * uses 4 bytes per block (1 bit per edge is theoretically possible)

If you want something more robust come back later or wait for
[Intel's Processor Tracing ](https://software.intel.com/en-us/blogs/2013/09/18/processor-tracing)

# Example #
Here is a complete example of running the trace on a RE2 test:
```
mkdir tracing_example
cd tracing_example

cat << EOF > re2_hello_world.cc
#include "re2/re2.h"
#include <cassert>
int main(int argc, char **argv) {
  assert(argc == 3);
  return RE2::PartialMatch(/*text*/argv[1], /*re=*/argv[2]);
}
EOF

hg clone https://re2.googlecode.com/hg re2
ASAN="-fsanitize=address" 
TRACE="-fsanitize-coverage=3 -mllvm -sanitizer-coverage-experimental-tracing"
(cd re2 && CXX="clang++ $ASAN $TRACE" make -j)
clang++ -fsanitize=address  re2_hello_world.cc -Ire2 re2/obj/libre2.a
ASAN_OPTIONS=coverage=1:verbosity=1 ./a.out hello 'h.*o'

#   ==6102== CovDump: Trace: 1440 PCs written
#   ==6102== CovDump: Trace: 16834 Events written


ls -l trace-*sancov

#   -rw-r----- 1 xxx xxx 67336 Nov 18 16:38 trace-events.6102.sancov
#   -rw-r----- 1 xxx xxx 90720 Nov 18 16:38 trace-points.6102.sancov

head -n 5 trace-points.*.sancov

#   a.out 0x49ccb8
#   a.out 0x49cf45
#   a.out 0x49cf74
#   a.out 0x49d9b5

wc -l trace-points.*sancov

#   1440 trace-points.6102.sancov

trace-dump trace-points.*.sancov trace-events.*.sancov | head -n 5

#   a.out 0x49ccb8
#   a.out 0x49cf45
#   a.out 0x49cf74
#   a.out 0x49d9b5
#   a.out 0x49db50

trace-dump trace-points.*.sancov trace-events.*.sancov | wc -l
#   16834
```


# Trace dump #
In case you want a simple `trace-dump` utility:
```
#include <cassert>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

int main(int argc, char **argv) {
  assert(argc == 3);

  std::string s;
  std::ifstream points(argv[1]);
  std::vector<std::string> p;
  while (std::getline(points, s))
    p.push_back(s);

  unsigned e;
  std::ifstream events(argv[2], std::ios::in | std::ifstream::binary);
  while (events.read(reinterpret_cast<char *>(&e), sizeof(e)))
    std::cout << p[e] << std::endl;
}
```