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

i32 main() {
    BumpAllocator alloc = BumpAllocator::Create(4096, 4096);
    auto buf1 = alloc.Alloc<u8>(26);
    auto buf1_raw = &*buf1;
    for (u8 i = 0; i < 26; ++i) {
	buf1[i] = i + 'A';
    }
    alloc.Alloc<u8>(4000);
    for (u8 i = 0; i < 26; ++i) {
	ASSERT(buf1[i] == i + 'A', "");
    }
    alloc.FreeAll();
    auto buf2 = alloc.Alloc<u8>(26);
    for (u8 i = 0; i < 26; ++i) {
	buf2[i] = i + 'a';
    }
    for (u8 i = 0; i < 26; ++i) {
	ASSERT(buf2[i] == i + 'a', "");
    }
    for (u8 i = 0; i < 26; ++i) {
	ASSERT(buf1_raw[i] == i + 'a', "");
    }
    BumpAllocator::Destroy(alloc);
}
