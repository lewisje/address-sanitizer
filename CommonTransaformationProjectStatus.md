

The goal of this project was to create memory safety instrumentation and optimization passes that could be used by different memory error detection systems. The general idea is to add checks before operations that may cause invalid memory accesses. Passes were also added to register memory objects becoming valid/invalid.

# Check/call types #
## Generic load/store checks ##
These checks are the ones added just before loads, stores, and
other memory intrinsics. All we know is the memory address accessed and the size of the access.

## Fast load/store checks ##
Generic checks can be turned into fast checks if the following criteria are met:
  * The memory object it must access is known.
  * The size of the memory object is known.
  * It is allowed to access the memory object at the time of the check.

This means that it must be sufficient to check the following (assuming no overflow):
```
object ptr <= access ptr && access ptr + access size <= object ptr + object size
```
For example allocas with function scope are always ok.
A counterexample would be a heap object that is freed before the check. In that case just being in bounds isn't enough. The current implementation only handles global variables and allocas that obviously have function scope.

## Memory object registration calls ##
These calls are added for every alloca and global variable. The idea is
that if we know that no check should look at a particular variable's red zone (because we optimized away all checks that could depend on that
variable) then the red zone doesn't have to be added.

# Why it is useful to prove that a check could be a fast check #
  * It is much easier to optimize fast checks away because they are essentially assertions about variable ranges. Regular checks also have to find the memory object at run-time and check that the memory objects have not been deallocated.
  * If run-time fast checks were implemented using comparisons instead of red zones then we could avoid registering the memory objects the fast checks are on (as long as there can be no other cases where the object might be needed for regular checks).

# Results on part of SPEC #
This table shows the comparison between the optimizations already in ASan compared to adding the new optimizations. Note that the first run-time check column excludes the ones already avoided by ASan and the second one compares to the ASan results, not the completely unoptimized ones. The results were obtained by compiling with -O2.

Optimizations opt0-3 are explained after the tables. Note that the order between the optimizations matters because there are cases where each of the load/store check optimizations could remove a particular check. The total number of checks optimized is not affected.

## Both generic and fast load/store checks ##
| bench          | num loads/stores | num optimized by opt0 | num optimized by opt2 | num optimized by opt1 | num optimized by opt3 | num optimized total | run-time checks with ASan | extra run-time checks avoided |
|:---------------|:-----------------|:----------------------|:----------------------|:----------------------|:----------------------|:--------------------|:--------------------------|:------------------------------|
| 401.bzip2      | 4157             | 181 (4%)              | 1337 (32%)            | 90 (2%)               | 30 (1%)               | 1638 (39%)          | 5137459805                | 125499329 (2%)                |
| 429.mcf        | 559              | 22 (4%)               | 62 (11%)              | 18 (3%)               | 2 (0%)                | 104 (19%)           | 1250399778                | 29364707 (2%)                 |
| 433.milc       | 4550             | 1179 (26%)            | 344 (8%)              | 606 (13%)             | 0 (0%)                | 2129 (47%)          | 10126947401               | 6904142 (0%)                  |
| 445.gobmk      | 23876            | 4381 (18%)            | 3253 (14%)            | 2542 (11%)            | 411 (2%)              | 10587 (44%)         | 11719396028               | 1208435475 (10%)              |
| 456.hmmer      | 12391            | 352 (3%)              | 1645 (13%)            | 823 (7%)              | 27 (0%)               | 2847 (23%)          | 4746709143                | 63003542 (1%)                 |
| 458.sjeng      | 5051             | 1832 (36%)            | 257 (5%)              | 680 (13%)             | 21 (0%)               | 2790 (55%)          | 2710001895                | 215039527 (8%)                |
| 462.libquantum | 1029             | 102 (10%)             | 169 (16%)             | 203 (20%)             | 0 (0%)                | 474 (46%)           | 1323575848                | 1659284 (0%)                  |
| 464.h264ref    | 32238            | 5268 (16%)            | 2559 (8%)             | 1996 (6%)             | 133 (0%)              | 9956 (31%)          | 26274260762               | 6120457746 (23%)              |
| 470.lbm        | 365              | 15 (4%)               | 29 (8%)               | 15 (4%)               | 0 (0%)                | 59 (16%)            | 1143400158                | 47 (0%)                       |
| all            | 84216            | 13332 (16%)           | 9655 (11%)            | 6973 (8%)             | 624 (1%)              | 30584 (36%)         | 64432150818               | 7770363799 (12%)              |
| median         |                  | 16%                   | 11%                   | 11%                   | 0%                    | 44%                 |                           | 2%                            |
| maximum        |                  | 36%                   | 16%                   | 20%                   | 2%                    | 55%                 |                           | 23%                           |

## Only generic load/store checks ##
| bench          | num loads/stores | num optimized by opt0 | num optimized by opt2 | num optimized by opt1 | num optimized by opt3 | num optimized total | run-time checks with ASan | extra run-time checks avoided |
|:---------------|:-----------------|:----------------------|:----------------------|:----------------------|:----------------------|:--------------------|:--------------------------|:------------------------------|
| 401.bzip2      | 3801             | 154 (4%)              | 1213 (32%)            | 0 (0%)                | 0 (0%)                | 1367 (36%)          | 4287114265                | 0 (0%)                        |
| 429.mcf        | 511              | 3 (1%)                | 59 (12%)              | 0 (0%)                | 0 (0%)                | 62 (12%)            | 1131362574                | 0 (0%)                        |
| 433.milc       | 3853             | 1157 (30%)            | 303 (8%)              | 0 (0%)                | 0 (0%)                | 1460 (38%)          | 10118336477               | 0 (0%)                        |
| 445.gobmk      | 16753            | 3111 (19%)            | 1985 (12%)            | 0 (0%)                | 0 (0%)                | 5096 (30%)          | 7664245889                | 0 (0%)                        |
| 456.hmmer      | 11106            | 225 (2%)              | 1419 (13%)            | 0 (0%)                | 0 (0%)                | 1644 (15%)          | 4667858416                | 0 (0%)                        |
| 458.sjeng      | 3852             | 1756 (46%)            | 138 (4%)              | 0 (0%)                | 0 (0%)                | 1894 (49%)          | 2139309390                | 0 (0%)                        |
| 462.libquantum | 611              | 5 (1%)                | 51 (8%)               | 0 (0%)                | 0 (0%)                | 56 (9%)             | 1321916564                | 0 (0%)                        |
| 464.h264ref    | 28473            | 4318 (15%)            | 2369 (8%)             | 0 (0%)                | 0 (0%)                | 6687 (23%)          | 19544998709               | 0 (0%)                        |
| 470.lbm        | 330              | 0 (0%)                | 24 (7%)               | 0 (0%)                | 0 (0%)                | 24 (7%)             | 1143400111                | 0 (0%)                        |
| all            | 69290            | 10729 (15%)           | 7561 (11%)            | 0 (0%)                | 0 (0%)                | 18290 (26%)         | 52018542395               | 0 (0%)                        |
| median         |                  | 15%                   | 8%                    | 0%                    | 0%                    | 23%                 |                           | 0%                            |
| maximum        |                  | 46%                   | 13%                   | 0%                    | 0%                    | 49%                 |                           | 0%                            |

This table basically demonstrates that the extra optimizations only work on fast checks.

## Only fast load/store checks ##
| bench          | num loads/stores | num optimized by opt0 | num optimized by opt2 | num optimized by opt1 | num optimized by opt3 | num optimized total | run-time checks with ASan | extra run-time checks avoided |
|:---------------|:-----------------|:----------------------|:----------------------|:----------------------|:----------------------|:--------------------|:--------------------------|:------------------------------|
| 401.bzip2      | 356              | 27 (8%)               | 124 (35%)             | 90 (25%)              | 30 (8%)               | 271 (76%)           | 850345540                 | 125499329 (15%)               |
| 429.mcf        | 48               | 19 (40%)              | 3 (6%)                | 18 (38%)              | 2 (4%)                | 42 (88%)            | 119037204                 | 29364707 (25%)                |
| 433.milc       | 697              | 22 (3%)               | 41 (6%)               | 606 (87%)             | 0 (0%)                | 669 (96%)           | 8610924                   | 6904142 (80%)                 |
| 445.gobmk      | 7123             | 1270 (18%)            | 1268 (18%)            | 2542 (36%)            | 411 (6%)              | 5491 (77%)          | 4055150139                | 1208435475 (30%)              |
| 456.hmmer      | 1285             | 127 (10%)             | 226 (18%)             | 823 (64%)             | 27 (2%)               | 1203 (94%)          | 78850727                  | 63003542 (80%)                |
| 458.sjeng      | 1199             | 76 (6%)               | 119 (10%)             | 680 (57%)             | 21 (2%)               | 896 (75%)           | 570692505                 | 215039527 (38%)               |
| 462.libquantum | 418              | 97 (23%)              | 118 (28%)             | 203 (49%)             | 0 (0%)                | 418 (100%)          | 1659284                   | 1659284 (100%)                |
| 464.h264ref    | 3765             | 950 (25%)             | 190 (5%)              | 1996 (53%)            | 133 (4%)              | 3269 (87%)          | 6729262053                | 6120457746 (91%)              |
| 470.lbm        | 35               | 15 (43%)              | 5 (14%)               | 15 (43%)              | 0 (0%)                | 35 (100%)           | 47                        | 47 (100%)                     |
| all            | 14926            | 2603 (17%)            | 2094 (14%)            | 6973 (47%)            | 624 (4%)              | 12294 (82%)         | 12413608423               | 7770363799 (63%)              |
| median         |                  | 23%                   | 14%                   | 53%                   | 2%                    | 94%                 |                           | 80%                           |
| maximum        |                  | 43%                   | 28%                   | 87%                   | 6%                    | 100%                |                           | 100%                          |

This table demonstrates that once we have proven a check to be a fast check then the extra optimizations are quite effective (at least on the current cases).

# Compilation results on Chromium #

It is the result of building Chromium as follows:
```
GYP_GENERATORS=make GYP_DEFINES="clang=1 clang_use_chrome_plugins=0 disable_nacl=1" ./build/gyp_chromium
make chrome BUILDTYPE=Release
```

| check type    | num loads/stores | num optimized by opt0 | num optimized by opt2 | num optimized by opt1 | num optimized by opt3 | num optimized total |
|:--------------|:-----------------|:----------------------|:----------------------|:----------------------|:----------------------|:--------------------|
| only generic  | 4009323          | 58501 (1%)            | 2135555 (53%)         | 0 (0%)                | 0 (0%)                | 2194056 (55%)       |
| only fast     | 2548274          | 45156 (2%)            | 1642068 (64%)         | 847406 (33%)          | 1280 (0%)             | 2535910 (100%)      |
| generic+fast  | 6557597          | 103657 (2%)           | 3777623 (58%)         | 847406 (13%)          | 1280 (0%)             | 4729966 (72%)       |

The very high optimization rate may be influenced by different generic optimization options or the language (C in the used SPEC benchmarks and a lot of C++ in Chromium).

# Run-time results on Chromium's browser\_tests #
The following table shows the amount of time in seconds taken to run Chromium's browser\_tests. The time is shown as the sum of the times taken to run all the test cases as reported by the test runner.

The tests were run 6 times: once as a warm-up and then five more times as shown in the `run 1-5` columns.

| Method | average | run 1 | run 2 | run 3 | run 4 | run 5 |
|:-------|:--------|:------|:------|:------|:------|:------|
| Current ASAN | **7344** | 7283  | 7385  | 7276  | 7215  | 7563  |
| Current ASAN + opt1 | **7088** | 6852  | 6965  | 7131  | 6954  | 7536  |
| All optimizations | **7079** | 6953  | 7039  | 7292  | 7109  | 7001  |

The results indicate that using opt1 makes it run about 3% faster on average. The other new optimizations don't seem to make a significant additional difference.

The following tests were disabled because they caused occasional failures:
```
AutofillTest.AutofillAfterReload
AutofillTest.AutofillViaDownArrow
AutofillTest.BasicFormFill
AutofillTest.ComparePhoneNumbers
AutofillTest.DistinguishMiddleInitialWithinName
AutofillTest.DynamicFormFill
AutofillTest.FormFillLatencyAfterSubmit
AutofillTest.FormFillableOnReset
AutofillTest.MultipleEmailFilledByOneUserGesture
AutofillTest.NoAutofillForReadOnlyFields
BrowserTagTest.Isolation
BubbleGtkTest.ArrowLocation
ClickToPlayPluginTest.Basic
ClickToPlayPluginTest.LoadAllBlockedPlugins
ClickToPlayPluginTest.NoCallbackAtLoad
DevToolsSanityTest.TestNetworkRawHeadersText
DevToolsSanityTest.TestNetworkSize
DevToolsSanityTest.TestNetworkSyncSize
DownloadsDOMHandlerTest.DownloadsDOMHandlerTest_Created
ExtensionApiTest.CaptureVisibleTabJpeg
ExtensionApiTest.I18N
ExtensionWebRequestApiTest.WebRequestNewTab
FileSystemApiTest.FileSystemApiGetWritableTest
FileSystemApiTest.FileSystemApiSaveExistingFileWithWriteTest
FindInPageControllerTest.FindMovesWhenObscuring
FontSettingsWebUITest.testOpenFontSettings
FullscreenControllerBrowserTest.FLAKY_FullscreenMouseLockContentSettings
FullscreenControllerTest.BrowserFullscreenAfterTabFSExit
FullscreenControllerTest.BrowserFullscreenExit
FullscreenControllerTest.FullscreenFileURL
FullscreenControllerTest.TestFullscreenBubbleMouseLockState
FullscreenControllerTest.TestFullscreenFromTabWhenAlreadyInBrowserFullscreenWorks
FullscreenControllerTest.TestTabDoesntExitFullscreenOnSubFrameNavigation
FullscreenControllerTest.TestTabExitsFullscreenOnGoBack
FullscreenControllerTest.TestTabExitsFullscreenOnNavigation
FullscreenControllerTest.TestTabExitsItselfFromFullscreen
GpuFeatureTest.AcceleratedCompositingAllowed
GpuFeatureTest.Canvas2DAllowed
GpuFeatureTest.RafNoDamage
GpuFeatureTest.WebGLAllowed
IsolatedAppTest.SubresourceCookieIsolation
NTP4WebUITest.FLAKY_NTPHasThumbnails
NewTabUIBrowserTest.LoadNTPInExistingProcess
PageCyclerCachedBrowserTest.PlaybackMode
PageCyclerCachedBrowserTest.URLNotInCache
PanelBrowserNavigatorTest.NavigateFromCrashedPanel
PerformanceMonitorBrowserTest.KilledByOSEvent
PerformanceMonitorBrowserTest.RendererCrashEvent
PrerenderBrowserTest.PrerenderDelayLoadPlugin
PrerenderBrowserTest.PrerenderHTML5Video
PrerenderBrowserTest.PrerenderHTML5VideoJs
PrerenderBrowserTest.PrerenderHTML5VideoNetwork
PrerenderBrowserTest.PrerenderIframeDelayLoadPlugin
PrerenderBrowserTest.PrerenderInfiniteLoop
SavePageAsMHTMLBrowserTest.SavePageAsMHTML
SavePageBrowserTest.FileNameFromPageTitle
SavePageBrowserTest.RemoveFromList
SavePageBrowserTest.SaveCompleteHTML
SavePageBrowserTest.SaveHTMLOnly
SavePageBrowserTest.SaveHTMLOnlyCancel
SavePageBrowserTest.SaveViewSourceHTMLOnly
SpellCheckHostBrowserTest.DeleteCorruptedBDICT
ThreadedCompositorTest.ThreadedCompositor
TimeFormatBrowserTest.DecimalPointNotDot
UnloadTest.BrowserCloseInfiniteBeforeUnload
UnloadTest.BrowserCloseInfiniteBeforeUnloadAlert
```

# Load/store Check Optimizations #
## opt0 (ASan's global scalar optimization) ##
This pass works just like ASan's global scalar optimization which removes all checks on accesses to global scalars. This pass is brought out separately only because it helps show the difference between existing and potential new optimizations. The opt1 can do everything this pass can and more.

## opt1 (optimize-fast-memory-checks) ##
This pass removes fast load/store checks (ones where the memory object is known and valid at the time of the check) if it can prove that the access is entirely inside the memory object. It uses the array bounds analysis group to do so.

This pass can handle the following:
  * Cases where a gep indexes into an object using constant indices (this subsumes the opt0 pass).
  * Some cases where the previous case is complicated by the select and/or phi instructions that select the gep to use.
  * Some cases where the gep depends on loop induction variables. [(example)](https://github.com/otinn/llvm/blob/master/test/Instrumentation/MemorySafety/optimization/always_valid/0039.ll)

This pass could be improved by improving the pass that converts checks into fast checks where possible (exactcheck-opt) or by improving the range analysis.

```
// This file shows examples of optimize-fast-memory-checks being applied.
// The code is from 456.hmmer/src/mathsupport.c with the define statements from
// the corresponding config.h
#include "math.h"

#define INTSCALE    1000.0   /* scaling constant for floats to integer scores */
#define LOGSUM_TBL  20000    /* controls precision of ILogsum()               */

static int ilogsum_lookup[LOGSUM_TBL];
static void 
init_ilogsum(void)
{
  int i;
  // The fast load check into ilogsum_lookup will be optimized away by
  // optimize-fast-memory-checks because it can prove that all of the accesses
  // in the loop are always in bounds.
  for (i = 0; i < LOGSUM_TBL; i++) 
    ilogsum_lookup[i] = (int) (INTSCALE * 1.44269504 * 
           (log(1.+exp(0.69314718 * (float) -i/INTSCALE))));
}
int 
ILogsum(int p1, int p2)
{
  int    diff;
  static int firsttime = 1;
  // The firsttime variable will become a global variable and get a fast load
  // check for the if-condition and a fast store check for the later assignment.
  // Both of the checks will be optimized away by optimize-fast-memory-checks
  // because they are obviously in bounds.
  if (firsttime) { init_ilogsum(); firsttime = 0; }

  diff = p1-p2;
  if      (diff >=  LOGSUM_TBL) return p1;
  else if (diff <= -LOGSUM_TBL) return p2;
  else if (diff > 0)            return p1 + ilogsum_lookup[diff];
  else                          return p2 + ilogsum_lookup[-diff];
  // The preceding two loads from ilogsum_lookup get fast load checks because
  // the underlying memory object is easily identifiable.
  // The checks are currently not optimized away because the analysis passes
  // can't prove that they will always be in bounds.
}

```

## opt2 (optimize-identical-ls-checks) ##
This pass removes identical load/store checks that appear in the same basic block.
It works by iterating over the calls in a basic block.
If any call is encountered then one of three things happens:
1) It is a load/store check with a previously unseen access pointer and size pair. The pair is added to the known accesses cache.
2) It is a load/store check with a previously seen access pointer and size pair. The check is removed.
3) It is another call that is not in a whitelist of functions known to not deallocate memory. The known access cache is cleared.

Very similar behaviour already exists in ASan.

This pass could be improved by determining whether a call can change the results of the checks.

```
// This file shows an example of optimize-identical-ls-checks being applied.
// The code is from 401.bzip2/src/spec.c.

long int seedi;
double ran()
/* See "Random Number Generators: Good Ones Are Hard To Find", */
/*     Park & Miller, CACM 31#10 October 1988 pages 1192-1201. */
/***********************************************************/
/* THIS IMPLEMENTATION REQUIRES AT LEAST 32 BIT INTEGERS ! */
/***********************************************************/
#define _A_MULTIPLIER  16807L
#define _M_MODULUS     2147483647L /* (2**31)-1 */
#define _Q_QUOTIENT    127773L     /* 2147483647 / 16807 */
#define _R_REMAINDER   2836L       /* 2147483647 % 16807 */
{
  long lo;
  long hi;
  long test;

  // A load check will be inserted before the load from seedi.
  // It will not be converted into a fast check because it has common linkage.
  // Thus the check here will not be optimized away.
  hi = seedi / _Q_QUOTIENT;
  lo = seedi % _Q_QUOTIENT;
  test = _A_MULTIPLIER * lo - _R_REMAINDER * hi;

  // A store check will be inserted before the store into seedi.
  // It will not become a fast check for the same reason as the load check.
  // It will be optimized away by optimize-identical-ls-checks because it can
  // fail only if the load check does.
  if (test > 0) {
    seedi = test;
  } else {
    seedi = test + _M_MODULUS;
  }
  return ( (float) seedi / _M_MODULUS);
}
```

## opt3 (optimize-implied-fast-ls-checks) ##
This pass removes fast load/store checks that are implied by other fast load/store checks.
It works by traversing a dominator tree to find out which checks must always happen before other checks.
It removes fast load/store checks that are dominated by another fast load/store check with the same access offset, access size, and object size triple.

```
// This file shows an example of optimize-implied-fast-ls-checks being applied.
// The code is from 429.mcf/src/pbeampp.c with some definitions from the
// corresponding defines.h

typedef long flow_t;
typedef long cost_t;

typedef struct node *node_p;

typedef struct arc arc_t;
typedef struct arc *arc_p;

struct node
{
  cost_t potential; 
  int orientation;
  node_p child;
  node_p pred;
  node_p sibling;
  node_p sibling_prev;     
  arc_p basic_arc; 
  arc_p firstout, firstin;
  arc_p arc_tmp;
  flow_t flow;
  long depth; 
  int number;
  int time;
};

struct arc
{
  cost_t cost;
  node_p tail, head;
  int ident;
  arc_p nextout, nextin;
  flow_t flow;
  cost_t org_cost;
};

#define K 300
#define B  50

typedef struct basket
{
    arc_t *a;
    cost_t cost;
    cost_t abs_cost;
} BASKET;

static long basket_size;
static BASKET basket[B+K+1];
static BASKET *perm[B+K+1];

#ifdef _PROTO_
void sort_basket( long min, long max )
#else
void sort_basket( min, max )
    long min, max;
#endif
{
    long l, r;
    cost_t cut;
    BASKET *xchange;

    l = min; r = max;

    // The array access will get a fast load check that can't be optimized away
    // because the function could be called with parameters that would make this
    // check fail.
    // The load from the BASKET element will get a load check. It will not be
    // converted into a fast check because it will depend on the previous load.
    cut = perm[ (long)( (l+r) / 2 ) ]->abs_cost;

    do
    {
        // Both of the loops will get a similar check pair as the previous case.
        while( perm[l]->abs_cost > cut )
            l++;
        while( cut > perm[r]->abs_cost )
            r--;
            
        if( l < r )
        {
            // The loads on the right side will be avoided by reusing the data
            // from the while loops so there will be no load checks here.
            xchange = perm[l];
            // This array access will get a fast store check that will be
            // optimized away by optimize-implied-fast-ls-checks because it can
            // only fail if the fast load check in the first while loop fails.
            perm[l] = perm[r];
            // Same as the previous check except that it is caused by the fast
            // load check in the second while loop.
            perm[r] = xchange;
        }
        if( l <= r )
        {
            l++; r--;
        }

    }
    while( l <= r );

    if( min < r )
        sort_basket( min, r );
    if( l < max && l <= B )
        sort_basket( l, max ); 
}
```

## Overlapping optimization cases ##
Here is an example where each of the existing optimizations could optimize away the last check:

```
static int global_int;
void f(void) {
  ++global_int;
}
```

This creates the sequence:
  1. fast load check
  1. load
  1. add
  1. fast store check
  1. store

Whichever current optimization is run first will optimize away the
fast store check.

opt0 (ASan's global scalar optimization):
Both checks just use global scalars so they can be removed.

opt1 (optimize-fast-memory-checks):
Both checks are in bounds so they can be removed.

opt2 (identical checks in the same basic block; similar optimization in ASan):
The store check repeats the load check with no function calls between
them so it can be removed.

opt3 (optimize-implied-fast-ls-checks):
Both checks access the same range in a global variable of the same
size so the store check can be removed.

# Analysis #
Based on both compile-time and run-time data it looks like the single most useful optimization is opt1.

On chromium's browser\_tests it reduced the runtime by about 3% on average.
On the tested part of SPEC it removed 12% of load/store checks left over after the ASan optimizations (on average) although it got rid of only 2% in the median case.

# The Code #

The code is available in the modified LLVM and Clang repositories linked below. It works with unmodified compiler-rt.

https://github.com/otinn/llvm

https://github.com/otinn/clang

https://github.com/otinn/compiler-rt (unmodified but should always contain a version that works with the modified LLVM and Clang)

Some testing utilities are in this repository:

https://github.com/otinn/memory-safety-testing-utils