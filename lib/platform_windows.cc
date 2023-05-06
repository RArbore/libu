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

static i32 ConvertProtectionBits(ProtectionBits protection_bits) {
    bool read = !!(protection_bits & Protection::Read);
    bool write = !!(protection_bits & Protection::Write);
    bool exec = !!(protection_bits & Protection::Exec);
    bool none = !!(protection_bits & Protection::None);
    i32 options[16] = {
	PAGE_READONLY,
	PAGE_NOACCESS,
	PAGE_READWRITE,
	PAGE_READWRITE,
	PAGE_EXECUTE_READ,
	PAGE_EXECUTE,
	PAGE_EXECUTE_READWRITE,
	PAGE_EXECUTE_READWRITE,
	PAGE_NOACCESS,
	PAGE_NOACCESS,
	PAGE_NOACCESS,
	PAGE_NOACCESS,
	PAGE_NOACCESS,
	PAGE_NOACCESS,
	PAGE_NOACCESS,
	PAGE_NOACCESS,
    };
    return options[read + write * 2 + exec * 4 + none * 8];
}

static std::pair<i32, i32> ConvertProtectionAndMappingBits(ProtectionBits protection_bits, MappingBits mapping_bits) {
    bool read = !!(protection_bits & Protection::Read);
    bool write = !!(protection_bits & Protection::Write);
    bool exec = !!(protection_bits & Protection::Exec);
    bool none = !!(protection_bits & Protection::None);
    int copy = !!(mapping_bits & Mapping::Private) ? FILE_MAP_COPY : 0;
    if (read && write && !exec && !none) {
	return {PAGE_READWRITE, FILE_MAP_ALL_ACCESS | copy};
    } else if (read && !write && !exec && !none) {
	return {PAGE_READONLY, FILE_MAP_READ | copy};
    } else if (read && write && exec && !none) {
	return {PAGE_EXECUTE_READ, FILE_MAP_ALL_ACCESS | FILE_MAP_EXECUTE | copy};
    } else if (read && !write && exec && !none) {
	return {PAGE_EXECUTE_READWRITE, FILE_MAP_READ | FILE_MAP_EXECUTE | copy};
    } else if (exec && !none) {
	return {PAGE_EXECUTE_READ, FILE_MAP_ALL_ACCESS | FILE_MAP_EXECUTE | copy};
    } else {
	PANIC("invalid set of protection bits");
    }
}

File File::Create(std::string_view path, FileOpenKind kind) {
    u64 path_size = path.size() + 1;
    auto buf = ring_alloc->Alloc<char>(path_size);
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

void File::truncate(u64 size) const {
    CheckStdInit();
    BOOL move_code = SetFilePointerEx(handle, {.QuadPart = static_cast<LONGLONG>(size)}, nullptr, FILE_BEGIN);
    ASSERT(move_code, "SetFilePointerEx failed");
    BOOL set_code = SetEndOfFile(handle);
    ASSERT(set_code, "SetEndOfFile failed");
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
	BOOL write_code = WriteFile(handle, static_cast<const char *>(buf) + i * 0xFFFFFFFF, 0xFFFFFFFF, &bytes_write, nullptr);
	ASSERT(write_code, "WriteFile failed");
	total_bytes_write += bytes_write;
    }
    BOOL write_code = WriteFile(handle, static_cast<const char *>(buf) + chunks_4gb * 0xFFFFFFFF, below_4gb, &bytes_write, nullptr);
    ASSERT(write_code, "WriteFile failed");
    total_bytes_write += bytes_write;
    return total_bytes_write;
}

std::pair<void *, u64> MemoryMapFile(File file, ProtectionBits protection_bits, MappingBits mapping_bits) {
    u64 file_size = file.size();
    u64 upper = file_size >> 32;
    u64 lower = file_size & 0xFFFFFFFF;
    auto bits = ConvertProtectionAndMappingBits(protection_bits, mapping_bits);
    HANDLE mapping = CreateFileMappingA(file.handle, nullptr, bits.first, static_cast<u32>(upper), static_cast<u32>(lower), nullptr);
    ASSERT(mapping, "CreateFileMappingA failed");
    void *view = MapViewOfFile(mapping, bits.second, 0, 0, static_cast<SIZE_T>(file_size));
    ASSERT(view, "MapViewOfFile failed");
    return {view, file_size};
}

void MemoryUnmapFile(void *mapped_ptr, u64 mapped_size) {
    BOOL flush_code = FlushViewOfFile(mapped_ptr, static_cast<SIZE_T>(mapped_size));
    ASSERT(flush_code, "FlushViewOfFile failed");
    BOOL unmap_code = UnmapViewOfFile(mapped_ptr);
    ASSERT(unmap_code, "UnmapViewOfFile failed");
}

void *VirtualReserve(u64 size, void *addr, ProtectionBits protection_bits, MappingBits mapping_bits) {
    void *reserved = VirtualAlloc(mapping_bits & Mapping::Fixed ? addr : nullptr, static_cast<SIZE_T>(size), MEM_RESERVE, ConvertProtectionBits(protection_bits));
    ASSERT(reserved, "VirtualAlloc failed");
    return reserved;
}

void *VirtualCommit(u64 size, void *addr, ProtectionBits protection_bits, MappingBits mapping_bits) {
    void *reserved = VirtualAlloc(mapping_bits & Mapping::Fixed ? addr : nullptr, static_cast<SIZE_T>(size), MEM_COMMIT, ConvertProtectionBits(protection_bits));
    ASSERT(reserved, "VirtualAlloc failed");
    return reserved;
}

void VirtualRelease(void *mapped_ptr, u64 mapped_size) {
    BOOL free_code = VirtualFree(mapped_ptr, static_cast<SIZE_T>(mapped_size), MEM_RELEASE);
    ASSERT(free_code, "VirtualFree failed");
}

void VirtualDecommit(void *mapped_ptr, u64 mapped_size) {
    BOOL free_code = VirtualFree(mapped_ptr, static_cast<SIZE_T>(mapped_size), MEM_DECOMMIT);
    ASSERT(free_code, "VirtualFree failed");
}
