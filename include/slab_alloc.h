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

struct SlabAllocator {
    void *backing_buf;
    u64 reserved_size;
    u64 commit_size;
    u32 slab_size;
    u32 blocks_committed;
    i32 head_free_slab;

    static SlabAllocator Create(u64 reserved_size, u64 commit_size, u32 slab_size);
    static void Destroy(SlabAllocator allocator);

    template<typename T>
    struct Pointer {
	u64 offset;
	SlabAllocator *allocator;
	
	T &operator* () {
	    T *buf = reinterpret_cast<T *>(static_cast<u8 *>(allocator->backing_buf) + offset);
	    return *buf;
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

    u64 alloc_raw();
    void commit_new_blocks(u32 num);
    
    template<typename T>
    Pointer<T> alloc() {
	u64 offset_bytes = alloc_raw();
	return {offset_bytes, this};
    }

    void free(void *ptr);
    template<typename T>
    void free(Pointer<T> ptr) {
	free(ptr.data());
    }
    void free_all();
};
