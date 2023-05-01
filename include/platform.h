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

#pragma once

#include <stdlib.h>
#include <utility>

#include "include/primitive_types.h"
#include "include/log.h"

#define PANIC(str) { LogPanic(str, __LINE__, __FUNCTION__, __FILE__); abort(); }
#define ASSERT(c, str) if (!(c)) PANIC(str)

struct File {
#ifdef __linux__
    i32 fd;
#endif

    static File Create(std::string_view path);

    u64 size() const;
    u64 read(void *buf, u64 count) const;
    u64 write(const void *buf, u64 count) const;
};

extern const File STDIN;
extern const File STDOUT;
extern const File STDERR;

using ProtectionBits = u8;

enum Protection {
    Read = 0x1,
    Write = 0x2,
    Exec = 0x4,
    None = 0x8,
};

using MappingBits = u8;

enum Mapping {
    Shared = 0x1,
    Private = 0x2,
    Fixed = 0x4,
    Anonymous = 0x8,
};

std::pair<void *, u64> MemoryMapFile(File file, ProtectionBits protection_bits, MappingBits mapping_bits);
void MemoryUnmapFile(void *mapped_ptr, u64 mapped_size);

void *VirtualReserve(u64 size, void *addr = nullptr);
void *VirtualCommit(u64 size, void *addr);
void VirtualRelease(void *addr, u64 size);
void VirtualDecommit(void *addr, u64 size);
