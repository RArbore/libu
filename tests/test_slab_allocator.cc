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

#include <string.h>

#include "libu.h"

int main() {
    SlabAllocator alloc1 = SlabAllocator::Create(4096, 256, 256);
    auto buf1 = alloc1.Alloc<u8>();
    for (u16 i = 0; i < 256; ++i) {
	buf1[i] = static_cast<u8>('A' + (i % 10));
    }
    for (u16 i = 0; i < 256; ++i) {
	ASSERT(buf1[i] == static_cast<u8>('A' + (i % 10)), "");
    }

    auto buf2 = alloc1.Alloc<u8>();
    for (u16 i = 0; i < 256; ++i) {
	buf2[i] = static_cast<u8>('a' + (i % 10));
    }
    for (u16 i = 0; i < 256; ++i) {
	ASSERT(buf2[i] == static_cast<u8>('a' + (i % 10)), "");
    }
    for (u16 i = 0; i < 256; ++i) {
	ASSERT(buf1[i] == static_cast<u8>('A' + (i % 10)), "");
    }

    alloc1.Free(buf1);
    auto buf3 = alloc1.Alloc<u8>();
    for (u16 i = 0; i < 256; ++i) {
	buf3[i] = static_cast<u8>('a' + (i % 10));
    }
    for (u16 i = 0; i < 256; ++i) {
	ASSERT(buf3[i] == static_cast<u8>('a' + (i % 10)), "");
    }
    for (u16 i = 0; i < 256; ++i) {
	ASSERT(buf1[i] == static_cast<u8>('a' + (i % 10)), "");
    }
    SlabAllocator::Destroy(alloc1);

    constexpr u32 slab_size = 1 << 8;
    constexpr u64 num_allocs = 1 << 8;
    SlabAllocator alloc2 = SlabAllocator::Create(1 << 20, 1 << 12, slab_size);
    u64 buf4s_size = num_allocs * sizeof(SlabAllocator::Pointer<u8>);
    SlabAllocator::Pointer<u8> *buf4s = reinterpret_cast<SlabAllocator::Pointer<u8> *>(VirtualReserve(buf4s_size));
    VirtualCommit(buf4s_size, buf4s);
    u64 permute[num_allocs];
    i32 primes[16] = {3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59};
    for (u64 p = 0; p < 16; ++p) {
	i32 n = primes[p];
	for (u64 i = 0; i < num_allocs; ++i) {
	    permute[i] = (i * n) % num_allocs;
	}
	for (u64 i = 0; i < num_allocs; ++i) {
	    buf4s[permute[i]] = alloc2.Alloc<u8>();
	    memset(buf4s[permute[i]].data(), n, slab_size);
	}
	for (u64 i = 0; i < num_allocs; ++i) {
	    int code = 0;
	    for (u64 j = 0; j < slab_size; ++j) {
		code |= buf4s[i][j] != n;
	    }
	    ASSERT(!code, "");
	    alloc2.Free(buf4s[i]);
	}
    }
    VirtualDecommit(buf4s, buf4s_size);
    VirtualRelease(buf4s, buf4s_size);
    SlabAllocator::Destroy(alloc2);
}
