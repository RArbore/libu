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

SlabAllocator SlabAllocator::Create(u64 reserved_size, u64 commit_size, u32 slab_size) {
    SlabAllocator allocator;
    allocator.backing_buf = VirtualReserve(reserved_size);
    allocator.reserved_size = reserved_size;
    ASSERT(commit_size >= slab_size, "can't create slab allocator with commit_size smaller than slab_size");
    ASSERT(commit_size % slab_size == 0, "can't create slab allocator where slabs don't fit neatly into commits");
    allocator.commit_size = commit_size;
    ASSERT(slab_size >= sizeof(i32), "can't create slab allocator with slab_size smaller than sizeof(i32)");
    allocator.slab_size = slab_size;
    allocator.blocks_committed = 0;
    allocator.head_free_slab = -1;
    return allocator;
}

void SlabAllocator::Destroy(SlabAllocator allocator) {
    for (u32 i = 0; i < allocator.blocks_committed; ++i) {
	VirtualDecommit(static_cast<char *>(allocator.backing_buf) + i * allocator.commit_size, allocator.commit_size);
    }
    VirtualRelease(allocator.backing_buf, allocator.reserved_size);
}

u64 SlabAllocator::alloc_raw() {
    if (head_free_slab < 0) {
	i32 prev_num_slabs = static_cast<i32>(blocks_committed * commit_size / slab_size);
	commit_new_blocks(1);
	i32 new_num_slabs = static_cast<i32>(commit_size / slab_size);
	for (i32 i = prev_num_slabs; i < new_num_slabs; ++i) {
	    *reinterpret_cast<i32 *>(static_cast<u8 *>(backing_buf) + i * slab_size) = i + 1 == new_num_slabs ? -1 : i + 1;
	}
	head_free_slab = prev_num_slabs;
    }
    i32 free_slab = head_free_slab;
    head_free_slab = *reinterpret_cast<i32 *>(static_cast<u8 *>(backing_buf) + free_slab * slab_size);
    return static_cast<u64>(free_slab * slab_size);
}

void SlabAllocator::commit_new_blocks(u32 num) {
    for (u32 i = blocks_committed; i < blocks_committed + num; ++i) {
	VirtualCommit(commit_size, static_cast<char *>(backing_buf) + i * commit_size);
    }
    blocks_committed += num;
}

void SlabAllocator::free(void *ptr) {
    *reinterpret_cast<i32 *>(ptr) = head_free_slab;
    head_free_slab = static_cast<i32>((reinterpret_cast<u8 *>(ptr) - reinterpret_cast<u8 *>(backing_buf)) / slab_size);
}

void SlabAllocator::free_all() {
    i32 num_slabs = static_cast<i32>(blocks_committed * commit_size / slab_size);
    for (i32 i = 0; i < num_slabs; ++i) {
	*reinterpret_cast<i32 *>(static_cast<u8 *>(backing_buf) + i * slab_size) = i + 1 == num_slabs ? -1 : i + 1;
    }
    head_free_slab = 0;
}
