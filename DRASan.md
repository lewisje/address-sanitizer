DRASan is based on the combination of compiler and dynamic instrumentation [[1](http://swsys.ru/index.php?page=article&id=3593&lang=en)].<br>The dynamic instrumentation is based on DynamoRIO (DR), hence the prefix.<br>
<br>
DRASan source code can be found <a href='https://code.google.com/p/address-sanitizer/source/browse/#svn%2Ftrunk%2Fdynamorio'>here</a>.<br>
<br>
<br>
By applying the ASan instrumentation dynamically to system libraries, JIT and self-modifying code, DRASan provides extra code coverage during analysis.  Extra dynamic instrumentation doesn't add much steady-state execution slowdown.  However, the startup performance degrades on large projects - this issue is being worked on.<br>
<br>
DRASan is currently used for testing Chromium on <a href='http://blog.chromium.org/2012/04/fuzzing-for-security.html'>ClusterFuzz</a>.