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

#include <string.h>

#include "include/primitive_types.h"
#include "include/platform.h"

template<typename K, typename V>
struct Map {
    void *backing_buf;
    u64 slots;

    struct MapEntry {
	K k;
	V v;
    };

    static u64 AllocSize(u64 slots) {
	return slots * sizeof(MapEntry);
    }
    
    static Map Create(u64 slots, void *backing_buf) {
	memset(backing_buf, 0, AllocSize(slots));
	return {backing_buf, slots};
    }
    
    static void Destroy(Map map) {
	
    }
};
