#include <string.h>
#include <sel4/sel4.h>
#include <utils/util.h>
#include "alloc.h"
#include "scthreads.h"

/* Assembly function prototype - see scthreads.S */
void scthreads_switch_internal(void **contexts, seL4_Word usid, void *(*start_routine)(void *), void *restrict args, int flag);

//TODO: Allocate space for other contexts on the fly, just reserve in advance
void *scthreads_init_contexts(seL4_BootInfo *info, void *base_address, unsigned int secdiv_num) {
    seL4_UserContext **contexts;

    /* Currently, only SecDiv 1 (the initial SecDiv) is allowed to initialize the threading contexts */
    unsigned int usid;
    csrr_usid(usid);
    assert(usid == 1);

    seL4_Error error;
    /* Calculate / set necessary range sizes */
    seL4_Word context_list_size = ROUND_UP(secdiv_num * sizeof(seL4_UserContext *), BIT(seL4_MinRangeBits));
    seL4_Word context_size = ROUND_UP(sizeof(seL4_UserContext), BIT(seL4_MinRangeBits));
    seL4_Word stack_size = ROUND_UP(USER_STACK_SIZE, BIT(seL4_MinRangeBits));

    seL4_Word vaddr = (seL4_Word) base_address;

    /* Map context list */
    seL4_CPtr context_list = alloc_object(info, seL4_RISCV_RangeObject, context_list_size);
    error = seL4_RISCV_Range_Map(context_list, seL4_CapInitThreadVSpace, vaddr, seL4_ReadWrite,
                                 seL4_RISCV_ExecuteNever);
    ZF_LOGF_IF(error != seL4_NoError, "Failed to map context list @ %p", (void *)vaddr);
    /* Point global pointer to context list memory area */
    contexts = (seL4_UserContext **)vaddr;
    vaddr += context_list_size;

    /* Map and initialize contexts (including stacks) */
    for (unsigned int secdiv = 1; secdiv < secdiv_num; secdiv++) {
        if(secdiv != 1) {
            seL4_RISCV_RangeTable_AddSecDiv_t ret;
            ret = seL4_RISCV_RangeTable_AddSecDiv(seL4_CapInitThreadVSpace);
            ZF_LOGF_IF(ret.error != seL4_NoError, "Failed to create Secdiv");
            ZF_LOGF_IF(ret.id != secdiv, "Allocated different SD (%d) than expected (%d)", ret.id, secdiv);
        }

        /* Allocate and map context */
        seL4_CPtr context = alloc_object(info, seL4_RISCV_RangeObject, context_size);
        error = seL4_RISCV_Range_Map(context, seL4_CapInitThreadVSpace, vaddr, seL4_ReadWrite,
                                     seL4_RISCV_ExecuteNever);
        ZF_LOGF_IF(error != seL4_NoError, "Failed to map context @ %p", (void *)vaddr);
        /* Initialize context */
        seL4_UserContext *ctx = (seL4_UserContext *)vaddr;
        memset(ctx, 0, sizeof(seL4_UserContext));
        contexts[secdiv] = ctx;
        vaddr += context_size;

        /* Allocate and map stack */
        seL4_CPtr stack = alloc_object(info, seL4_RISCV_RangeObject, stack_size);
        error = seL4_RISCV_Range_Map(stack, seL4_CapInitThreadVSpace, vaddr, seL4_ReadWrite,
                                     seL4_RISCV_ExecuteNever);
        ZF_LOGF_IF(error != seL4_NoError, "Failed to map stack @ %p", (void *)vaddr);
        /* Assign stack to context (stack top (i.e., last accessible word) since stack grows downwards) */
        ctx->sp = vaddr + stack_size - sizeof(seL4_Word);
        vaddr += stack_size;

        /* Transfer access privileges only to the SecDiv in question (that is not the initial SecDiv) */
        if (secdiv != 1) {            
            error = seL4_RISCV_RangeTable_GrantSecDivPermissions(seL4_CapInitThreadVSpace,
                                                                (seL4_Word)secdiv,
                                                                (seL4_Word)ctx->sp, 
                                                                RT_R | RT_W);
            ZF_LOGF_IF(error != seL4_NoError, "Failed to grant compartment permissions");            
            prot(ctx->sp, 0);
            error = seL4_RISCV_RangeTable_GrantSecDivPermissions(seL4_CapInitThreadVSpace,
                                                                (seL4_Word)secdiv,
                                                                (seL4_Word)ctx, 
                                                                RT_R | RT_W);
            ZF_LOGF_IF(error != seL4_NoError, "Failed to grant compartment permissions");            
            prot(ctx, 0);
        }
    }

    /* Make context list read-only for all SecDivs */
    for (unsigned int secdiv = 2; secdiv < secdiv_num; secdiv++) {
        error = seL4_RISCV_RangeTable_GrantSecDivPermissions(seL4_CapInitThreadVSpace,
                                                                (seL4_Word)secdiv,
                                                                (seL4_Word)contexts, 
                                                                RT_R);
        // grant(contexts, secdiv, RT_R);
    }
    prot(contexts, RT_R);

    return contexts;
}

void __attribute__((optimize(2))) 
scthreads_switch(void **contexts, seL4_Word target_usid) {
    scthreads_switch_internal(contexts, target_usid, NULL, NULL, 0);
}
