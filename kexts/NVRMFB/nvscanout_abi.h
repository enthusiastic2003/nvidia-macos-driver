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
    uint8_t reserved[NVSCAN_HEADER_BYTES - 72];
};

#if defined(__cplusplus)
static_assert(sizeof(struct nvscan_header) == NVSCAN_HEADER_BYTES,
              "nvscan header must occupy one page");
#else
_Static_assert(sizeof(struct nvscan_header) == NVSCAN_HEADER_BYTES,
               "nvscan header must occupy one page");
#endif

static inline uint64_t nvscan_slot_offset(uint32_t slot)
{
    return NVSCAN_HEADER_BYTES + (uint64_t)slot * NVSCAN_SLOT_BYTES;
}

