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

#define STACK_SIZE (1 << 15)
#define COMMIT_SIZE (1 << 15)
#define RESERVE_SIZE (1 << 24)
#define YIELD_STACK_SIZE (1 << 10)
#define RING_ALLOC_SIZE (1 << 10)
#define INIT_POST_SWITCH_STACK_SIZE 64

void *CoroutineAllocateStack();
void CoroutineDestroyStack(void *stack);
jmp_buf *CoroutinePushYieldStack(jmp_buf *callee_context, void **ret_loc);
void CoroutineYield(void *ret);
void CoroutineYieldLongjmp(void *ret);
void *CoroutineRingAlloc(u64 size, u64 align);

#define GetSP(p)				\
    asm volatile("movq %%rsp, %0" : "=r"(p))
#define GetFP(p)				\
    asm volatile("movq %%rbp, %0" : "=r"(p))
#define SetSP(p)				\
    asm volatile("movq %0, %%rsp" : : "r"(p))
#define SetFP(p)				\
    asm volatile("movq %0, %%rbp" : : "r"(p))

template <typename RTy>
void Yield(RTy ret) {
    RTy *ret_alloc = reinterpret_cast<RTy *>(CoroutineRingAlloc(sizeof(RTy), alignof(RTy)));
    *ret_alloc = ret;
    CoroutineYield(ret_alloc);
}

inline void Yield(void) {
    void *ret_alloc = reinterpret_cast<void *>(CoroutineRingAlloc(0, 0));
    CoroutineYield(ret_alloc);
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

    Coroutine(RTy (*_func)(Args...)): func(_func) {}

    void Create(Args... _args) {
	args = {_args...};
	stack = CoroutineAllocateStack();
	u8 *top_of_stack = reinterpret_cast<u8 *>(stack) + STACK_SIZE;
	*reinterpret_cast<decltype(this)*>(top_of_stack) = this;
	GetSP(old_sp);
	GetFP(old_fp);

	SetSP(top_of_stack - INIT_POST_SWITCH_STACK_SIZE);
	SetFP(top_of_stack);
	GetFP(top_of_stack);
	decltype(this) recovered_this = *reinterpret_cast<decltype(this)*>(top_of_stack);

	if (!setjmp(recovered_this->callee_context)) {
	    SetSP(recovered_this->old_sp);
	    SetFP(recovered_this->old_fp);
	    return;
	}
	RTy *ret_alloc = nullptr;
	if constexpr (std::is_same<RTy, void>::value) {
	    std::apply(recovered_this->func, recovered_this->args);
	} else {
	    RTy final_ret = std::apply(recovered_this->func, recovered_this->args);
	    ret_alloc = reinterpret_cast<RTy *>(CoroutineRingAlloc(sizeof(RTy), alignof(RTy)));
	    *ret_alloc = final_ret;
	}
	recovered_this->done = true;
	CoroutineYieldLongjmp(ret_alloc);
    }

    RTy Next() {
	ASSERT(!done, "can't call coroutine after it's finished");
	RTy *ret = nullptr;
	jmp_buf *caller_context_ptr = CoroutinePushYieldStack(&callee_context, reinterpret_cast<void **>(&ret));
	if (!setjmp(*caller_context_ptr)) {
	    longjmp(callee_context, 1);
	}
	if constexpr (!std::is_same<RTy, void>::value) {
	    return *ret;
	}
    }

    void Destroy() {
	CoroutineDestroyStack(stack);
    }
};
