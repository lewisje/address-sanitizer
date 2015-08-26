The AddressSanitizer run-time library and the LLVM compiler module are being developed in the LLVM source tree. Please see the [LLVM Developer Policy](http://llvm.org/docs/DeveloperPolicy.html)

The run-time library resides in the [Compiler-RT](http://compiler-rt.llvm.org/) tree, under `lib/asan` as well as some shared files under `lib/sanitizer_common`, etc.

To request a merge into the sanitizer's code-base, please send patches based on the LLVM tree to `llvm-commits@cs.uiuc.edu`. For non-trivial patches please use [Phabricator](http://llvm.org/docs/Phabricator.html) -- this will help us reply faster.

The GCC version of AddressSanitizer run-time is a copy of the LLVM version, please do not commit changes to GCC trunk w/o committing them to LLVM trunk first. See also `libsanitizer/README.gcc` in the GCC trunk.

Some files only exist in the GCC tree, to make it interoperate with the rest of the GCC code. If you need to change them, you'll have to split your patch, apply the LLVM part first, merge it back to the GCC tree and then apply your patch there. We should aim to keep both trees in sync, so that GCC wouldn't need to go through this, but that's for another topic.

We're creating a set of guidelines for GCC developers to contribute back with ease, and below are some guidelines. Please, let us know if they don't work, of if there's a better way:

  1. Develop and test in GCC's sanitizer tree
  1. Take a diff one directory up (to avoid "libsanitizer")
  1. Apply the patch on an LLVM sanitizer's tree (inside compiler-rt/lib) and re-work the build to LLVM style.
  1. Test in the LLVM tree. For new architectures this is **hard** and you might need help. We are working to make it much easier, but in the interim please ask for help in the LLVM list.
  1. Submit the patch to LLVM's tree via list/Phabricator (see above)
  1. Commit to compiler-rt (or ask someone to do that for you).
  1. Merge back into GCC's libsanitizer, and apply the missing patches.

Please, remember: The GCC compiler module is developed according to the [general GCC rules](http://gcc.gnu.org/contribute.html).

If you are adding support for a new architecture/platform, we encourage you to [set up a public build bot](http://llvm.org/docs/HowToAddABuilder.html), otherwise we can not guarantee that we will keep your code in working conditions.