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
    RingAllocator alloc = RingAllocator::Create(4096);
    auto buf1 = alloc.alloc<u8>(26);
    auto buf1_raw = &*buf1;
    for (u8 i = 0; i < 26; ++i) {
	buf1[i] = i + 'A';
    }
    alloc.alloc<u8>(4000);
    for (u8 i = 0; i < 26; ++i) {
	ASSERT(buf1[i] == i + 'A', "");
    }
    auto buf2 = alloc.alloc<u8>(4000);
    for (u16 i = 0; i < 4000; ++i) {
	buf2[i] = 0;
    }
    for (u8 i = 0; i < 26; ++i) {
	ASSERT(buf1_raw[i] == 0, "");
    }
    RingAllocator::Destroy(alloc);
}
