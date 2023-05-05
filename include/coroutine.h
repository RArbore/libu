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

#include <type_traits>
#include <tuple>

#include <setjmp.h>

#include "include/primitive_types.h"
#include "include/platform.h"

#define STACK_SIZE (1 << 12)
#define COMMIT_SIZE (1 << 15)
#define RESERVE_SIZE (1 << 20)
#define YIELD_STACK_SIZE (1 << 10)
#define RING_ALLOC_SIZE (1 << 10)
#define INIT_POST_SWITCH_STACK_SIZE 64

void *coroutine_allocate_stack();
jmp_buf *coroutine_push_yield_stack(jmp_buf *callee_context, void **ret_loc);
void coroutine_yield(void *ret);
void coroutine_yield_longjmp(void *ret);
void *coroutine_ring_alloc(u64 size, u64 align);

#define get_sp(p)				\
    asm volatile("movq %%rsp, %0" : "=r"(p))
#define get_fp(p)				\
    asm volatile("movq %%rbp, %0" : "=r"(p))
#define set_sp(p)				\
    asm volatile("movq %0, %%rsp" : : "r"(p))
#define set_fp(p)				\
    asm volatile("movq %0, %%rbp" : : "r"(p))

template <typename RTy>
void yield(RTy ret) {
    RTy *ret_alloc = reinterpret_cast<RTy *>(coroutine_ring_alloc(sizeof(RTy), alignof(RTy)));
    *ret_alloc = ret;
    coroutine_yield(ret_alloc);
}

inline void yield(void) {
    void *ret_alloc = reinterpret_cast<void *>(coroutine_ring_alloc(0, 0));
    coroutine_yield(ret_alloc);
}

struct YieldStackEntry {
    jmp_buf *callee_context_ptr;
    jmp_buf caller_context;
    void **ret_ptr;
};

template <typename RTy, typename... Args>
struct Coroutine {
    RTy (*func)(Args...);
    std::tuple<Args...> args;
    void *stack = nullptr;
    bool done = false;

    jmp_buf callee_context = {};
    jmp_buf caller_context = {};

    void *old_sp = nullptr;
    void *old_fp = nullptr;

    Coroutine(RTy (*_func)(Args...), Args... _args): func(_func), args({_args...}) {}

    void init() {
	stack = coroutine_allocate_stack();
	u8 *top_of_stack = reinterpret_cast<u8 *>(stack) + STACK_SIZE;
	*reinterpret_cast<decltype(this)*>(top_of_stack) = this;
	get_sp(old_sp);
	get_fp(old_fp);

	set_sp(top_of_stack - INIT_POST_SWITCH_STACK_SIZE);
	set_fp(top_of_stack);
	get_fp(top_of_stack);
	decltype(this) recovered_this = *reinterpret_cast<decltype(this)*>(top_of_stack);

	if (!setjmp(recovered_this->callee_context)) {
	    set_sp(recovered_this->old_sp);
	    set_fp(recovered_this->old_fp);
	    return;
	}
	RTy *ret_alloc = nullptr;
	if constexpr (std::is_same<RTy, void>::value) {
	    std::apply(recovered_this->func, recovered_this->args);
	} else {
	    RTy final_ret = std::apply(recovered_this->func, recovered_this->args);
	    ret_alloc = reinterpret_cast<RTy *>(coroutine_ring_alloc(sizeof(RTy), alignof(RTy)));
	    *ret_alloc = final_ret;
	}
	recovered_this->done = true;
	coroutine_yield_longjmp(ret_alloc);
    }

    RTy next() {
	ASSERT(!done, "can't call coroutine after it's finished");
	RTy *ret = nullptr;
	jmp_buf *caller_context_ptr = coroutine_push_yield_stack(&callee_context, reinterpret_cast<void **>(&ret));
	if (!setjmp(*caller_context_ptr)) {
	    longjmp(callee_context, 1);
	}
	if constexpr (!std::is_same<RTy, void>::value) {
	    return *ret;
	}
    }
};
