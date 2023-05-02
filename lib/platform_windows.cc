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

#include <windows.h>

#include "include/ring_alloc.h"
#include "include/singleton.h"
#include "include/platform.h"
#include "include/defer.h"

File File::Create(std::string_view path, FileOpenKind kind) {
}

void File::Destroy(File file) {
}

u64 File::size() const {
}

u64 File::read(void *buf, u64 count) const {
}

u64 File::write(const void *buf, u64 count) const {
}

std::pair<void *, u64> MemoryMapFile(File file, ProtectionBits protection_bits, MappingBits mapping_bits) {
}

void MemoryUnmapFile(void *mapped_ptr, u64 mapped_size) {
}

void *VirtualReserve(u64 size, void *addr, ProtectionBits protection_bits, MappingBits mapping_bits) {
}

void *VirtualCommit([[maybe_unused]] u64 size, void *addr) {
}

void VirtualRelease(void *mapped_ptr, u64 mapped_size) {
}

void VirtualDecommit([[maybe_unused]] void *mapped_ptr, [[maybe_unused]] u64 mapped_size) {}
