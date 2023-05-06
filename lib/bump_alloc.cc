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

#include "include/bump_alloc.h"

BumpAllocator BumpAllocator::Create(u64 reserved_size, u64 commit_size) {
    BumpAllocator allocator;
    allocator.backing_buf = VirtualReserve(reserved_size);
    allocator.reserved_size = reserved_size;
    allocator.commit_size = commit_size;
    allocator.cursor = 0;
    allocator.blocks_committed = 0;
    allocator.generation = 0;
    return allocator;
}

void BumpAllocator::Destroy(BumpAllocator allocator) {
    for (u32 i = 0; i < allocator.blocks_committed; ++i) {
	VirtualDecommit(static_cast<char *>(allocator.backing_buf) + i * allocator.commit_size, allocator.commit_size);
    }
    VirtualRelease(allocator.backing_buf, allocator.reserved_size);
}

u64 BumpAllocator::AllocRaw(u64 bytes, i64 alignment) {
    u64 cursor_aligned = (cursor + alignment - 1) & -alignment;
    ASSERT(cursor_aligned + bytes <= reserved_size, "bump allocator isn't large enough for allocation");
    i64 spill = cursor_aligned + bytes - blocks_committed * commit_size;
    if (spill > 0) {
	CommitNewBlocks(static_cast<u32>((spill + commit_size - 1) / commit_size));
    }
    u64 offset = cursor_aligned;
    cursor = cursor_aligned + bytes;
    return offset;
}

void BumpAllocator::CommitNewBlocks(u32 num) {
    for (u32 i = blocks_committed; i < blocks_committed + num; ++i) {
	VirtualCommit(commit_size, static_cast<char *>(backing_buf) + i * commit_size);
    }
    blocks_committed += num;
}

void BumpAllocator::FreeAll() {
    cursor = 0;
    ++generation;
}
