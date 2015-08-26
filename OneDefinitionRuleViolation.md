In short, [One Definition Rule](http://en.wikipedia.org/wiki/One_Definition_Rule) (ODR) requires that every object in C++ is defined just once.


```
==> odr0.cc <==
int XXX;
int main(){}

==> odr1.cc <==
long long XXX;

==> odr2.cc <==
float XXX;
```

Simple cases can be detected by linker:
```
% clang++ odr0.cc odr1.cc 
/tmp/odr1-f97b07.o:(.bss+0x0): multiple definition of `XXX'
```

In various other cases (e.g. when shared libraries are involved) the linker remains silent. AddressSanitizer, however, is able to detect most of the ODR violations at process startup.

Currently it requires a special run-time switch `ASAN_OPTIONS=detect_odr_violation=1`
```
% clang++ -fsanitize=address -fPIC -shared odr1.cc -o odr1.so
% clang++ -fsanitize=address odr0.cc odr1.so -Wl,-R.
% ASAN_OPTIONS=detect_odr_violation=1 ./a.out 
=================================================================
==19043==ERROR: AddressSanitizer: odr-violation (0x000001316280):
  [1] size=4 XXX odr0.cc
  [2] size=8 XXX odr1.cc
```

To find even more ODR violations (where the object sizes are equal)
run with `ASAN_OPTIONS=detect_odr_violation=2`
```
% clang++ -fsanitize=address -fPIC -shared odr2.cc -o odr2.so
% clang++ -fsanitize=address odr0.cc odr2.so -Wl,-R.
% ASAN_OPTIONS=detect_odr_violation=1 ./a.out 
 <Silent>
% ASAN_OPTIONS=detect_odr_violation=2 ./a.out 
=================================================================
==19182==ERROR: AddressSanitizer: odr-violation (0x000001316280):
  [1] size=4 XXX odr0.cc
  [2] size=4 XXX odr2.cc
```