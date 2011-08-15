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

// Implementation of DynamoRIO instrumentation for ASan

#include "dr_api.h"

#define TESTALL(mask, var) (((mask) & (var)) == (mask))
#define TESTANY(mask, var) (((mask) & (var)) != 0)

#define CHECK_IMPL(condition, file, line) \
    do { \
      if (!(condition)) { \
        dr_printf("Check failed: `%s`\nat %s:%d\n", #condition, file, line); \
        dr_abort(); \
      } \
    } while(0)  // TODO: stacktrace

#define CHECK(condition) CHECK_IMPL(condition, __FILE__, __LINE__)

//#define VERBOSE_VERBOSE

#if defined(VERBOSE_VERBOSE) && !defined(VERBOSE)
# define VERBOSE
#endif

bool OperandIsInteresting(opnd_t opnd) {
  // TOTHINK: we may access waaaay beyound the stack, do we need to check it?
  return
      (opnd_is_memory_reference(opnd) &&
       (!opnd_is_base_disp(opnd) ||
        (reg_to_pointer_sized(opnd_get_base(opnd)) != DR_REG_XSP &&
         reg_to_pointer_sized(opnd_get_base(opnd)) != DR_REG_XBP) ||
        opnd_get_index(opnd) != DR_REG_NULL ||
        opnd_is_far_memory_reference(opnd)));
}

bool WantToInstrument(instr_t *instr) {
  if (instr_ok_to_mangle(instr) == false)  // TODO: WTF is this?
    return false;

  /*  TODO
  if (instr_reads_memory(instr)) {
    for (int s = 0; s < instr_num_srcs(instr); s++) {
      opnd_t op = instr_get_src(instr, s);
      if (OperandIsInteresting(op))
        return true;
    }
  }
  */

  if (instr_writes_memory(instr)) {
    for (int d = 0; d < instr_num_dsts(instr); d++) {
      opnd_t op = instr_get_dst(instr, d);
      if (OperandIsInteresting(op))
        return true;
    }
  }

  return false;
}

bool
event_restore_state(void *drcontext, bool restore_memory,
                    dr_restore_state_info_t *info)
{
  // This guy is called each time our instrumentation generates a fault.

  // TODO: do we need anything smarter?
  return true;
}

static dr_emit_flags_t
event_basic_block(void *drcontext, void *tag, instrlist_t *bb,
                  bool for_trace, bool translating)
{
  // TOFILE: `tag` should be (byte*) ?

  // TODO: auto whitelist
  const unsigned int WHITELIST[] = {
    0x08052ba4,
    ~0};
  for (int w = 0; ; w++) {
    if (WHITELIST[w] == (unsigned int)tag)  // in the whitelist
      break;
    else if (WHITELIST[w] == ~0u)  // not in whitelist
      return DR_EMIT_DEFAULT;
  }

#if defined(VERBOSE)
  dr_printf("To be instrumented: %p; translating = %s\n",
            tag, translating ? "true" : "false");
  instrlist_disassemble(drcontext, (byte*)tag, bb, STDOUT);
#else
  dr_printf("Instrumenting: %p; translating = %s\n",
            tag, translating ? "true" : "false");
#endif

#define PRE(at, what) instrlist_meta_preinsert(bb, at, INSTR_CREATE_##what);
#define PREF(at, what) instrlist_meta_preinsert(bb, at, what);

  for (instr_t *i = instrlist_first(bb); i != NULL; i = instr_get_next(i)) {
    if (!WantToInstrument(i))
      continue;

#if defined(VERBOSE_VERBOSE)
    uint flags = instr_get_arith_flags(i);
    dr_printf("+%d -> to be instrumented! [flags = 0x%08X]\n",
              instr_get_app_pc(i) - (byte*)tag, flags);
#endif

    // TODO: instrument reads

    // instrument writes
    if (instr_writes_memory(i)) {
      bool instrumented_anything = false;
      for (int d = 0; d < instr_num_dsts(i); d++) {
        opnd_t op = instr_get_dst(i, d);
        if (!OperandIsInteresting(op) || opnd_get_base(op) == DR_REG_NULL)
          continue;

        CHECK(!instrumented_anything);
        instrumented_anything = true;

        bool need_to_restore_eflags = false;
        uint flags = instr_get_arith_flags(i);
        if (!TESTALL(EFLAGS_WRITE_6, flags) || TESTANY(EFLAGS_READ_6, flags)) {
          // TODO: for some reason, spilling eflags makes ASAN RTL
          // a bit crazy: it detects a simple OOB write as use-after-free...
#if 0
#if defined(VERBOSE_VERBOSE)
          dr_printf("Spilling eflags...\n");
#endif
          need_to_restore_eflags = true;
          dr_save_arith_flags(drcontext, bb, i, SPILL_SLOT_3);
#endif
        }

        reg_id_t R1 = opnd_get_base(op),  // Register #2 memory address is already there!
                 R1_8 = reg_32_to_opsz(R1, OPSZ_1),  // TODO: on x64?
                 R2 = (R1 == DR_REG_XCX ? DR_REG_XDX : DR_REG_XCX),
                 R2_8 = reg_32_to_opsz(R2, OPSZ_1);
        CHECK(reg_to_pointer_sized(R1) == R1);  // otherwise R2 may be wrong.

        // Save the current values of R1 and R2.
        dr_save_reg(drcontext, bb, i, R1, SPILL_SLOT_1);
        // TODO: Something smarter than spilling a "fixed" register R2?
        dr_save_reg(drcontext, bb, i, R2, SPILL_SLOT_2);

        PRE(i, shr(drcontext, opnd_create_reg(R1), OPND_CREATE_INT8(3)));
        PRE(i, mov_ld(drcontext, opnd_create_reg(R2), OPND_CREATE_MEM32(R1,0x20000000)));
        PRE(i, test(drcontext, opnd_create_reg(R2_8), opnd_create_reg(R2_8)));

        instr_t *OK_label = INSTR_CREATE_label(drcontext);
        PRE(i, jcc(drcontext, OP_je_short, opnd_create_instr(OK_label)));

        opnd_size_t access_size = opnd_get_size(op);
        CHECK(access_size != OPSZ_NA);
        if (access_size != OPSZ_8) {
          // Slowpath to support accesses smaller than pointer-sized.
          dr_restore_reg(drcontext, bb, i, R1, SPILL_SLOT_1);
          PRE(i, and(drcontext, opnd_create_reg(R1), OPND_CREATE_INT8(7)));
          switch (access_size) {
          case OPSZ_4:
            PRE(i, add(drcontext, opnd_create_reg(R1), OPND_CREATE_INT8(3)));
            break;
          case OPSZ_2:
            PRE(i, add(drcontext, opnd_create_reg(R1), OPND_CREATE_INT8(2)));
            break;
          case OPSZ_1:
            PRE(i, inc(drcontext, opnd_create_reg(R1)));
            break;
          default:
            CHECK(0);
          }
          PRE(i, cmp(drcontext, opnd_create_reg(R1_8), opnd_create_reg(R2_8)));
          PRE(i, jcc(drcontext, OP_je_short, opnd_create_instr(OK_label)));
        }

        // Trap code:
        // 1) Restore the original access address in XAX 
        dr_restore_reg(drcontext, bb, i, DR_REG_XAX, SPILL_SLOT_1);
        // 2) Send SIGILL to be handled by ASan RTL 
        instrlist_meta_fault_preinsert(bb, i,
                                       INSTR_XL8(
                                           INSTR_CREATE_ud2a(drcontext),
                                           instr_get_app_pc(i))
                                      );
        // NOTE: we don't need to put extra access info (size/is_write)
        // since RTL will see the UNinstrumented code on SIGILL!
        // TODO: review and commit the asan_rtl.cc change accounting for this.

        PREF(i, OK_label);
        // Restore the registers and flags.
        dr_restore_reg(drcontext, bb, i, R1, SPILL_SLOT_1);
        dr_restore_reg(drcontext, bb, i, R2, SPILL_SLOT_2);
        if (need_to_restore_eflags) {
#if defined(VERBOSE_VERBOSE)
          dr_printf("Restoring eflags\n");
#endif
          dr_restore_arith_flags(drcontext, bb, i, SPILL_SLOT_3);
        }
        // The original instruction is left untouched.
      }
    }
  }

#if defined(VERBOSE_VERBOSE)
  dr_printf("\nFinished instrumenting dynamorio_basic_block(tag="PFX")\n", tag);
  instrlist_disassemble(drcontext, (byte*)tag, bb, STDOUT);
#endif
  return DR_EMIT_DEFAULT;
}

void event_exit() {
  dr_printf("==DRASAN== DONE\n");
}

DR_EXPORT void dr_init(client_id_t id) {
  dr_register_exit_event(event_exit);
  dr_register_bb_event(event_basic_block);
  dr_register_restore_state_ex_event(event_restore_state);
  dr_printf("==DRASAN== Starting!\n");
}
