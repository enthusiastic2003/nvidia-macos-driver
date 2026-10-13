/*
 * NullMoth/QEMU shared scanout ABI.
 * SPDX-License-Identifier: PolyForm-Noncommercial-1.0.0
 */
#pragma once

#include <stdint.h>

#define NVSCAN_MAGIC UINT64_C(0x4e565343414e3031) /* "NVSCAN01" */
#define NVSCAN_VERSION 1u
#define NVSCAN_HEADER_BYTES 4096u
#define NVSCAN_SLOT_BYTES (32u * 1024u * 1024u)
#define NVSCAN_SHMEM_BYTES (128u * 1024u * 1024u)

enum nvscan_pixel_format {
    NVSCAN_X8R8G8B8 = 1,
};

struct nvscan_header {
    uint64_t magic;
    uint32_t version;
    uint32_t header_bytes;
    uint32_t width;
    uint32_t height;
    uint32_t stride;
    uint32_t pixel_format;
    uint32_t slot_bytes;
    uint32_t slot_count;
    uint32_t write_slot;
    uint32_t flags;
    uint64_t frame_sequence;
    uint64_t frame_time_ns;
    uint64_t dropped_frames;
    /* The hardware cursor (a separate display plane, so not in the frames):
     * its image (straight ARGB, cursor_width x cursor_height, pitch
     * cursor_width * 4) at NVSCAN_CURSOR_OFFSET, and its state. Each
     * sequence is bumped after the data it covers is written; 0 = none. */
    uint32_t cursor_width;
    uint32_t cursor_height;
    uint32_t cursor_hot_x;
    uint32_t cursor_hot_y;
    int32_t  cursor_x;          /* the image's top-left on screen */
    int32_t  cursor_y;
    uint32_t cursor_visible;
    uint32_t cursor_pad;
    uint64_t cursor_image_sequence;
    uint64_t cursor_state_sequence;
    uint8_t reserved[NVSCAN_HEADER_BYTES - 120];
};

#if defined(__cplusplus)
static_assert(sizeof(struct nvscan_header) == NVSCAN_HEADER_BYTES,
              "nvscan header must occupy one page");
#else
_Static_assert(sizeof(struct nvscan_header) == NVSCAN_HEADER_BYTES,
               "nvscan header must occupy one page");
#endif

#define NVSCAN_CURSOR_MAX 256u
#define NVSCAN_CURSOR_OFFSET (NVSCAN_HEADER_BYTES + 2ull * NVSCAN_SLOT_BYTES)

static inline uint64_t nvscan_slot_offset(uint32_t slot)
{
    return NVSCAN_HEADER_BYTES + (uint64_t)slot * NVSCAN_SLOT_BYTES;
}

