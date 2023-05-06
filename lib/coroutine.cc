/*
 * This file is part of libu.
 * libu is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 * libu is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with libu. If not, see <https://www.gnu.org/licenses/>.
 */

#include "include/slab_alloc.h"
#include "include/ring_alloc.h"
#include "include/coroutine.h"
#include "include/singleton.h"
#include "include/log.h"

static thread_local Singleton<SlabAllocator, u64, u64, u32> slab_alloc(RESERVE_SIZE, COMMIT_SIZE, STACK_SIZE);
static thread_local YieldStackEntry Yield_stack[YIELD_STACK_SIZE];
static thread_local u16 Yield_stack_top = 0;

static thread_local Singleton<RingAllocator, u64> ring_alloc(RING_ALLOC_SIZE);

void *CoroutineAllocateStack() {
    return reinterpret_cast<void *>(slab_alloc->Alloc<u8>().data());
}

void CoroutineDestroyStack(void *stack) {
    slab_alloc->Free(stack);
}

jmp_buf *CoroutinePushYieldStack(jmp_buf *callee_context, void **ret_loc) {
    Yield_stack[Yield_stack_top].callee_context_ptr = callee_context;
    jmp_buf *caller_context_ptr = &Yield_stack[Yield_stack_top].caller_context;
    Yield_stack[Yield_stack_top].ret_ptr = ret_loc;
    ++Yield_stack_top;
    return caller_context_ptr;
}

void CoroutineYield(void *ret) {
    ASSERT(Yield_stack_top, "cannot Yield when there are no contexts on the Yield stack");
    if (!setjmp(*Yield_stack[Yield_stack_top - 1].callee_context_ptr)) {
	CoroutineYieldLongjmp(ret);
    }
}

void CoroutineYieldLongjmp(void *ret) {
    ASSERT(Yield_stack_top, "cannot Yield when there are no contexts on the Yield stack");
    --Yield_stack_top;
    *Yield_stack[Yield_stack_top].ret_ptr = ret;
    longjmp(Yield_stack[Yield_stack_top].caller_context, 1);
}

void *CoroutineRingAlloc(u64 size, u64 align) {
    return reinterpret_cast<u8 *>(ring_alloc->backing_buf) + ring_alloc->AllocRaw(size, align);
}
