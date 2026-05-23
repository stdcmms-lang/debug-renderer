# Makefile for building common-renderer with CPU rendering support

# Compiler configuration
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wno-sign-compare -Wno-unused-parameter -DSK_DEBUG

# Platform-specific configuration
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	SDK_PATH := $(shell xcrun --show-sdk-path)
	CXXFLAGS += -isysroot $(SDK_PATH)
	PLATFORM_LIBS = -framework CoreFoundation -framework CoreGraphics -framework CoreText -framework ApplicationServices
	# macOS ld64 resolves static-archive cross-references without help.
	LINK_GROUP_START =
	LINK_GROUP_END =
else
	# Linux (Ubuntu)
	PLATFORM_LIBS = -lfontconfig -lfreetype -ldl -lm
	# GNU ld scans each .a once; wrap Skia archives so cross-archive
	# references (e.g. libskparagraph -> libskshaper, libsvg -> libskia)
	# resolve regardless of listed order.
	LINK_GROUP_START = -Wl,--start-group
	LINK_GROUP_END = -Wl,--end-group
endif

# Colors
GREEN = \033[0;32m
YELLOW = \033[1;33m
RED = \033[0;31m
NC = \033[0m # No Color

# Paths
SKIA_ROOT = third_party/skia
SKIA_LIB = $(SKIA_ROOT)/out/unittest/libskia.a
SKIA_DEPS = $(SKIA_ROOT)/out/unittest/libpng.a $(SKIA_ROOT)/out/unittest/libjpeg.a $(SKIA_ROOT)/out/unittest/libwebp.a $(SKIA_ROOT)/out/unittest/libzlib.a $(SKIA_ROOT)/out/unittest/libexpat.a $(SKIA_ROOT)/out/unittest/libwuffs.a $(SKIA_ROOT)/out/unittest/libskcms.a $(SKIA_ROOT)/out/unittest/libharfbuzz.a $(SKIA_ROOT)/out/unittest/libicu.a $(SKIA_ROOT)/out/unittest/libskshaper.a $(SKIA_ROOT)/out/unittest/libskresources.a $(SKIA_ROOT)/out/unittest/libskunicode_core.a $(SKIA_ROOT)/out/unittest/libskunicode_icu.a $(SKIA_ROOT)/out/unittest/libskparagraph.a $(SKIA_ROOT)/out/unittest/libsvg.a
OUTPUT_DIR = dist
TEST_OUTPUT = $(OUTPUT_DIR)/render_test

# Skia build configuration
SKIA_GN = $(SKIA_ROOT)/bin/gn
SKIA_NINJA = $(SKIA_ROOT)/third_party/ninja/ninja
# Paths relative to SKIA_ROOT for use after `cd $(SKIA_ROOT)`
SKIA_GN_REL = bin/gn
SKIA_NINJA_REL = third_party/ninja/ninja

# Parallelism for the Skia ninja build. Lower this (e.g. SKIA_JOBS=1) on
# memory-constrained machines if cc1plus gets OOM-killed during the build.
SKIA_JOBS ?= 2

# Include paths
INCLUDES = -Iinclude -isystem $(SKIA_ROOT) -isystem third_party/googletest/googletest -isystem third_party/googletest/googletest/include

# Source files
COMMON_SOURCES = src/common_renderer.cpp src/render_utils.cpp src/native_parser.cpp $(SKIA_ROOT)/modules/jsonreader/SkJSONReader.cpp
TEST_SOURCES = src/render_test.cpp src/render_utils_test.cpp $(COMMON_SOURCES) third_party/googletest/googletest/src/gtest-all.cc

# Default target
all: test

# Build and run tests
test: $(TEST_OUTPUT)
	@echo "$(GREEN)Running render tests...$(NC)"
	@if [ -n "$$RESULTS_FILE" ]; then \
		mkdir -p results; \
		GTEST_JSON="results/gtest.json"; \
		./$(TEST_OUTPUT) --gtest_output=json:$$GTEST_JSON; \
		EXIT=$$?; \
		python3 scripts/gtest_to_agona.py $$GTEST_JSON "$$RESULTS_FILE" $$EXIT; \
		exit $$EXIT; \
	else \
		./$(TEST_OUTPUT); \
	fi

# Build test executable
$(TEST_OUTPUT): $(TEST_SOURCES) $(SKIA_LIB) $(SKIA_DEPS)
	@mkdir -p $(OUTPUT_DIR)
	@echo "$(GREEN)Building render tests...$(NC)"
	@$(CXX) $(CXXFLAGS) $(TEST_SOURCES) -o $(TEST_OUTPUT) $(INCLUDES) -pthread $(LINK_GROUP_START) $(SKIA_LIB) $(SKIA_DEPS) $(LINK_GROUP_END) $(PLATFORM_LIBS)

# Build Skia for CPU (unittest)
# Minimum config required by the unit test suite:
#   - skparagraph + skshaper + skunicode (text rendering)
#   - SVG module (SkSVGDOM)
#   - PNG / JPEG / WebP decode + PNG / WebP encode
#   - SkRuntimeEffect (built into core libskia)
#   - fontconfig + freetype on Linux for SkFontMgr
# GPU backends, Skottie, PDF, DNG, PIEX are NOT used and disabled to cut build time.
# Skia's bundled third_party copies of libpng/jpeg/webp/harfbuzz/icu/expat are used
# (skia_use_system_* left at the default of false) so the only system libs needed
# on Ubuntu are fontconfig + freetype.
# Platform-specific font backend:
#   - macOS uses CoreText (no FreeType / fontconfig needed)
#   - Linux uses FreeType + fontconfig
ifeq ($(UNAME_S),Darwin)
SKIA_FONT_ARGS = skia_use_freetype=false skia_use_fontconfig=false
else
SKIA_FONT_ARGS = skia_use_freetype=true skia_use_fontconfig=true
endif

SKIA_GN_ARGS = is_debug=true \
	is_official_build=false \
	skia_enable_ganesh=false \
	skia_enable_graphite=false \
	skia_use_gl=false \
	skia_use_vulkan=false \
	skia_use_metal=false \
	skia_use_dawn=false \
	skia_enable_pdf=false \
	skia_enable_skottie=false \
	skia_use_dng_sdk=false \
	skia_use_piex=false \
	skia_enable_svg=true \
	skia_enable_skparagraph=true \
	skia_use_expat=true \
	skia_use_harfbuzz=true \
	skia_use_icu=true \
	$(SKIA_FONT_ARGS) \
	skia_use_libpng_decode=true \
	skia_use_libpng_encode=true \
	skia_use_libjpeg_turbo_decode=true \
	skia_use_libwebp_decode=true \
	skia_use_libwebp_encode=true

build-skia: check-skia-tools
	@echo "$(GREEN)Building Skia for CPU...$(NC)"
	@cd $(SKIA_ROOT) && \
		rm -rf out/unittest && \
		python3 tools/git-sync-deps && \
		$(SKIA_GN_REL) gen out/unittest --args='$(SKIA_GN_ARGS)'
	@cd $(SKIA_ROOT) && $(SKIA_NINJA_REL) -j$(SKIA_JOBS) -C out/unittest skia skparagraph modules/svg:svg
	@echo "$(GREEN)Skia CPU build completed successfully$(NC)"

# Setup Skia build tools
setup-skia-tools:
	@echo "$(GREEN)Setting up Skia build tools...$(NC)"
	@cd $(SKIA_ROOT) && ./bin/fetch-gn
	@cd $(SKIA_ROOT) && ./bin/fetch-ninja

# Check Skia build tools
check-skia-tools:
	@echo "$(GREEN)Checking Skia build tools...$(NC)"
	@if [ ! -f "$(SKIA_GN)" ]; then \
		echo "$(YELLOW)GN not found, setting up Skia tools...$(NC)"; \
		$(MAKE) setup-skia-tools; \
	fi
	@if [ ! -f "$(SKIA_NINJA)" ]; then \
		echo "$(YELLOW)Ninja not found, setting up Skia tools...$(NC)"; \
		$(MAKE) setup-skia-tools; \
	fi

# Clean build artifacts
clean:
	@echo "$(GREEN)Cleaning build artifacts...$(NC)"
	@rm -rf $(OUTPUT_DIR)
	@rm -f $(TEST_OUTPUT)
	@echo "$(GREEN)Clean completed$(NC)"

# Formatting target
format:
	@echo "$(GREEN)Fixing C++ code formatting...$(NC)"
	@clang-format --style=file -i $(COMMON_SOURCES) src/render_test.cpp src/render_utils_test.cpp

# Detect clang-tidy path
CLANG_TIDY := $(shell which clang-tidy)
# Prefer Homebrew's clang-tidy on macOS if available
ifneq ($(wildcard /opt/homebrew/opt/llvm/bin/clang-tidy),)
    CLANG_TIDY := /opt/homebrew/opt/llvm/bin/clang-tidy
else ifneq ($(wildcard /usr/local/bin/clang-tidy),)
    CLANG_TIDY := /usr/local/bin/clang-tidy
endif

lint:
	@echo "$(GREEN)Running clang-tidy on C++ sources...$(NC)"
	@clang-tidy -header-filter='^include/.*' -fix -fix-errors $(COMMON_SOURCES) src/render_test.cpp src/render_utils_test.cpp -- $(CXXFLAGS) $(INCLUDES)

# Show help
help:
	@echo "$(GREEN)common-renderer Build System$(NC)"
	@echo "Available targets:"
	@echo "  test             - Build and run render tests (default)"
	@echo "  build-skia       - Build Skia for CPU rendering"
	@echo "  setup-skia-tools - Setup Skia build tools (GN and Ninja)"
	@echo "  clean            - Clean build artifacts"
	@echo "  format           - Format C++ code with clang-format"
	@echo "  lint             - Run clang-tidy linting"
	@echo "  help             - Show this help message"

.PHONY: all test build-skia setup-skia-tools check-skia-tools clean format lint help
