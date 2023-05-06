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
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "include/ring_alloc.h"
#include "include/singleton.h"
#include "include/platform.h"
#include "include/defer.h"

File STDIN = {0};
File STDOUT = {1};
File STDERR = {2};

static thread_local Singleton<RingAllocator, u64> ring_alloc(4096);

static i32 ConvertFileOpenKind(FileOpenKind kind) {
    return
	!!(kind & FileOpenKind::ReadOnly) * (O_RDONLY) |
	!!(kind & FileOpenKind::ReadWrite) * (O_CREAT | O_RDWR | O_TRUNC);
}

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

File File::Create(std::string_view path, FileOpenKind kind) {
    u64 path_size = path.size() + 1;
    auto buf = ring_alloc->Alloc<char>(path_size);
    memcpy(buf.data(), path.data(), path.size());
    buf[path_size] = '\0';
    i32 fd = open(buf.data(), ConvertFileOpenKind(kind), 0644);
    ASSERT(fd >= 0, "open failed");
    return {fd};
}

void File::Destroy(File file) {
    i32 close_code = close(file.fd);
    ASSERT(!close_code, "close failed");
}

u64 File::size() const {
    struct stat stat;
    i32 fstat_code = fstat(fd, &stat);
    ASSERT(!fstat_code, "fstat failed");
    return stat.st_size;
}

void File::truncate(u64 size) const {
    i32 truncate_code = ftruncate(fd, size);
    ASSERT(!truncate_code, "ftruncate failed");
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
    i32 munmap_code = munmap(mapped_ptr, mapped_size);
    ASSERT(!munmap_code, "munmap failed");
}

void *VirtualReserve(u64 size, void *addr, ProtectionBits protection_bits, MappingBits mapping_bits) {
    void *mapped_ptr = mmap(addr, size, ConvertProtectionBits(protection_bits), ConvertMappingBits(mapping_bits), -1, 0);
    ASSERT(mapped_ptr != MAP_FAILED, "mmap failed");
    return mapped_ptr;
}

void *VirtualCommit([[maybe_unused]] u64 size, void *addr, [[maybe_unused]] ProtectionBits protection_bits, [[maybe_unused]] MappingBits mapping_bits) {
    return addr;
}

void VirtualRelease(void *mapped_ptr, u64 mapped_size) {
    i32 munmap_code = munmap(mapped_ptr, mapped_size);
    ASSERT(!munmap_code, "munmap failed");
}

void VirtualDecommit([[maybe_unused]] void *mapped_ptr, [[maybe_unused]] u64 mapped_size) {}
