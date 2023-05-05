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

void simple_coro(std::tuple<i32> *args) {
    LogSTDOUT("simple_coro\n");
    int x = std::get<0>(*args);
    x = x + 1;
    ASSERT(x == 43, "");
    yield();
    x = x + 1;
    ASSERT(x == 44, "");
    return;
}

int main() {
    Coroutine<i32> coro(simple_coro, 42);
    coro.init();
    coro.next();
    coro.next();
    ASSERT(coro.done, "");
}
