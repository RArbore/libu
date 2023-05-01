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

#include "include/primitive_types.h"
#include "include/ring_alloc.h"
#include "include/singleton.h"
#include "include/platform.h"
#include "include/log.h"

static thread_local Singleton<RingAllocator, u64> ring_alloc(4096);

static void Print(char *&buf, std::string_view msg) {
    memcpy(buf, msg.data(), msg.size());
    buf += msg.size();
}

static void Print(char *&buf, u64 fixed) {
    char local_buf[64];
    char *local_cursor = local_buf + 63;
    while (fixed) {
	*local_cursor = static_cast<char>(fixed % 10) + '0';
	fixed /= 10;
	--local_cursor;
    }
    ++local_cursor;
    u8 cpy_size = static_cast<u8>(local_buf + 63 - local_cursor + 1);
    memcpy(buf, local_cursor, cpy_size);
    buf += cpy_size;
}

void LogPanic(std::string_view message, u64 line, std::string_view function, std::string_view file) {
    char *orig_buf = ring_alloc->alloc<char>(4096).data();
    char *buf = orig_buf;
    Print(buf, "PANIC: \"");
    Print(buf, message);
    Print(buf, "\" occurred at line ");
    Print(buf, line);
    Print(buf, " in function ");
    Print(buf, function);
    Print(buf, " in file ");
    Print(buf, file);
    Print(buf, ".\n");
    STDERR.write(orig_buf, buf - orig_buf);
}
