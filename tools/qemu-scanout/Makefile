CC ?= cc
CFLAGS ?= -O2 -g -Wall -Wextra

.PHONY: all clean
all: nvscan-viewer nvscan-test-pattern

nvscan-viewer: nvscan-viewer.c ../../kexts/NVRMFB/nvscanout_abi.h
	$(CC) $(CFLAGS) $(shell pkg-config --cflags sdl2) $< -o $@ \
		$(shell pkg-config --libs sdl2)

nvscan-test-pattern: nvscan-test-pattern.c ../../kexts/NVRMFB/nvscanout_abi.h
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f nvscan-viewer nvscan-test-pattern
