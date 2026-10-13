/* SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0 */
#include "../../kexts/NVRMFB/nvscanout_abi.h"

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    const char *path = argc > 1 ? argv[1] : "/dev/shm/nvscanout-test";
    int fd = open(path, O_RDWR | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
    if (fd < 0 || ftruncate(fd, NVSCAN_SHMEM_BYTES)) {
        perror(path);
        return 1;
    }
    uint8_t *shared = mmap(NULL, NVSCAN_SHMEM_BYTES, PROT_READ | PROT_WRITE,
                           MAP_SHARED, fd, 0);
    if (shared == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    close(fd);

    struct nvscan_header *header = (void *)shared;
    memset(header, 0, sizeof(*header));
    header->magic = NVSCAN_MAGIC;
    header->version = NVSCAN_VERSION;
    header->header_bytes = NVSCAN_HEADER_BYTES;
    header->width = 1920;
    header->height = 1080;
    header->stride = 1920 * 4;
    header->pixel_format = NVSCAN_X8R8G8B8;
    header->slot_bytes = NVSCAN_SLOT_BYTES;
    header->slot_count = 2;

    for (uint64_t frame = 1;; frame++) {
        uint32_t slot = frame & 1;
        uint32_t *pixels = (void *)(shared + nvscan_slot_offset(slot));
        for (uint32_t y = 0; y < header->height; y++) {
            for (uint32_t x = 0; x < header->width; x++) {
                uint8_t r = (x + frame * 4) & 255;
                uint8_t g = (y + frame * 2) & 255;
                uint8_t b = ((x ^ y) + frame) & 255;
                pixels[(uint64_t)y * header->width + x] =
                    0xff000000u | (uint32_t)r << 16 | (uint32_t)g << 8 | b;
            }
        }
        header->write_slot = slot;
        __atomic_store_n(&header->frame_sequence, frame, __ATOMIC_RELEASE);
        usleep(16667);
    }
}
