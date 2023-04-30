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
    u64 reserve_size = 1 << 16;
    u64 commit_size = 1 << 8;
    char *buf = static_cast<char *>(VirtualReserve(reserve_size));
    for (u64 i = 0; i < reserve_size; i += commit_size) {
	u64 *local_buf = static_cast<u64 *>(VirtualCommit(commit_size, buf + i));
	for (u64 j = 0; j < commit_size / sizeof(u64); ++j) {
	    local_buf[j] = j;
	}
	for (u64 j = 0; j < commit_size / sizeof(u64); ++j) {
	    ASSERT(local_buf[j] == j, "");
	}
	VirtualDecommit(buf + i, commit_size);
    }
    VirtualRelease(buf, reserve_size);
}
