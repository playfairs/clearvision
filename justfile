CC := "clang"
CFLAGS := "-std=c23 -Wall -Wextra -Werror -pedantic"
LDFLAGS := ""
DEBUG_CFLAGS := CFLAGS + " -g -O0 -DDEBUG"
RELEASE_CFLAGS := CFLAGS + " -O3 -DNDEBUG"

SRC_DIR := "src"
PKG_CONFIG_CFLAGS := `pkg-config --cflags sqlite3 libpcre2-8 ncurses`
PKG_CONFIG_LIBS := `pkg-config --libs sqlite3 libpcre2-8 ncurses`
SOURCES := `find src -name '*.c' | tr '\n' ' '`

default: build

build:
    @echo "Building ClearVision (debug)."
    {{CC}} {{DEBUG_CFLAGS}} {{PKG_CONFIG_CFLAGS}} -I{{SRC_DIR}} {{SOURCES}} -o cv {{PKG_CONFIG_LIBS}} -lpthread

release:
    @echo "Building ClearVision (release)."
    {{CC}} {{RELEASE_CFLAGS}} {{PKG_CONFIG_CFLAGS}} -I{{SRC_DIR}} {{SOURCES}} -o cv {{PKG_CONFIG_LIBS}} -lpthread

run: build
    ./cv

clean:
    @echo "Cleaning."
    rm -f cv
    find {{SRC_DIR}} -name '*.o' -delete

format:
    @echo "Formatting."
    clang-format -i {{SOURCES}}

lint:
    @echo "Linting."
    clang-tidy {{SOURCES}} -- {{CFLAGS}} {{PKG_CONFIG_CFLAGS}} -I{{SRC_DIR}}

debug: build
    gdb ./cv

bench: release
    @echo "Benchmarking."
    ./cv benchmark

install: release
    @echo "Installing ClearVision."
    install -Dm755 cv /usr/local/bin/cv

uninstall:
    @echo "Uninstalling ClearVision."
    rm -f /usr/local/bin/cv
