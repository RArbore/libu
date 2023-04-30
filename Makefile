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

CXX := g++

RELEASE ?= 0

ifeq ($(RELEASE), 1)
	CXXFLAGS := $(CXXFLAGS) -Ofast -march=native -DRELEASE
	LDFLAGS := $(LDFLAGS)
else
	CXXFLAGS := $(CXXFLAGS) -g
	GLSLFLAGS := $(GLSLFLAGS) -g
endif

CXXFLAGS := $(CXXFLAGS) -fno-rtti -pipe -std=c++20 -I./ -Iinclude/
WFLAGS := $(WFLAGS) -Wall -Wextra -Wshadow -Wconversion -Wpedantic

BUILD_DIR := build
SRCS := $(shell find lib -name "*.cc")
HEADERS := $(shell find include -name "*.h")
OBJS := $(subst lib/, $(BUILD_DIR)/, $(patsubst %.cc, %.o, $(SRCS)))

$(BUILD_DIR)/libu.a: $(OBJS)
	ar -crs $@ $(OBJS)

$(OBJS): $(BUILD_DIR)/%.o: lib/%.cc $(HEADERS) $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(WFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf build

.PHONY: clean
