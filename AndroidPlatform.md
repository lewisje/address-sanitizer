# Introduction #

AddressSanitizer is supported in AOSP starting with JellyBean? release.

# Details #

To run applications built with ASan you'll need an -eng build of Android.

To build any part of Android system with ASan, add LOCAL\_ADDRESS\_SANITIZER:=true to the appropriate Android.mk.

To run a standalone native binary, prefix it with asanwrapper: /system/bin/asanwrapper _path\_to\_your\_binary_ _options_