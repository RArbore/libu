# 
# This file is part of libu.
# libu is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
# libu is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with libu. If not, see <https://www.gnu.org/licenses/>.
# 

OS ?= LINUX

ifeq ($(OS), WINDOWS)
	CXX := i686-w64-mingw32-g++
	AR := i686-w64-mingw32-ar
	CXXFLAGS := $(CXXFLAGS) -isystem /usr/x86_64-w64-mingw32/include -static -static-libgcc -static-libstdc++
	PLATFORM_SRC := lib/platform_windows.cc
else
	CXX := g++
	AR := ar
	PLATFORM_SRC := lib/platform_linux.cc
endif

RELEASE ?= 0

ifeq ($(RELEASE), 1)
	CXXFLAGS := $(CXXFLAGS) -Ofast -march=native -flto -DRELEASE
	LDFLAGS := $(LDFLAGS)
else
	CXXFLAGS := $(CXXFLAGS) -g3 #-fsanitize=address -fsanitize=undefined
endif

CXXFLAGS := $(CXXFLAGS) -fno-rtti -fno-exceptions -pipe -std=c++20 -I./ -Iinclude/
WFLAGS := $(WFLAGS) -Wall -Wextra -Wshadow -Wconversion -Wpedantic

BUILD_DIR := build/$(OS)
SRCS := $(shell find lib -name "*.cc" | grep -v "platform")
HEADERS := $(shell find include -name "*.h")
OBJS := $(subst lib/, $(BUILD_DIR)/, $(patsubst %.cc, %.o, $(SRCS)))
TEST_SRCS := $(shell find tests -name "*.cc")
TEST_EXES := $(subst tests/, $(BUILD_DIR)/, $(patsubst %.cc, %.cc.out, $(TEST_SRCS)))
PLATFORM_OBJ := $(subst lib/, $(BUILD_DIR)/, $(patsubst %.cc, %.o, $(PLATFORM_SRC)))
LIB := $(BUILD_DIR)/libu.a

$(LIB): $(OBJS) $(PLATFORM_OBJ)
	$(AR) -crs $@ $^

$(OBJS): $(BUILD_DIR)/%.o: lib/%.cc $(HEADERS) $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(WFLAGS) -c $< -o $@

$(PLATFORM_OBJ): $(PLATFORM_SRC) $(HEADERS) $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(WFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

test: $(BUILD_DIR) $(LIB)
	python3 tests/test_driver.py $(OS) $(CXX) $(CXXFLAGS) $(WFLAGS)

build_tests: $(LIB) $(TEST_EXES)

$(TEST_EXES): $(BUILD_DIR)/%.cc.out: tests/%.cc $(HEADERS) $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(WFLAGS) $< -o $@ -lu -L$(BUILD_DIR)

clean:
	rm -rf build

.PHONY: test clean
