One kind of bugs that AddressSanitizer can find with the help of code annotations is, as we call it, "container-overflow".
Simplest example:
```
#include <vector>
#include <assert.h>
typedef long T;
int main() {
  std::vector<T> v;
  v.push_back(0);
  v.push_back(1);
  v.push_back(2);
  assert(v.capacity() >= 4);
  assert(v.size() == 3);
  T *p = &v[0];
  // Here the memory is accessed inside a heap-allocated buffer
  // but outside of the region `[v.begin(), v.end())`.
  return p[3];  // OOPS.
  // v[3] could be detected by simple checks in std::vector.
  // *(v.begin()+3) could be detected by a mighty debug iterator
  // (&v[0])[3] can only be detected with AddressSanitizer or similar.
}
```

`std::vector` is annotated in LLVM's libc++ as of [r208319](http://llvm.org/viewvc/llvm-project?view=revision&revision=208319).
(See also [the discussion](http://lists.cs.uiuc.edu/pipermail/cfe-dev/2013-November/033649.html)).
We are also working on annotations for libstdc++: [first step](http://gcc.gnu.org/viewcvs/gcc?view=revision&revision=207517).
Next steps are to annotate `std::string` and `std::deque`.

Another side effect of container annotations will be improved sensitivity of LeakSanitizer.
In the following example we have a leak because a pointer is removed from a global vector using `pop_back`,
but w/o the annotations LeakSanitizer will treat the pointer as live because it still remains inside the vector's storage.

```
#include <vector>
std::vector<int *> *v;
int main(int argc, char **argv) {
  v = new std::vector<int *>;
  v->push_back(new int [10]);
  v->push_back(new int [20]);
  v->push_back(new int [30]);
  v->push_back(new int [40]);
  v->pop_back();  // The last element leaks now.
}
```