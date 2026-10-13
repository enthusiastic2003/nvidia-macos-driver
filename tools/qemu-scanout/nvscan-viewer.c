/* SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0 */
#include "../../kexts/NVRMFB/nvscanout_abi.h"

#include <SDL.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

static void die(const char *what)
{
    fprintf(stderr, "nvscan-viewer: %s: %s\n", what, strerror(errno));
    exit(1);
}

int main(int argc, char **argv)
{
    const char *path = argc > 1 ? argv[1] : "/dev/shm/nvscanout";
    int fd = open(path, O_RDONLY | O_CLOEXEC);
    if (fd < 0) {
        die(path);
    }

    uint8_t *shared = mmap(NULL, NVSCAN_SHMEM_BYTES, PROT_READ, MAP_SHARED,
                           fd, 0);
    if (shared == MAP_FAILED) {
        die("mmap");
    }
    close(fd);

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "nvscan-viewer: SDL_Init: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("macOS NVIDIA scanout",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 960, 540,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture *texture = NULL;
    uint32_t texture_w = 0, texture_h = 0;
    uint64_t displayed = 0;
    bool running = window && renderer;

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT ||
                (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)) {
                running = false;
            }
        }

        struct nvscan_header snapshot;
        const struct nvscan_header *header = (const void *)shared;
        uint64_t sequence = __atomic_load_n(&header->frame_sequence,
                                             __ATOMIC_ACQUIRE);
        if (sequence == displayed || header->magic != NVSCAN_MAGIC ||
            header->version != NVSCAN_VERSION) {
            SDL_Delay(2);
            continue;
        }
        memcpy(&snapshot, header, sizeof(snapshot));
        if (!snapshot.width || !snapshot.height ||
            snapshot.pixel_format != NVSCAN_X8R8G8B8 ||
            snapshot.write_slot >= snapshot.slot_count ||
            snapshot.slot_count > 2 ||
            (uint64_t)snapshot.stride * snapshot.height > snapshot.slot_bytes ||
            nvscan_slot_offset(snapshot.write_slot) + snapshot.slot_bytes >
                NVSCAN_SHMEM_BYTES) {
            SDL_Delay(10);
            continue;
        }

        if (!texture || texture_w != snapshot.width ||
            texture_h != snapshot.height) {
            SDL_DestroyTexture(texture);
            texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                SDL_TEXTUREACCESS_STREAMING, snapshot.width, snapshot.height);
            texture_w = snapshot.width;
            texture_h = snapshot.height;
            SDL_SetWindowSize(window, snapshot.width / 2,
                              snapshot.height / 2);
        }
        if (!texture) {
            fprintf(stderr, "nvscan-viewer: texture: %s\n", SDL_GetError());
            break;
        }

        const void *pixels = shared + nvscan_slot_offset(snapshot.write_slot);
        SDL_UpdateTexture(texture, NULL, pixels, snapshot.stride);
        if (__atomic_load_n(&header->frame_sequence, __ATOMIC_ACQUIRE) !=
            sequence) {
            continue;
        }
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);
        SDL_RenderPresent(renderer);
        displayed = sequence;
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    munmap(shared, NVSCAN_SHMEM_BYTES);
    return 0;
}
