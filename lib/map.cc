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

#include "include/map.h"

constexpr u64 squirrel3(u64 at) {
    constexpr u64 BIT_NOISE1 = 0x9E3779B185EBCA87ULL;
    constexpr u64 BIT_NOISE2 = 0xC2B2AE3D27D4EB4FULL;
    constexpr u64 BIT_NOISE3 = 0x27D4EB2F165667C5ULL;
    at *= BIT_NOISE1;
    at ^= (at >> 8);
    at += BIT_NOISE2;
    at ^= (at << 8);
    at *= BIT_NOISE3;
    at ^= (at >> 8);
    return at;
}
