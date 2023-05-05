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

#include <tuple>

#include <stdlib.h>
#include <stdio.h>

#include "libu.h"

void simple_coro1(i32 arg) {
    int x = arg + 1;
    ASSERT(x == 43, "");
    yield();
    x = x + 1;
    ASSERT(x == 44, "");
    return;
}

void simple_coro2(i32 x, i32 y) {
    int fac[8];
    fac[0] = 1;
    for (int i = 1; i < 8; ++i) {
	fac[i] = fac[i - 1] * (i + 1);
	ASSERT(fac[i] == fac[i - 1] * (i + 1), "");
	yield();
    }
    int sum = x + y;
    for (int i = 0; i < 8; ++i) {
	printf("%d\n", fac[i]);
	sum += fac[i];
	yield();
    }
    printf("%d %d %d\n", sum, x, y);
    ASSERT(sum == x + y + 46233, "");
    return;
}

int main() {
    Coroutine coro1(simple_coro1, 42);
    coro1.init();
    coro1.next();
    coro1.next();
    ASSERT(coro1.done, "");

    Coroutine coro2(simple_coro2, 102, 489);
    coro2.init();
    for (int i = 0; i < 16; ++i) {
	coro2.next();
    }
    ASSERT(coro2.done, "");
}
