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

struct BumpAllocator {
    void *backing_buf;
    u64 reserved_size;
    u64 commit_size;
    u64 cursor;
    u32 blocks_committed;
    u32 generation;

    static BumpAllocator Create(u64 reserved_size, u64 commit_size);
    static void Destroy(BumpAllocator allocator);

    template<typename T>
    struct Pointer {
	u64 offset;
	u32 generation;
	BumpAllocator &allocator;
	
	T &operator* () {
	    ASSERT(generation == allocator.generation,
		   "bump allocator has been freed since pointer was created");
	    T *buf = reinterpret_cast<T *>(static_cast<u8 *>(allocator.backing_buf) + offset);
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

    u64 AllocRaw(u64 bytes, i64 alignment);
    void CommitNewBlocks(u32 num);
    void FreeAll();
    
    template<typename T>
    Pointer<T> Alloc(u64 num) {
	u64 offset_bytes = AllocRaw(num * sizeof(T), alignof(T));
	return {offset_bytes, generation, *this};
    }
};
