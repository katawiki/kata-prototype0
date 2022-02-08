# makefile - build system for the 'UNIX' platform
#
# for the web platform, `make -f web.make`
#
# @author: Cade Brown <me@cade.site>


### Config ###

# TODO: rpath?
CFLAGS      += -Ofast -march=native
LDFLAGS     += -Llib

# debug
CFLAGS      += -g

### Input Files ###

# C source code
SRC_C       := $(wildcard src/*.c)

# add builtins
SRC_C       += $(wildcard src/types/*.c)

# add modules
SRC_C       += $(wildcard src/sys/*.c)
SRC_C       += $(wildcard src/mem/*.c)
SRC_C       += $(wildcard src/io/*.c)
SRC_C       += $(wildcard src/bf/*.c)
SRC_C       += $(wildcard src/ks/*.c)

# C headers
SRC_H       := $(wildcard include/kata/*.h)

# C-API tests
TEST_C      := $(wildcard test/*.c)


### Extras ###

# define standard colors and escape codes
# SEE: https://en.wikipedia.org/wiki/ANSI_escape_code#Colors
RST         := "\033[0m"
BOLD        := "\033[1m"
FAINT       := "\033[2m"
ITAL        := "\033[3m"
ULIN        := "\033[3m"
SLOWB       := "\033[3m"
FASTB       := "\033[3m"
REV         := "\033[3m"
STRIKE      := "\033[3m"
FONT0       := "\033[10m"
FONT1       := "\033[11m"
FONT2       := "\033[12m"
FONT3       := "\033[13m"
FONT4       := "\033[14m"
FONT5       := "\033[15m"
FONT6       := "\033[16m"
FONT7       := "\033[17m"
FONT8       := "\033[18m"
FONT9       := "\033[19m"
REG         := "\033[22m"
NOITAL      := "\033[23m"
NOULIN      := "\033[24m"
NOBLINK     := "\033[25m"
MONO        := "\033[26m" # "propertional scaling"
NOREV       := "\033[27m"
NOCONCEAL   := "\033[28m"
NOSTRIKE    := "\033[29m"

# colors (foregrounds)
BLK         := "\033[30m"
RED         := "\033[31m"
GRN         := "\033[32m"
YLW         := "\033[33m"
BLU         := "\033[34m"
MAG         := "\033[35m"
CYN         := "\033[36m"
WHT         := "\033[37m"
LBLK        := "\033[90m"
LRED        := "\033[91m"
LGRN        := "\033[92m"
LYLW        := "\033[93m"
LBLU        := "\033[94m"
LMAG        := "\033[95m"
LCYN        := "\033[96m"
LWHT        := "\033[97m"

# colors (backgrounds)
BLKB        := "\033[40m"
REDB        := "\033[41m"
GRNB        := "\033[42m"
YLWB        := "\033[43m"
BLUB        := "\033[44m"
MAGB        := "\033[45m"
CYNB        := "\033[46m"
WHTB        := "\033[47m"
LBLKB       := "\033[100m"
LREDB       := "\033[101m"
LGRNB       := "\033[102m"
LYLWB       := "\033[103m"
LBLUB       := "\033[104m"
LMAGB       := "\033[105m"
LCYNB       := "\033[106m"
LWHTB       := "\033[107m"

### Output Files ###

# subst with '.unix.o' so we can build multiple platforms
OBJ_O       := $(patsubst %.c,%.unix.o,$(SRC_C))
TEST_O      := $(patsubst %.c,%.unix.o,$(TEST_C))

TEST_BINS   := $(patsubst %.c,bin/%,$(TEST_C))
TEST_RUNS   := $(patsubst %.c,%,$(TEST_C))


### Targets ###

all: bin/ks lib/libkata.so

test: $(TEST_BINS) $(TEST_RUNS)

clean:
	rm -f $(wildcard bin/ks)
	rm -f $(wildcard $(OBJ_O))
	rm -f $(wildcard $(TEST_BINS))

lib/libkata.so: $(OBJ_O)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $(OBJ_O) -shared \
		$(LDFLAGS)

bin/ks: cli/ks.unix.o lib/libkata.so
	@mkdir -p $(dir $@)
	$(CC) -o $@ $< -lkata \
		$(CFLAGS) \
		$(LDFLAGS) \
		'-Wl,-rpath,$$ORIGIN/../lib'

bin/test/%: test/%.unix.o lib/libkata.so
	@mkdir -p $(dir $@)
	$(CC) -o $@ $< -lkata \
		$(CFLAGS) \
		$(LDFLAGS) \
		'-Wl,-rpath,$$ORIGIN/../../lib'

# run a test
test/%: bin/test/%
	@echo $(BLU)"TEST: ./$@"$(RST) && ./$<&& echo $(GRN)PASS: ./\$@$(RST) && echo "" || (echo $(RED)$(BOLD)FAIL: ./\$@$(RST) && echo "" && exit 1)

.PHONY: all test clean


### Rules ###

# builds single source file, using C compilers
%.unix.o: %.c $(SRC_H)
	$(CC) -o $@ $< -c -fPIC -Iinclude \
		$(CFLAGS)


