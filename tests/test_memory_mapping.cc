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
    u64 reserve1_size = 0xFFFFFF;
    u64 commit1_size = 0xFFF;
    u8 *buf1 = static_cast<u8 *>(VirtualReserve(reserve1_size));
    for (u64 i = 0; i < reserve1_size; i += commit1_size) {
	u64 *local_buf1 = static_cast<u64 *>(VirtualCommit(commit1_size, buf1 + i));
	for (u64 j = 0; j < commit1_size / sizeof(u64); ++j) {
	    local_buf1[j] = j;
	}
	for (u64 j = 0; j < commit1_size / sizeof(u64); ++j) {
	    ASSERT(local_buf1[j] == j, "");
	}
	VirtualDecommit(buf1 + i, commit1_size);
    }
    VirtualRelease(buf1, reserve1_size);

    u64 reserve2_size = 0xFFFFFFF;
    u8 *buf2 = static_cast<u8 *>(VirtualReserve(reserve2_size));
    VirtualCommit(reserve2_size, buf2);
    for (u64 j = 0; j < reserve2_size; ++j) {
	buf2[j] = static_cast<u8>(j);
    }
    for (u64 j = 0; j < reserve2_size; ++j) {
	ASSERT(buf2[j] == static_cast<u8>(j), "");
    }
    VirtualDecommit(buf2, reserve2_size);
    VirtualRelease(buf2, reserve2_size);
}
