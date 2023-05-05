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

#include "libu.h"

int main() {
    SlabAllocator alloc1 = SlabAllocator::Create(4096, 256, 256);
    auto buf1 = alloc1.alloc<u8>();
    for (u16 i = 0; i < 256; ++i) {
	buf1[i] = static_cast<u8>('A' + (i % 10));
    }
    for (u16 i = 0; i < 256; ++i) {
	ASSERT(buf1[i] == static_cast<u8>('A' + (i % 10)), "");
    }

    auto buf2 = alloc1.alloc<u8>();
    for (u16 i = 0; i < 256; ++i) {
	buf2[i] = static_cast<u8>('a' + (i % 10));
    }
    for (u16 i = 0; i < 256; ++i) {
	ASSERT(buf2[i] == static_cast<u8>('a' + (i % 10)), "");
    }
    for (u16 i = 0; i < 256; ++i) {
	ASSERT(buf1[i] == static_cast<u8>('A' + (i % 10)), "");
    }

    alloc1.free(buf1);
    auto buf3 = alloc1.alloc<u8>();
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

    SlabAllocator alloc2 = SlabAllocator::Create(1 << 20, 1 << 12, 1 << 8);
    u64 buf4s_size = (1 << 8) * sizeof(SlabAllocator::Pointer<u8>);
    SlabAllocator::Pointer<u8> *buf4s = reinterpret_cast<SlabAllocator::Pointer<u8> *>(VirtualReserve(buf4s_size));
    VirtualCommit(buf4s_size, buf4s);
    
    VirtualDecommit(buf4s, buf4s_size);
    VirtualRelease(buf4s, buf4s_size);
    SlabAllocator::Destroy(alloc2);
}
