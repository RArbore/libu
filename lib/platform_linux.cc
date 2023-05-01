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

#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

#include "include/platform.h"
#include "include/defer.h"

const File STDIN = {0};
const File STDOUT = {1};
const File STDERR = {2};

static i32 ConvertProtectionBits(ProtectionBits protection_bits) {
    return
	!!(protection_bits & Protection::Read) * PROT_READ |
	!!(protection_bits & Protection::Write) * PROT_WRITE |
	!!(protection_bits & Protection::Exec) * PROT_EXEC |
	!!(protection_bits & Protection::None) * PROT_NONE;
}

static i32 ConvertMappingBits(MappingBits mapping_bits) {
    return
	!!(mapping_bits & Mapping::Shared) * MAP_SHARED |
	!!(mapping_bits & Mapping::Private) * MAP_PRIVATE |
	!!(mapping_bits & Mapping::Fixed) * MAP_FIXED |
	!!(mapping_bits & Mapping::Anonymous) * MAP_ANONYMOUS;
}

File File::Create(std::string_view path) {
    int fd = -1;
    return {fd};
}

u64 File::size() const {
    struct stat stat;
    int fstat_code = fstat(fd, &stat);
    ASSERT(fstat_code, "fstat failed");
    return stat.st_size;
}

u64 File::read(void *buf, u64 count) const {
    i64 read_code = ::read(fd, buf, count);
    ASSERT(read_code >= 0, "read failed");
    return read_code;
}

u64 File::write(const void *buf, u64 count) const {
    i64 write_code = ::write(fd, buf, count);
    ASSERT(write_code >= 0, "write failed");
    return write_code;
}

std::pair<void *, u64> MemoryMapFile(File file, ProtectionBits protection_bits, MappingBits mapping_bits) {
    u64 file_size = file.size();
    void *mapped_file_ptr = mmap(NULL, file_size, ConvertProtectionBits(protection_bits), ConvertMappingBits(mapping_bits), file.fd, 0);
    ASSERT(mapped_file_ptr != MAP_FAILED, "mmap failed");
    return {mapped_file_ptr, file_size};
}

void MemoryUnmapFile(void *mapped_ptr, u64 mapped_size) {
    int munmap_code = munmap(mapped_ptr, mapped_size);
    ASSERT(!munmap_code, "munmap failed");
}

void *VirtualReserve(u64 size, void *addr) {
    void *mapped_ptr = mmap(addr, size, ConvertProtectionBits(Protection::Read | Protection::Write), ConvertMappingBits(Mapping::Private | Mapping::Anonymous), -1, 0);
    ASSERT(mapped_ptr != MAP_FAILED, "mmap failed");
    return mapped_ptr;
}

void *VirtualCommit([[maybe_unused]] u64 size, void *addr) {
    return addr;
}

void VirtualRelease(void *mapped_ptr, u64 mapped_size) {
    int munmap_code = munmap(mapped_ptr, mapped_size);
    ASSERT(!munmap_code, "munmap failed");
}

void VirtualDecommit([[maybe_unused]] void *mapped_ptr, [[maybe_unused]] u64 mapped_size) {}
