

# Introduction #
AddressSanitizer introduces [~2x slowdown on average](PerformanceNumbers.md).
This is very good for most kinds of testing, but still often prohibitively slow
for production use. We believe that AddressSanitizer can be
efficiently implemented in hardware which will reduce the CPU overhead to ~20%
and thus allow wider use of AddressSanitizer in production.
On this page we try to explain why a hardware-assisted AddressSanitizer (**HWASAN**)
could be faster and more flexible than a pure-software implementation (**SWASAN**).

# Instrumentation #
As explained in detail in AddressSanitizerAlgorithm,
every memory access in the program is instrumented like this:
```
CheckAddressAndCrashIfBad(Addr, kSize);
*Addr = ... # Original memory access, reads or writes kSize bytes
```
The size of the memory access in bytes (`kSize`)
is a compile-time constant and is one of 1, 2, 4, 8, 16, 32, and 64.

The code for `CheckAddressAndCrashIfBad(Addr, kSize)` is inlined by the compiler module.
Currently it looks like this (kSize < 8):
```
ShadowAddr = (Addr >> 3) + kOffset;
Shadow = LoadByte(ShadowAddr);
if (Shadow && Shadow <= (Addr & 7) + kSize - 1)
  ReportBug(Addr);
```
and even simpler for kSize >= 8:
```
ShadowAddr = (Addr >> 3) + kOffset;
Shadow = LoadNBytes(ShadowAddr, kSize / 8);
if (Shadow)
  ReportBug(Addr);
```
kOffset is a compile-time constant that depends on the particular platform.
E.g. on Linux i386 kOffset=0x20000000, and on Linux x86\_64 kOffset=0x7fff8000.
The offset is different when using AddressSanitizerForKernel.

This is how the x86\_64 assembly look:
```
# long load8(long *a) { return *a; }
0000000000000030 <load8>:
  30:	48 89 f8             	mov    %rdi,%rax
  33:	48 c1 e8 03          	shr    $0x3,%rax
  37:	80 b8 00 80 ff 7f 00 	cmpb   $0x0,0x7fff8000(%rax)
  3e:	75 04                	jne    44 <load8+0x14>
  40:	48 8b 07             	mov    (%rdi),%rax   <<<<<< original load
  43:	c3                   	retq   
  44:	52                   	push   %rdx
  45:	e8 00 00 00 00       	callq  __asan_report_load8
```
```
# int  load4(int *a)  { return *a; }
0000000000000000 <load4>:
   0:	48 89 f8             	mov    %rdi,%rax
   3:	48 89 fa             	mov    %rdi,%rdx
   6:	48 c1 e8 03          	shr    $0x3,%rax
   a:	83 e2 07             	and    $0x7,%edx
   d:	0f b6 80 00 80 ff 7f 	movzbl 0x7fff8000(%rax),%eax
  14:	83 c2 03             	add    $0x3,%edx
  17:	38 c2                	cmp    %al,%dl
  19:	7d 03                	jge    1e <load4+0x1e>
  1b:	8b 07                	mov    (%rdi),%eax    <<<<<< original load
  1d:	c3                   	retq   
  1e:	84 c0                	test   %al,%al
  20:	74 f9                	je     1b <load4+0x1b>
  22:	50                   	push   %rax
  23:	e8 00 00 00 00       	callq  __asan_report_load4
```

So, for an 8-byte memory accesses we add two arithmetic instructions, one load and
one branch instruction
on the main path. One or two instructions are added for failure handling
(we could use any trap instruction instead of calling `__asan_report_load8`).
For smaller memory accesses extra branch instruction and 3-4 arithmetic instructions are added.

We believe that all these computations could be done by a single new machine instruction,
**ASANCHK**, that takes `Addr` as a single parameter and has 7 modifications for 7 different
accesses sizes.
This may sound too complex for one instruction, but Intel has recently introduced
two even more complex instructions: [BNDLDX/BNDSTX](IntelMemoryProtectionExtensions.md).

The instrumented code would look like this:
```
# long load8(long *a) { return *a; }
0000000000000030 <load8>:
asanchk.8 (%rdi)
mov       (%rdi),%rax   <<<<<< original load
retq
```
```
# int  load4(int *a)  { return *a; }
0000000000000000 <load4>:
asanchk.4 (%rdi)
mov       (%rdi),%eax    <<<<<< original load
retq
```

# Configuration #
Just like Intel's MPX has configuration registers BNDCFGU/BNDCFGS
(one for user-space, one for kernel),
HWASAN could use a pair of configuration registers (ASANCFGU for user space, ASANCFGS for kernel)
to achieve greater flexibility.
## Enabled bit ##
One bit in ASANCFGS could indicate whether HWASAN is enabled.
If this bit is not set, ASANCHK is treated as a NOP.
This would be a great advantage over SWASAN since SWASAN has no way to enable/disable
checking at run-time.

We could have two separate 'enabled' bits, one for loads and one for stores,
so that a user can enable checking loads and stores independently.
In this case the ASANCHK instruction opcode will need to contain a _load-or-store_ bit:
```
# long load8(long *a) { return *a; }
0000000000000030 <load8>:
asanchk.l.8 (%rdi)
mov         (%rdi),%rax   <<<<<< original load
retq
```
```
# void store8(long *a) { *a = 0x1234; }
0000000000000030 <store8>:
asanchk.s.8 (%rdi)
movq        $0x1234,(%rdi)
retq
```

## kOffset ##
48 bits in ASANCHK could store kOffset. Yet another advantage over SWASAN providing greater flexibility.
Currently, SWASAN has to reserve the shadow memory region at startup at a fixed address,
which causes conflict with other sandbox-like environments. With kOffset in ASANCFGS
the tool will be able to choose an arbitrary region for the shadow.

## ShadowScale ##
Two bits in ASANCFGS could indicate the ShadowScale
(the number by which the Addr is right-shifted to compute the shadow).
In SWASAN, ShadowScale is 3 which appears to be the best value performance-wise
for the pure software approach because it allows
to have shorter instrumentation for 8-byte accesses.
However values 4, 5, and 6 allow to use less memory for shadow at the cost of increased minimal
redzone. If the minimal redzone size is 16 bytes (which is the default for SWASAN),
then ShadowScale=4 might be better for HWASAN.

This configuration option is less important and may be harder to implement than the first two.

# Sources of speedup #
We expect HWASAN to be faster than SWASAN for a few reasons.

1. Fewer instructions, fewer instruction cache problem. This is important for huge application,
i.e. especially for production deployment.

2. ASANCHK should not require any extra general purpose registers, thus fewer spills/fills.

3. The arithmetic performed by ASANCHK is simple and can be implemented more efficiently
than a series of general instructions.

4. (probably, the major reason) accesses to the shadow and to the application memory
are interrelated
and the memory subsystem could start fetching the application address together with the
shadow address.

Since the shadow memory is 1/8 of the application memory or less (i.e. <= 12.5%)
and the arithmetic instructions cost next to nothing, our _guesstimate_
is that HWASAN may slowdown the application by 20% on average.

# Other benefits #

Similar to Intel MPX, ASANCHK instruction could use one of existing NOP opcodes
so that the binaries can run on legacy hardware.

Using single instruction for bounds checking will simplify manual assembly debugging
and inspection (in SWASAN, the instrumentation instructions pollute the code too much)

It will become possible to always build shared libraries with instrumentation enabled
and link them to all binaries (instrumented or not).
This is a common issue with SWASAN where users want to use a single shared library
with their instrumented binary and with a pre-built non-instrumented binary of e.g. Python interpreter.

# Possible modifications #
SWASAN implementation has been tuned for the pure software case.
However HWASAN could be different because a hardware implementation may allow us
to do more work per every memory access.

## Pattern mode ##
Several memory error detection tools
([DrMemory](http://www.drmemory.org/),
[LBC](http://www.cs.sunysb.edu/~nhasabni/papers/LBC_CGO_Presentation.pdf))
rely on magic bytes (patterns) in the application memory to detect unaddressable accesses
and use the shadow memory as a slow-and-rare fall-back.
In SWASAN this will introduce much more instrumentation code.
But for HWASAN this could be beneficial since the memory system will be stressed less.

Pattern mode is less suitable for detecting stack-buffer-overflow bugs because it requires
to poison 8 times more memory on every function entry and exit.

## Byte-to-bit shadow ##
Alternative to the AddressSanitizer shadow encoding is a simple byte-to-bit shadow mapping where
one bit of shadow represent addressability of the corresponding byte in memory.
Disadvantage of this mapping is that it requires 1/8 of address space to be used for shadow memory,
while ShadowScale>=4 allows to use less address space.
Besides, with byte-to-bit mapping it will be harder to find
[unaligned partially OOB accesses](https://code.google.com/p/address-sanitizer/issues/detail?id=100).

But byte-to-bit mapping _may_ be simpler to implement in hardware.

## Automatically check loads/stores ##
An alternative to ASANCHK instruction is to perform the checks for all load/store instructions
if HWASAN is anabled (ASANCFGU).
The benefit is that no instrumentation will be required and all legacy code will be automatically checked
if linked with the AddressSanitizer run-time library.
The downside is that no compiler optimizations (eliminations of ASANCHK) will be possible.

## Links ##
  * The paper [WatchdogLite: Hardware-Accelerated Compiler-Based Pointer Checking](http://www.cs.rutgers.edu/~santosh.nagarakatte/papers/cgo2014-final.pdf) proposes a similar set of extra instructions, although they are much closer in spirit to [MPX](IntelMemoryProtectionExtensions.md) than to AddressSanitizer