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

#include "libu.h"

i32 main() {
    File file1 = File::Create("build/test_file_a", FileOpenKind::ReadWrite);
    char msg1[] = "This is some test text.";
    u64 bytes_write = file1.Write(msg1, sizeof(msg1));
    ASSERT(bytes_write == sizeof(msg1), "");
    ASSERT(file1.Size() == sizeof(msg1), "");
    File::Destroy(file1);

    File file2 = File::Create("build/test_file_a", FileOpenKind::ReadOnly);
    char msg2[sizeof(msg1)];
    u64 bytes_read = file2.Read(msg2, sizeof(msg2));
    ASSERT(bytes_read == sizeof(msg2), "");
    ASSERT(!strcmp(msg1, msg2), "");
    File::Destroy(file2);

    File file3 = File::Create("build/test_file_b", FileOpenKind::ReadWrite);
    file3.Truncate(sizeof(msg1));
    auto [ptr1, msize1] = MemoryMapFile(file3);
    ASSERT(msize1 == sizeof(msg1), "");
    ASSERT(file3.Size() == sizeof(msg1), "");
    memcpy(ptr1, msg1, sizeof(msg1));
    MemoryUnmapFile(ptr1, msize1);
    File::Destroy(file3);

    File file4 = File::Create("build/test_file_b", FileOpenKind::ReadOnly);
    auto [ptr2, msize2] = MemoryMapFile(file3, Protection::Read);
    ASSERT(msize2 == sizeof(msg1), "");
    ASSERT(file4.Size() == sizeof(msg1), "");
    ASSERT(!strcmp(static_cast<char *>(ptr2), msg1), "");
    MemoryUnmapFile(ptr2, msize2);
    File::Destroy(file4);
}
