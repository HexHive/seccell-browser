#include <scthreads.h>

.section .text

.global scthreads_switch_internal
.global scthreads_start
.global scthreads_end
.global scthreads_call
.global scthreads_return

scthreads_start:
    nop

scthreads_save_context:
    /*
     * (preserved) a0: contexts
     * (preserved) a1: target usid
     * (preserved) a2: continuation address (if call)
     * (preserved) a3: args pointer (if call) or return value pointer (if return)
     * (preserved) a4: flag: switch (0) or call (1) or return from call (2)?
     */
    /* Use t0, t1, t2 as temporaries */
    /* 1. Calculate address of state store from contexts (a0) */
    csrr t0, usid
    slli t0, t0, WORDSIZESHIFT
    add  t0, t0, a0
    ld   t0, (t0)
    /* In case of returning from thread: need to restore caller USID from context */
    addi t1, a4, -2
    bnez t1, .save_context_if_switch_or_call
.load_return_usid:
    /* Load urid from context */
    ld a1,  (32*WORDSIZE)(t0)
    ret
.save_context_if_switch_or_call:
    /* Save return address, currently on stack */
    ld t1, (sp)
    sd t1, (1*WORDSIZE)(t0)
    /* Save SP and caller-saved registers */
    sd sp, (2*WORDSIZE)(t0)
    sd s0,  (4*WORDSIZE)(t0)
    sd s1,  (5*WORDSIZE)(t0)
    sd s2,  (6*WORDSIZE)(t0)
    sd s3,  (7*WORDSIZE)(t0)
    sd s4,  (8*WORDSIZE)(t0)
    sd s5,  (9*WORDSIZE)(t0)
    sd s6,  (10*WORDSIZE)(t0)
    sd s7,  (11*WORDSIZE)(t0)
    sd s8,  (12*WORDSIZE)(t0)
    sd s9,  (13*WORDSIZE)(t0)
    sd s10, (14*WORDSIZE)(t0)
    sd s11, (15*WORDSIZE)(t0)
    ret

scthreads_restore_context:
    /*
     * a0: contexts
     * a1: --------
     * a2: continuation address (if call)
     * a3: args pointer (if call) or return value pointer (if return)
     * a4: flag: switch (0) or call (1) or return from call (2)?
     */
    csrr t0, usid
    slli t0, t0, WORDSIZESHIFT
    add  a0, a0, t0
    ld   a0, (a0)
    /* Restore SP, and adjust for frame of scthreads_switch_internal */
    ld   sp, (2*WORDSIZE)(a0)
    addi sp, sp, WORDSIZE
    /* Switch and return restore state, call sets up args */
    addi a4, a4, -1
    bnez a4, .restore_context_if_switch_return
.setup_args_if_call:
    /* Save the urid in the context for later */
    csrr t0, urid
    sd   t0, (32*WORDSIZE)(a0)
    mv   a0, a3
    mv   ra, a2
    ret
.restore_context_if_switch_return:
    /* Restore caller-saved registers */
    ld s0,  (4*WORDSIZE)(a0)
    ld s1,  (5*WORDSIZE)(a0)
    ld s2,  (6*WORDSIZE)(a0)
    ld s3,  (7*WORDSIZE)(a0)
    ld s4,  (8*WORDSIZE)(a0)
    ld s5,  (9*WORDSIZE)(a0)
    ld s6,  (10*WORDSIZE)(a0)
    ld s7,  (11*WORDSIZE)(a0)
    ld s8,  (12*WORDSIZE)(a0)
    ld s9,  (13*WORDSIZE)(a0)
    ld s10, (14*WORDSIZE)(a0)
    ld s11, (15*WORDSIZE)(a0)
    ld ra,  (1*WORDSIZE)(a0)
    /* Move return value into a0 */
    mv a0, a3
    ret

scthreads_switch_internal:
    /*
     * a0: contexts
     * a1: target usid
     * a2: continuation address (if call)
     * a3: args pointer (if call) or return value pointer (if return)
     * a4: flag: switch (0) or call (1) or return from call (2)?
     */
    addi sp, sp, -WORDSIZE
    sd ra, 0(sp)
    jal scthreads_save_context
    addi sp, sp, WORDSIZE
    jals a1, .switch_sd
.switch_sd:
    entry
    /* scthreads_restore_context is a no-return function => execution continues as defined in the saved context */
    j scthreads_restore_context

scthreads_call:
    /*
     * a0: contexts
     * a1: target usid
     * a2: continuation address (if call)
     * a3: args pointer (if call) or return value pointer (if return)
     */
   addi    sp,sp,-(2*WORDSIZE)
   li      a4,1
   sd      ra, WORDSIZE(sp)
   jal     scthreads_switch_internal
   ld      ra, WORDSIZE(sp)
   addi    sp,sp,(2*WORDSIZE)
   ret

scthreads_return:
   mv      a3,a1
   li      a4,2
   li      a2,0
   li      a1,0
   j       scthreads_switch_internal

scthreads_end:
    nop
