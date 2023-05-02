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

File STDIN = {nullptr};
File STDOUT = {nullptr};
File STDERR = {nullptr};

static thread_local Singleton<RingAllocator, u64> ring_alloc(4096);

static void CheckStdInit() {
    static bool initialized = false;
    if (!initialized) {
	STDIN.handle = GetStdHandle(STD_INPUT_HANDLE);
	STDOUT.handle = GetStdHandle(STD_OUTPUT_HANDLE);
	STDERR.handle = GetStdHandle(STD_ERROR_HANDLE);
    }
}

static std::pair<i32, i32> ConvertFileOpenKind(FileOpenKind kind) {
    return
	{!!(kind & FileOpenKind::ReadOnly) * (GENERIC_READ) |
	 !!(kind & FileOpenKind::ReadWrite) * (GENERIC_READ | GENERIC_WRITE),
	 !!(kind & FileOpenKind::ReadOnly) * (OPEN_EXISTING) |
	 !!(kind & FileOpenKind::ReadWrite) * (CREATE_ALWAYS)};
}

/*static i32 ConvertProtectionBits(ProtectionBits protection_bits) {
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
	}*/

File File::Create(std::string_view path, FileOpenKind kind) {
    u64 path_size = path.size() + 1;
    auto buf = ring_alloc->alloc<char>(path_size);
    memcpy(buf.data(), path.data(), path.size());
    buf[path_size] = '\0';
    auto bits = ConvertFileOpenKind(kind);
    HANDLE handle = CreateFileA(buf.data(), bits.first, 0, nullptr, bits.second, FILE_ATTRIBUTE_NORMAL, nullptr);
    ASSERT(handle != INVALID_HANDLE_VALUE, "CreateFileA failed");
    return {handle};
}

void File::Destroy(File file) {
    BOOL close_code = CloseHandle(file.handle);
    ASSERT(close_code, "CloseHandle failed");
}

u64 File::size() const {
    CheckStdInit();
    LARGE_INTEGER size;
    BOOL size_code = GetFileSizeEx(handle, &size);
    ASSERT(size_code, "GetFileSizeEx failed");
    return static_cast<u64>(size.QuadPart);
}

u64 File::read(void *buf, u64 count) const {
    CheckStdInit();
    u64 chunks_4gb = count / 0xFFFFFFFF;
    u32 below_4gb = static_cast<u32>(count % 0xFFFFFFFF);
    u64 total_bytes_read = 0;
    long unsigned int bytes_read = 0;
    for (u64 i = 0; i < chunks_4gb; ++i) {
	BOOL read_code = ReadFile(handle, static_cast<char *>(buf) + i * 0xFFFFFFFF, 0xFFFFFFFF, &bytes_read, nullptr);
	ASSERT(read_code, "ReadFile failed");
	total_bytes_read += bytes_read;
    }
    BOOL read_code = ReadFile(handle, static_cast<char *>(buf) + chunks_4gb * 0xFFFFFFFF, below_4gb, &bytes_read, nullptr);
    ASSERT(read_code, "ReadFile failed");
    total_bytes_read += bytes_read;
    return total_bytes_read;
}

u64 File::write(const void *buf, u64 count) const {
    CheckStdInit();
    u64 chunks_4gb = count / 0xFFFFFFFF;
    u32 below_4gb = static_cast<u32>(count % 0xFFFFFFFF);
    u64 total_bytes_write = 0;
    long unsigned int bytes_write = 0;
    for (u64 i = 0; i < chunks_4gb; ++i) {
	BOOL write_code = WriteFile(handle, static_cast<char *>(buf) + i * 0xFFFFFFFF, 0xFFFFFFFF, &bytes_write, nullptr);
	ASSERT(write_code, "WriteFile failed");
	total_bytes_write += bytes_write;
    }
    BOOL write_code = WriteFile(handle, static_cast<char *>(buf) + chunks_4gb * 0xFFFFFFFF, below_4gb, &bytes_write, nullptr);
    ASSERT(write_code, "WriteFile failed");
    total_bytes_write += bytes_write;
    return total_bytes_write;
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
