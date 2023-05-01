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

#include "include/ring_alloc.h"

RingAllocator RingAllocator::Create(u64 size) {
    RingAllocator allocator;
    allocator.backing_buf = VirtualReserve(size);
    VirtualCommit(size, allocator.backing_buf);
    allocator.buf_size = size;
    allocator.cursor = 0;
    allocator.generation = 0;
    return allocator;
}

void RingAllocator::Destroy(RingAllocator allocator) {
    VirtualDecommit(allocator.backing_buf, allocator.buf_size);
    VirtualRelease(allocator.backing_buf, allocator.buf_size);
}

u64 RingAllocator::alloc_raw(u64 bytes, i64 alignment) {
    ASSERT(bytes <= buf_size, "ring allocator isn't large enough for allocation");
    u64 cursor_aligned = (cursor + alignment - 1) & -alignment;
    if (bytes + cursor_aligned <= buf_size) {
	u64 offset = cursor_aligned;
	cursor = cursor_aligned + bytes;
	return offset;
    } else {
	cursor = bytes;
	++generation;
	return 0;
    }
}
