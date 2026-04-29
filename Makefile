# Project names
NAME			:= smc
# Version (Tag + commit-counter, excluding hash, including dirty-flag)
GIT_VER 		:= $(shell git describe --tags --always --dirty 2>/dev/null | sed 's/-g[0-9a-f]\{7,40\}//')
VERSION 		:= $(if $(GIT_VER),$(GIT_VER),V0.0.0-unknown)
INSTALL_PATH	:= /usr/local/bin

# Compiler & flags
CXX			:= clang++
MIN_OS      := -mmacosx-version-min=14.6
ARCHS       := -arch arm64 -arch x86_64
CLFLAGS		:= -O3 -flto -fno-exceptions -std=c++20 $(MIN_OS) $(ARCHS)
CXXFLAGS	:= $(CLFLAGS) -Wall -Werror -DNDEBUG -DAPP_VERSION=\"$(VERSION)\"
LDFLAGS 	:= $(CLFLAGS) -framework IOKit

# Folders
SRC_DIR		:= src
BUILD_DIR	:= build

# Find sources: .cpp
SRCS := $(wildcard $(SRC_DIR)/*.cpp)
# Generate object files: .o
OBJS := $(patsubst $(SRC_DIR)/%, $(BUILD_DIR)/%.o, $(SRCS))
TARGET := $(BUILD_DIR)/$(NAME)

# Standard target
all: $(TARGET)

# Link objects
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $(TARGET)

# Compile sources
$(BUILD_DIR)/%.cpp.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

install: all
	sudo install -d $(INSTALL_PATH)
	sudo install -m 755 $(TARGET) $(INSTALL_PATH)/$(NAME)
	sudo strip $(INSTALL_PATH)/$(NAME)

clean:
	rm -rf $(BUILD_DIR)

uninstall: clean
	sudo rm -f $(INSTALL_PATH)/$(NAME)

.PHONY: all install clean uninstall
