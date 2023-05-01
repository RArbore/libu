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

#pragma once

#include "include/primitive_types.h"
#include "include/platform.h"

struct RingAllocator {
    void *backing_buf;
    u64 buf_size;
    u64 cursor;
    u32 generation;

    static RingAllocator Create(u64 size);
    static void Destroy(RingAllocator allocator);

    template<typename T>
    struct Pointer {
	u32 offset;
	u32 generation;
	RingAllocator &allocator;
	
	T &operator* () {
	    ASSERT(generation == allocator.generation ||
		   (generation == allocator.generation - 1 &&
		    offset >= allocator.cursor),
		   "ring allocator has overwritten memory pointed to");
	    T *buf = static_cast<T *>(allocator.backing_buf);
	    return buf[offset];
	}
	
	T *operator-> () {
	    return &**this;
	}
	
	T &operator[] (u64 idx) {
	    return (&**this)[idx];
	}

	T *data() {
	    return &**this;
	}
    };

    u64 alloc_raw(u64 bytes, i64 alignment);
    
    template<typename T>
    Pointer<T> alloc(u64 num) {
	u64 offset_bytes = alloc_raw(num * sizeof(T), alignof(T));
	u32 offset_t = static_cast<u32>(offset_bytes / sizeof(T));
	return {offset_t, generation, *this};
    }
};
