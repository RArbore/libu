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

template<typename T, typename... Args>
struct Singleton {
    T t;
    Singleton(Args... args): t(T::Create(args...)) {};
    ~Singleton() {
	T::Destroy(t);
    }
    operator T&() {
	return t;
    }
    T *operator-> () {
	return &t;
    }
};
