
# Zero-based shadow mapping #

To enable this mode, build **all** the sources with `-fsanitize-address-zero-base-shadow`, clang-only.

Instead of computing `Shadow = (Mem >> 3) + kOffset` we compute `Shadow = (Mem >> 3)`, i.e. `kOffset = 0` (see AddressSanitizerAlgorithm).
This mode is possible only if 1/8 of address space starting from the zero page is not occupied at startup and AddressSanitizer can map it for the shadow.
We are aware of only one situation where this works: Linux executables compiled with `-fPIC` or `-fPIE` and linked with `-pie`.

```
% cat  procmaps.cc 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
int main() {
  char buff[100];
  sprintf(buff, "cat /proc/%d/maps", getpid());
  system(buff);
}
```

Regular mode:
```
% clang -fsanitize=address ./procmaps.cc && ./a.out 
00400000-00421000 r-xp 00000000 fc:00 1573310                            /tmp/a.out
00620000-00621000 r--p 00020000 fc:00 1573310                            /tmp/a.out
00621000-00622000 rw-p 00021000 fc:00 1573310                            /tmp/a.out
00622000-0338e000 rw-p 00000000 00:00 0 
ffffffff000-120000000000 rw-p 00000000 00:00 0 <<<<<<<<<<< LowShadow
120000000000-140000000000 ---p 00000000 00:00 0  <<<<<<<<< SHadowGap
140000000000-200000000000 rw-p 00000000 00:00 0  <<<<<<<<< HighShadow
7f1ebd5a9000-7f1ebd75e000 r-xp 00000000 fc:00 1208459                    /lib/x86_64-linux-gnu/libc-2.15.so
....
7f1ebe1c2000-7f1ebe1c4000 rw-p 00023000 fc:00 1208920                    /lib/x86_64-linux-gnu/ld-2.15.so
7fffccd4b000-7fffccd6c000 rw-p 00000000 00:00 0                          [stack]
7fffccdff000-7fffcce00000 r-xp 00000000 00:00 0                          [vdso]
```

Zero shadow offset mode (with -pie):

```
% clang -fsanitize=address -fsanitize-address-zero-base-shadow -fPIE -pie ./procmaps.cc && ./a.out 
00040000-20000000000 ---p 00000000 00:00 0       <<<<<<<<< ShadowGap
20000000000-100000000000 rw-p 00000000 00:00 0   <<<<<<<<< HighShadow
7f343283c000-7f34329f1000 r-xp 00000000 fc:00 1208459                    /lib/x86_64-linux-gnu/libc-2.15.so
7f3433679000-7f343367a000 rw-p 00022000 fc:00 1573310                    /tmp/a.out
7f343367a000-7f34363e6000 rw-p 00000000 00:00 0 
7fff79e32000-7fff79e53000 rw-p 00000000 00:00 0                          [stack]
7fff79f1a000-7fff79f1b000 r-xp 00000000 00:00 0                          [vdso]
```

## Example ##

Why is this worth the trouble?
Zero shadow offset not just saves one arithmetic instruction,
but also saves one register, which is otherwise occupied with the large constant. Compare these two:


```
% cat load.c 
long Foo(long *a) { return *a; }
```



```
% clang -fsanitize=address -fsanitize-address-zero-base-shadow -O2 -c load.c && objdump -d load.o
0000000000000000 <Foo>:
   0:	50                   	push   %rax
   1:	48 89 f8             	mov    %rdi,%rax
   4:	48 c1 e8 03          	shr    $0x3,%rax
   8:	80 38 00             	cmpb   $0x0,(%rax)
   b:	75 05                	jne    12 <Foo+0x12>
   d:	48 8b 07             	mov    (%rdi),%rax
  10:	5a                   	pop    %rdx
```
```
% clang -fsanitize=address -O2 -c load.c && objdump -d load.o
0000000000000000 <Foo>:
   0:	50                   	push   %rax
   1:	48 89 f8             	mov    %rdi,%rax
   4:	48 c1 e8 03          	shr    $0x3,%rax
   8:	48 b9 00 00 00 00 00 	movabs $0x100000000000,%rcx
   f:	10 00 00 
  12:	48 09 c1             	or     %rax,%rcx
  15:	80 39 00             	cmpb   $0x0,(%rcx)
  18:	75 05                	jne    1f <Foo+0x1f>
  1a:	48 8b 07             	mov    (%rdi),%rax
  1d:	5a                   	pop    %rdx
  1e:	c3                   	retq   
```

## Performance ##
The data below reflects the performance gain from ZeroBasedShadow on SPEC CPU 2006. 

&lt;BR&gt;


Measured LLVM [r174590](https://code.google.com/p/address-sanitizer/source/detail?r=174590) on Intel Xeon W3690 @3.47GHz

```
       400.perlbench,      1200.00,      1179.00,         0.98
           401.bzip2,       885.00,       821.00,         0.93
             403.gcc,       744.00,       724.00,         0.97
             429.mcf,       605.00,       579.00,         0.96
           445.gobmk,       843.00,       809.00,         0.96
           456.hmmer,      1305.00,      1236.00,         0.95
           458.sjeng,       923.00,       898.00,         0.97
      462.libquantum,       539.00,       532.00,         0.99
         464.h264ref,      1269.00,      1194.00,         0.94
         471.omnetpp,       629.00,       598.00,         0.95
           473.astar,       676.00,       663.00,         0.98
       483.xalancbmk,       493.00,       452.00,         0.92
            433.milc,       712.00,       656.00,         0.92
            444.namd,       637.00,       592.00,         0.93
          447.dealII,       654.00,       631.00,         0.96
          450.soplex,       391.00,       369.00,         0.94
          453.povray,       458.00,       419.00,         0.91
             470.lbm,       388.00,       384.00,         0.99
         482.sphinx3,       995.00,       886.00,         0.89
```


## Experimental ##
We are experimenting with another way to achieve zero-based offset using `-Wl,-Ttext-segment=0x40000000`. **TODO**