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
    Yield();
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
	Yield();
    }
    int sum = x + y;
    for (int i = 0; i < 8; ++i) {
	sum += fac[i];
	Yield();
    }
    ASSERT(sum == x + y + 46233, "");
    return;
}

i32 simple_coro3() {
    i32 a = 0;
    i32 b = 1;
    while (true) {
	int c = a + b;
	a = b;
	b = c;
	Yield(c);
    }
    return -1;
}

i32 recursive_coro(i32 x) {
    if (x == 1) {
	return 1;
    }
    
    Coroutine coro1(recursive_coro);
    coro1.Create(x - 1);
    i32 a = coro1.Next();
    ASSERT(coro1.done, "");

    Coroutine coro2(recursive_coro);
    coro2.Create(x - 1);
    i32 b = coro2.Next();
    ASSERT(coro2.done, "");

    return a + b;
}

i32 main() {
    Coroutine coro1(simple_coro1);
    coro1.Create(42);
    coro1.Next();
    coro1.Next();
    coro1.Destroy();
    ASSERT(coro1.done, "");

    Coroutine coro2(simple_coro2);
    coro2.Create(102, 489);
    for (int i = 0; i < 16; ++i) {
	coro2.Next();
    }
    coro2.Destroy();
    ASSERT(coro2.done, "");

    Coroutine coro3(simple_coro3);
    coro3.Create();
    i32 a = 0;
    i32 b = 1;
    for (int i = 0; i < 30; ++i) {
	int c = a + b;
	a = b;
	b = c;
	int f = coro3.Next();
	ASSERT(f == c, "");
    }
    coro3.Destroy();

    Coroutine coro4(recursive_coro);
    coro4.Create(6);
    int total = coro4.Next();
    ASSERT(total == 32, "");
    coro4.Destroy();
}
