# Getting SPEC 2006 #

SPEC 2006 may be obtained from http://www.spec.org/cpu2006
(it is not free, but most compiler experts already have access to it).
Make sure to use the latest version 1.2

You will also need to install SPEC on your system. The following worked for me on Ubuntu 10.04:
```
cd your-spec-dir
./install.sh
```


# Patching SPEC 2006 #

There are several bugs in SPEC 2006 detectable by AddressSanitizer:
[FoundBugs#Spec\_CPU\_2006](FoundBugs#Spec_CPU_2006.md) 

&lt;BR&gt;


You will need to patch the spec code in order to run the benchmarks w/o errors: https://address-sanitizer.googlecode.com/svn/trunk/spec/spec2006-asan.patch

# Running SPEC 2006 #
Take a look at https://code.google.com/p/address-sanitizer/source/browse/trunk/spec/run_spec_clang_asan.sh



```
cd your-spec-dir
# First parameter: any uniq id
# Second parameter: test|train|ref (sizeof of input data)
# Following parameters: list of benchmarks (use all_c for all C tests, all_cpp for all C++ tests)
./run_spec_clang_asan.sh z test bzip2
```