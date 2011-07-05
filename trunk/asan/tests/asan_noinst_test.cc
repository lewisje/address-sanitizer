/* Copyright 2011 Google Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

// This file is a part of AddressSanitizer, an address sanity checker.


#include "asan_allocator.h"
#include "asan_int.h"
#include "asan_mapping.h"
#include "asan_stack.h"

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include "gtest/gtest.h"

using namespace std;

TEST(AddressSanitizer, InternalSimpleDeathTest) {
  EXPECT_DEATH(exit(1), "");
}

static void MallocStress(size_t n) {
  AsanStackTrace stack1;
  stack1.trace[0] = 0xa123;
  stack1.trace[1] = 0xa456;
  stack1.size = 2;

  AsanStackTrace stack2;
  stack2.trace[0] = 0xb123;
  stack2.trace[1] = 0xb456;
  stack2.size = 2;

  AsanStackTrace stack3;
  stack3.trace[0] = 0xc123;
  stack3.trace[1] = 0xc456;
  stack3.size = 2;

  vector<void *> vec;
  for (size_t i = 0; i < n; i++) {
    if ((i % 3) == 0) {
      if (vec.empty()) continue;
      size_t idx = rand() % vec.size();
      void *ptr = vec[idx];
      vec[idx] = vec.back();
      vec.pop_back();
      __asan_free(ptr, &stack1);
    } else {
      size_t size = rand() % 1000 + 1;
      switch ((rand() % 128)) {
        case 0: size += 1024; break;
        case 1: size += 2048; break;
        case 2: size += 4096; break;
      }
      size_t alignment = 1 << (rand() % 10 + 1);
      char *ptr = (char*)__asan_memalign(alignment, size, &stack2);
      vec.push_back(ptr);
      for (size_t i = 0; i < size; i++) {
        ptr[i] = 0;
      }
    }
  }
  for (size_t i = 0; i < vec.size(); i++)
    __asan_free(vec[i], &stack3);
}


TEST(AddressSanitizer, InternalMallocTest) {
  MallocStress(1000000);
}


static void PrintShadow(const char *tag, uintptr_t ptr, size_t size) {
  fprintf(stderr, "%s shadow: %lx size % 3ld: ", tag, (long)ptr, (long)size);
  uintptr_t prev_shadow = 0;
  for (long i = -32; i < (long)size + 32; i++) {
    uintptr_t shadow = MemToShadow(ptr + i);
    if (i == 0 || i == (long)size)
      fprintf(stderr, ".");
    if (shadow != prev_shadow) {
      prev_shadow = shadow;
      fprintf(stderr, "%02x", (int)*(uint8_t*)shadow);
    }
  }
  fprintf(stderr, "\n");
}

TEST(AddressSanitizer, DISABLED_InternalPrintShadow) {
  for (size_t size = 1; size <= 513; size++) {
    char *ptr = new char[size];
    PrintShadow("m", (uintptr_t)ptr, size);
    delete [] ptr;
    PrintShadow("f", (uintptr_t)ptr, size);
  }
}

static uintptr_t pc_array[] = {
#if __WORDSIZE == 64
  0x7effbf756068ULL,
  0x7effbf75e5abULL,
  0x7effc0625b7cULL,
  0x7effc05b8997ULL,
  0x7effbf990577ULL,
  0x7effbf990c56ULL,
  0x7effbf992f3cULL,
  0x7effbf950c22ULL,
  0x7effc036dba0ULL,
  0x7effc03638a3ULL,
  0x7effc035be4aULL,
  0x7effc0539c45ULL,
  0x7effc0539a65ULL,
  0x7effc03db9b3ULL,
  0x7effc03db100ULL,
  0x7effc037c7b8ULL,
  0x7effc037bfffULL,
  0x7effc038b777ULL,
  0x7effc038021cULL,
  0x7effc037c7d1ULL,
  0x7effc037bfffULL,
  0x7effc038b777ULL,
  0x7effc038021cULL,
  0x7effc037c7d1ULL,
  0x7effc037bfffULL,
  0x7effc038b777ULL,
  0x7effc038021cULL,
  0x7effc037c7d1ULL,
  0x7effc037bfffULL,
  0x7effc0520d26ULL,
  0x7effc009ddffULL,
  0x7effbf90bb50ULL,
  0x7effbdddfa69ULL,
  0x7effbdde1fe2ULL,
  0x7effbdde2424ULL,
  0x7effbdde27b3ULL,
  0x7effbddee53bULL,
  0x7effbdde1988ULL,
  0x7effbdde0904ULL,
  0x7effc106ce0dULL,
  0x7effbcc3fa04ULL,
  0x7effbcc3f6a4ULL,
  0x7effbcc3e726ULL,
  0x7effbcc40852ULL,
  0x7effb681ec4dULL,
#endif  // __WORDSIZE
  0xB0B5E768,
  0x7B682EC1,
  0x367F9918,
  0xAE34E13,
  0xBA0C6C6,
  0x13250F46,
  0xA0D6A8AB,
  0x2B07C1A8,
  0x6C844F4A,
  0x2321B53,
  0x1F3D4F8F,
  0x3FE2924B,
  0xB7A2F568,
  0xBD23950A,
  0x61020930,
  0x33E7970C,
  0x405998A1,
  0x59F3551D,
  0x350E3028,
  0xBC55A28D,
  0x361F3AED,
  0xBEAD0F73,
  0xAEF28479,
  0x757E971F,
  0xAEBA450,
  0x43AD22F5,
  0x8C2C50C4,
  0x7AD8A2E1,
  0x69EE4EE8,
  0xC08DFF,
  0x4BA6538,
  0x3708AB2,
  0xC24B6475,
  0x7C8890D7,
  0x6662495F,
  0x9B641689,
  0xD3596B,
  0xA1049569,
  0x44CBC16,
  0x4D39C39F
};

TEST(AddressSanitizer, CompressStackTraceTest) {
  const size_t n = ASAN_ARRAY_SIZE(pc_array);
  uint32_t compressed[2 * n];

  for (int iter = 0; iter < 10000; iter++) {
    random_shuffle(pc_array, pc_array + n);
    AsanStackTrace stack0, stack1;
    stack0.CopyFrom(pc_array, n);
    stack0.size = std::max((size_t)1, (size_t)rand() % stack0.size);
    size_t compress_size = std::max((size_t)2, (size_t)rand() % (2 * n));
    size_t n_frames = AsanStackTrace::CompressStack(&stack0, compressed, compress_size);
    CHECK(n_frames <= stack0.size);
    AsanStackTrace::UncompressStack(&stack1, compressed, compress_size);
    //fprintf(stderr, "Compressed %ld frames to %ld words; uncompressed to %ld\n",
    //       (long)n_frames, (long)compress_size, (long)stack1.size);
    CHECK(stack1.size == n_frames);
    for (size_t i = 0; i < stack1.size; i++) {
      CHECK(stack0.trace[i] == stack1.trace[i]);
    }
  }
}

TEST(AddressSanitizer, QuarantineTest) {
  AsanStackTrace stack;
  stack.trace[0] = 0x890;
  stack.size = 1;

  const int size = 32;
  void *p = __asan_malloc(size, &stack);
  __asan_free(p, &stack);
  size_t i;
  size_t max_i = 1 << 30;
  for (i = 0; i < max_i; i++) {
    void *p1 = __asan_malloc(size, &stack);
    __asan_free(p1, &stack);
    if (p1 == p) break;
  }
  // fprintf(stderr, "i=%ld\n", i);
  EXPECT_GE(i, 100000U);
  EXPECT_LT(i, max_i);
}
