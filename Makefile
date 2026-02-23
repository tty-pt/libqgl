BE := glfw

INPUT-fb := dev
INPUT-glfw := glfw

LDLIBS-Linux += -lGL -lX11 -lGLEW -lglfw
LDLIBS-OpenBSD += -lGL -lGLU -lX11 -lGLEW -lglfw
LDLIBS-Windows += -lglew32 -lglfw3 -lopengl32 -lgdi32 -luser32 -lkernel32
LDLIBS-Darwin += -lglfw

LDFLAGS-Darwin += -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

add-prefix-OpenBSD += /usr/X11R6

LDLIBS := -lm -lxxhash -lqmap -lqsys -lpng
LDLIBS += ${LDLIBS-${BE}}

LDLIBS-Linux += -lEGL

obj-y := glfw img png
obj-y += tile font
obj-y += ui ui-style ui-cache shadow
obj-y += input input-glfw
libqgl-obj-y := ${obj-y:%=src/%.o}
posix := fb input-dev
libqgl-obj-y-Linux := ${posix:%=src/%.o}
# libqgl-obj-y-OpenBSD := ${posix:%=src/%.o}

CFLAGS := -g
all := libqgl

-include ../mk/include.mk

# Test configuration
TEST_DIR := tests
TEST_CFLAGS := -I${PWD}/include
TEST_LDFLAGS := -L${PWD}/lib
TEST_LDLIBS := -lqgl ${LDLIBS}

# Test binaries
TEST_BINS := ${TEST_DIR}/test_core${EXE} \
	${TEST_DIR}/test_textures${EXE} \
	${TEST_DIR}/test_fonts${EXE} \
	${TEST_DIR}/test_tilemaps${EXE} \
	${TEST_DIR}/test_ui_layout${EXE} \
	${TEST_DIR}/test_ui_style${EXE} \
	${TEST_DIR}/test_ui_render${EXE} \
	${TEST_DIR}/test_integration${EXE}

${TEST_DIR}:
	@mkdir -p ${TEST_DIR}/fixtures 2>/dev/null || true

${TEST_DIR}/test_core${EXE}: ${TEST_DIR} ${TEST_DIR}/test_core.c lib/libqgl.${SO}
	${cc} -o $@ ${TEST_DIR}/test_core.c ${CFLAGS} ${TEST_CFLAGS} \
		${LDFLAGS} ${TEST_LDFLAGS} ${TEST_LDLIBS}

${TEST_DIR}/test_textures${EXE}: ${TEST_DIR} ${TEST_DIR}/test_textures.c lib/libqgl.${SO}
	${cc} -o $@ ${TEST_DIR}/test_textures.c ${CFLAGS} ${TEST_CFLAGS} \
		${LDFLAGS} ${TEST_LDFLAGS} ${TEST_LDLIBS}

${TEST_DIR}/test_fonts${EXE}: ${TEST_DIR} ${TEST_DIR}/test_fonts.c lib/libqgl.${SO}
	${cc} -o $@ ${TEST_DIR}/test_fonts.c ${CFLAGS} ${TEST_CFLAGS} \
		${LDFLAGS} ${TEST_LDFLAGS} ${TEST_LDLIBS}

${TEST_DIR}/test_tilemaps${EXE}: ${TEST_DIR} ${TEST_DIR}/test_tilemaps.c lib/libqgl.${SO}
	${cc} -o $@ ${TEST_DIR}/test_tilemaps.c ${CFLAGS} ${TEST_CFLAGS} \
		${LDFLAGS} ${TEST_LDFLAGS} ${TEST_LDLIBS}

${TEST_DIR}/test_ui_layout${EXE}: ${TEST_DIR} ${TEST_DIR}/test_ui_layout.c lib/libqgl.${SO}
	${cc} -o $@ ${TEST_DIR}/test_ui_layout.c ${CFLAGS} ${TEST_CFLAGS} \
		${LDFLAGS} ${TEST_LDFLAGS} ${TEST_LDLIBS}

${TEST_DIR}/test_ui_style${EXE}: ${TEST_DIR} ${TEST_DIR}/test_ui_style.c lib/libqgl.${SO}
	${cc} -o $@ ${TEST_DIR}/test_ui_style.c ${CFLAGS} ${TEST_CFLAGS} \
		${LDFLAGS} ${TEST_LDFLAGS} ${TEST_LDLIBS}

${TEST_DIR}/test_ui_render${EXE}: ${TEST_DIR} ${TEST_DIR}/test_ui_render.c lib/libqgl.${SO}
	${cc} -o $@ ${TEST_DIR}/test_ui_render.c ${CFLAGS} ${TEST_CFLAGS} \
		${LDFLAGS} ${TEST_LDFLAGS} ${TEST_LDLIBS}

${TEST_DIR}/test_integration${EXE}: ${TEST_DIR} ${TEST_DIR}/test_integration.c lib/libqgl.${SO}
	${cc} -o $@ ${TEST_DIR}/test_integration.c ${CFLAGS} ${TEST_CFLAGS} \
		${LDFLAGS} ${TEST_LDFLAGS} ${TEST_LDLIBS}

test-build: lib/libqgl.${SO} ${TEST_BINS}

test: test-build
	@echo "Running test_core..."
	@LD_LIBRARY_PATH=./lib ./${TEST_DIR}/test_core
	@echo "Running test_textures..."
	@LD_LIBRARY_PATH=./lib ./${TEST_DIR}/test_textures
	@echo "Running test_fonts..."
	@LD_LIBRARY_PATH=./lib ./${TEST_DIR}/test_fonts
	@echo "Running test_tilemaps..."
	@LD_LIBRARY_PATH=./lib ./${TEST_DIR}/test_tilemaps
	@echo "Running test_ui_layout..."
	@LD_LIBRARY_PATH=./lib ./${TEST_DIR}/test_ui_layout
	@echo "Running test_ui_style..."
	@LD_LIBRARY_PATH=./lib ./${TEST_DIR}/test_ui_style
	@echo "Running test_ui_render..."
	@LD_LIBRARY_PATH=./lib ./${TEST_DIR}/test_ui_render
	@echo "Running test_integration..."
	@LD_LIBRARY_PATH=./lib ./${TEST_DIR}/test_integration
	@echo "All tests passed!"

.PHONY: test test-build

# Examples configuration
EXAMPLES_DIR := examples
EXAMPLES_CFLAGS := -I${PWD}/include
EXAMPLES_LDFLAGS := -L${PWD}/lib
EXAMPLES_LDLIBS := -lqgl ${LDLIBS}

# Example binaries
EXAMPLE_BINS := ${EXAMPLES_DIR}/01_basic_rendering${EXE} \
	${EXAMPLES_DIR}/02_textures${EXE} \
	${EXAMPLES_DIR}/03_fonts${EXE} \
	${EXAMPLES_DIR}/04_tilemaps${EXE} \
	${EXAMPLES_DIR}/05_ui_layout${EXE} \
	${EXAMPLES_DIR}/06_ui_advanced${EXE}

${EXAMPLES_DIR}:
	@mkdir -p ${EXAMPLES_DIR} 2>/dev/null || true

${EXAMPLES_DIR}/01_basic_rendering${EXE}: ${EXAMPLES_DIR} ${EXAMPLES_DIR}/01_basic_rendering.c lib/libqgl.${SO}
	${cc} -o $@ ${EXAMPLES_DIR}/01_basic_rendering.c ${CFLAGS} ${EXAMPLES_CFLAGS} \
		${LDFLAGS} ${EXAMPLES_LDFLAGS} ${EXAMPLES_LDLIBS}

${EXAMPLES_DIR}/02_textures${EXE}: ${EXAMPLES_DIR} ${EXAMPLES_DIR}/02_textures.c lib/libqgl.${SO}
	${cc} -o $@ ${EXAMPLES_DIR}/02_textures.c ${CFLAGS} ${EXAMPLES_CFLAGS} \
		${LDFLAGS} ${EXAMPLES_LDFLAGS} ${EXAMPLES_LDLIBS}

${EXAMPLES_DIR}/03_fonts${EXE}: ${EXAMPLES_DIR} ${EXAMPLES_DIR}/03_fonts.c lib/libqgl.${SO}
	${cc} -o $@ ${EXAMPLES_DIR}/03_fonts.c ${CFLAGS} ${EXAMPLES_CFLAGS} \
		${LDFLAGS} ${EXAMPLES_LDFLAGS} ${EXAMPLES_LDLIBS}

${EXAMPLES_DIR}/04_tilemaps${EXE}: ${EXAMPLES_DIR} ${EXAMPLES_DIR}/04_tilemaps.c lib/libqgl.${SO}
	${cc} -o $@ ${EXAMPLES_DIR}/04_tilemaps.c ${CFLAGS} ${EXAMPLES_CFLAGS} \
		${LDFLAGS} ${EXAMPLES_LDFLAGS} ${EXAMPLES_LDLIBS}

${EXAMPLES_DIR}/05_ui_layout${EXE}: ${EXAMPLES_DIR} ${EXAMPLES_DIR}/05_ui_layout.c lib/libqgl.${SO}
	${cc} -o $@ ${EXAMPLES_DIR}/05_ui_layout.c ${CFLAGS} ${EXAMPLES_CFLAGS} \
		${LDFLAGS} ${EXAMPLES_LDFLAGS} ${EXAMPLES_LDLIBS}

${EXAMPLES_DIR}/06_ui_advanced${EXE}: ${EXAMPLES_DIR} ${EXAMPLES_DIR}/06_ui_advanced.c lib/libqgl.${SO}
	${cc} -o $@ ${EXAMPLES_DIR}/06_ui_advanced.c ${CFLAGS} ${EXAMPLES_CFLAGS} \
		${LDFLAGS} ${EXAMPLES_LDFLAGS} ${EXAMPLES_LDLIBS}

examples: lib/libqgl.${SO} ${EXAMPLE_BINS}

.PHONY: examples
