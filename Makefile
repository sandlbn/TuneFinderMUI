# Makefile for TuneFinderMUI
OS := $(shell uname)
CPU_FLAGS =

# Directories
LIBDIR = /opt/amiga/m68k-amigaos/lib
SDKDIR = /opt/amiga/m68k-amigaos/sys-include
NDKDIR = /opt/amiga/m68k-amigaos/ndk-include
INCDIR = /opt/amiga/m68k-amigaos/include
SRCDIR = src
BUILDDIR = build/os3/obj
OUTDIR = out
AMINET_NAME = TuneFinderMUI
ASSETS_DIR = assets
AMINET_DIR = $(ASSETS_DIR)/aminet
ICON_DIR = $(ASSETS_DIR)/icon
RELEASE_DIR = release

# Program settings
CC = m68k-amigaos-gcc
PROGRAM_NAME = TuneFinderMUI

# Source files organized by component
SOURCES = \
	$(SRCDIR)/main.c \
	$(SRCDIR)/locale.c \
	$(SRCDIR)/network.c \
	$(SRCDIR)/app.c 

# Generate object files list
OBJECTS = $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SOURCES))

# Compiler flags
BASE_CCFLAGS = -MP -MMD -Wextra -Wno-unused-function \
    -Wno-discarded-qualifiers -Wno-int-conversion \
    -Wno-volatile-register-var -fno-lto -noixemul \
    -fbaserel $(CPU_FLAGS) \
    -I$(INCDIR) -I$(SDKDIR) -I$(NDKDIR) -Iinclude

# Additional libraries
LIBS = -lamiga -lmui -lm -ljson-c

ifdef DEBUG
CCFLAGS = $(BASE_CCFLAGS) -DDEBUG_BUILD -O0 -g
BUILD_TYPE = debug
else
CCFLAGS = $(BASE_CCFLAGS) -O2 -fomit-frame-pointer
BUILD_TYPE = release
endif

LDFLAGS = -Wl,-Map=$(OUTDIR)/$(PROGRAM_NAME).map,-L$(LIBDIR) $(LIBS)

# Directory structure for build
BUILD_DIRS = \
    $(BUILDDIR)/


# Targets
.PHONY: all clean debug release dirs

all: release

debug:
	@echo "Building debug version..."
	$(MAKE) dirs
	$(MAKE) $(OUTDIR)/$(PROGRAM_NAME) DEBUG=1

release:
	@echo "Building release version..."
	$(MAKE) dirs
	$(MAKE) $(OUTDIR)/$(PROGRAM_NAME)

dirs:
	@mkdir -p $(BUILD_DIRS)
	@mkdir -p $(OUTDIR)

$(OUTDIR)/$(PROGRAM_NAME): $(OBJECTS)
	$(info Linking $(PROGRAM_NAME) ($(BUILD_TYPE) build))
	$(CC) $(CCFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(info Compiling $< ($(BUILD_TYPE) build))
	$(CC) $(CCFLAGS) -c -o $@ $<

clean:
	$(info Cleaning...)
	@$(RM) -rf $(BUILDDIR)
	@$(RM) -f $(OUTDIR)/$(PROGRAM_NAME)
	@$(RM) -f $(OUTDIR)/$(PROGRAM_NAME).map

-include $(OBJECTS:.o=.d)