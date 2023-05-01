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
    SlabAllocator alloc = SlabAllocator::Create(4096, 256, 256);
    auto buf1 = alloc.alloc<u8>();
    for (u16 i = 0; i < 256; ++i) {
	buf1[i] = static_cast<u8>('A' + (i % 10));
    }
    for (u16 i = 0; i < 256; ++i) {
	ASSERT(buf1[i] == static_cast<u8>('A' + (i % 10)), "");
    }
    auto buf2 = alloc.alloc<u8>();
    for (u16 i = 0; i < 256; ++i) {
	buf2[i] = static_cast<u8>('a' + (i % 10));
    }
    for (u16 i = 0; i < 256; ++i) {
	ASSERT(buf2[i] == static_cast<u8>('a' + (i % 10)), "");
    }
    for (u16 i = 0; i < 256; ++i) {
	ASSERT(buf1[i] == static_cast<u8>('A' + (i % 10)), "");
    }
    alloc.free(buf1);
    auto buf3 = alloc.alloc<u8>();
    for (u16 i = 0; i < 256; ++i) {
	buf3[i] = static_cast<u8>('a' + (i % 10));
    }
    for (u16 i = 0; i < 256; ++i) {
	ASSERT(buf3[i] == static_cast<u8>('a' + (i % 10)), "");
    }
    for (u16 i = 0; i < 256; ++i) {
	ASSERT(buf1[i] == static_cast<u8>('a' + (i % 10)), "");
    }
    SlabAllocator::Destroy(alloc);
}
