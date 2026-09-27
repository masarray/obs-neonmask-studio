/* SPDX-License-Identifier: GPL-2.0-or-later */
#pragma once
#include <stdbool.h>
#include <stddef.h>

#define NM_SVG_PATH_MAX 1024u
#define NM_SVG_MAX_BYTES 65536u
#define NM_SVG_MAX_EDGES 512u
#define NM_SVG_SDF_SIZE 256u

typedef struct nm_svg_edge {
    float ax, ay, bx, by;
} nm_svg_edge;

typedef struct nm_svg_shape {
    float min_x, min_y, width, height;
    size_t edge_count;
    bool evenodd;
    nm_svg_edge edges[NM_SVG_MAX_EDGES];
} nm_svg_shape;

/* Strict LOCAL, no-resource, path-only SVG subset. Unknown tags, attributes,
 * entities, DTDs, event handlers and unsupported commands fail closed.
 * No external dependencies, graphics contexts or arbitrary SVG execution. */
bool nm_svg_parse(const char *bytes, size_t length, nm_svg_shape *out,
                  char *why, size_t why_size);
bool nm_svg_read_local(const char *path, nm_svg_shape *out,
                       char *why, size_t why_size);
/* Expensive only on a changed asset/mask size; NEVER called per video frame.
 * Signed physical-pixel distances after aspect-preserving contain fit. */
bool nm_svg_raster_sdf(const nm_svg_shape *shape, float half_width,
                       float half_height, float *output, unsigned size);
