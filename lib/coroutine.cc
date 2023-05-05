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
static thread_local YieldStackEntry yield_stack[YIELD_STACK_SIZE];
static thread_local u16 yield_stack_top = 0;

static thread_local Singleton<RingAllocator, u64> ring_alloc(RING_ALLOC_SIZE);

void *coroutine_allocate_stack() {
    return reinterpret_cast<void *>(slab_alloc->alloc<u8>().data());
}

jmp_buf *coroutine_push_yield_stack(jmp_buf *callee_context, void **ret_loc) {
    yield_stack[yield_stack_top].callee_context_ptr = callee_context;
    jmp_buf *caller_context_ptr = &yield_stack[yield_stack_top].caller_context;
    yield_stack[yield_stack_top].ret_ptr = ret_loc;
    ++yield_stack_top;
    return caller_context_ptr;
}

void coroutine_yield(void *ret) {
    ASSERT(yield_stack_top, "cannot yield when there are no contexts on the yield stack");
    if (!setjmp(*yield_stack[yield_stack_top - 1].callee_context_ptr)) {
	coroutine_yield_longjmp(ret);
    }
}

void coroutine_yield_longjmp(void *ret) {
    ASSERT(yield_stack_top, "cannot yield when there are no contexts on the yield stack");
    --yield_stack_top;
    *yield_stack[yield_stack_top].ret_ptr = ret;
    longjmp(yield_stack[yield_stack_top].caller_context, 1);
}

void *coroutine_ring_alloc(u64 size, u64 align) {
    return reinterpret_cast<u8 *>(ring_alloc->backing_buf) + ring_alloc->alloc_raw(size, align);
}
