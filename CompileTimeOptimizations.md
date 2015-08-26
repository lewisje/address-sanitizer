

# Introduction #
AddressSanitizer does not need to instrument all memory accesses to find all bugs.

The simplest example:
```
void inc(int *a) {
  (*a)++;
}
```
Here we have two memory accesses, load and store,
but we need to instrument only the first one.

Currently, we have very few optimizations implemented.
Some of the possible optimizations are listed below.
Your suggestions are welcome.

Instrument only the first access.
```
*a = ...
if (...)
  *a = ...
```

Instrument only the second access
```
if (...)
  *a = ...
*a = ...
```

Instrument this loop with `__asan_region_is_poisoned(a, n*sizeof(*a))` and avoid instrumenting individual accesses.
```
for (int i = 0; i < n; i++)
  a[i] = ...;
```

Combine two accesses into one
```
struct { int a, b; } x;  ...
x.a = ...;
x.b = ...;
```

No point in instrumenting accesses to scalar globals
(but beware of symbol interposition in shared libs)
```
int glob;
int get_glob() {
  return glob;
}
```

# Dataflow proposal #

Using dataflow techniques for range checks elimination (see works by Markstein, Gupta and others), it should be possible to perform generic optimization of Asan checks. Some concrete examples are given below.

Fully redundant checks:
```
if (f) {
  *p;
} else {
  *p;
}
*p; // Eliminable
```

Same could be done taking care of size:
```
if (f) {
  *(char *)p;
} else {
  *p;
}
*(char *)p; // Eliminable
```

And copies:
```
*p;
q = p;
*q; // Eliminable
```

And phi-functions:
```
if (f) {
  p1 = ...
  *p1;
} else {
  p2 = ...
  *p2;
}
p3 = phi(p1, p2)
*p3; // Eliminable
```

Hoisting very busy expressions:
```
// Enough to check *p here once
if (x) {
  *p;
} else {
  *p;
}
```

Combining checks:
```
int *p;
// Enough to check *(long *)p
p[0];
p[1];
```

Strengthening checks:
```
*(char *)p; // Eliminable
*p;
```

Combining checks across loop iterations:
```
int *p;
// Enough to check __asan_region_is_poisoned(a, n*sizeof(*a))
for (i = 0; i < n; ++i)
  p[i];
```

All mentioned optimization could potentially be done for memory ranges with symbolic lengths (in addition to scalar memory accesses) but this is not yet planned.

# Complete tests #

## Escape analysis for use-after-return ##
Here we can prove that `a` does not escape `foo()` and thus we may avoid putting it on fake stack.
```
int bar();
int foo(unsigned i) {
  int a[3] = {bar(), bar(), bar()};
  return a[i % 3];
}

```
# Comments? #
Send comments to address-sanitizer@googlegroups.com