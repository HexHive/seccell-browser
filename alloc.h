/*
 * Copyright 2018, Data61
 * Commonwealth Scientific and Industrial Research Organisation (CSIRO)
 * ABN 41 687 119 230.
 *
 * This software may be distributed and modified according to the terms of
 * the BSD 2-Clause license. Note that NO WARRANTY is provided.
 * See "LICENSE_BSD2.txt" for details.
 *
 * @TAG(DATA61_BSD)
 */

/* A very simple, inefficient, non-freeing allocator for doing the tutorials */

/*
 * Allocate a slot from boot info. Allocates slots from info->empty.start.
 * This will not work if slots in the bootinfo empty range have already been used.
 *
 * @return a cslot that has not already been returned by this function, from the range
 *         specified in info->empty
 */
seL4_CPtr alloc_slot(seL4_BootInfo *info);

/*
 * Create an object of the desired type and size.
 *
 * This function iterates through the info->untyped capability range, attempting to
 * retype into an object of the provided type and size, until a successful allocation is made.
 *
 * @param type of the object to create
 * @param size of the object to create. Unused if the object is not variably sized.
 * @return a cslot containing the newly created untyped.
 */
seL4_CPtr alloc_object(seL4_BootInfo *info, seL4_Word type, seL4_Word size);
