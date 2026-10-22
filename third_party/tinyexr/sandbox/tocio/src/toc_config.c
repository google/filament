/*
 * tocio - config model: parse a YAML node tree into a queryable OCIO config and
 * provide colorspace/role/display/view lookups.
 *
 * Reimplemented from the OpenColorIO config format (BSD-3-Clause).
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "toc_internal.h"

toc_result toc_config_parse(const char *yaml, size_t len, const toc_allocator *a,
                            toc_config **out) {
    toc_config *cfg;
    toc_result rc;
    int err = 0;
    if (!yaml || !out) return TOC_ERROR_INVALID_ARGUMENT;
    *out = NULL;
    if (!a) a = toc_default_allocator();
    cfg = (toc_config *)toc_malloc(a, sizeof(*cfg));
    if (!cfg) return TOC_ERROR_OUT_OF_MEMORY;
    memset(cfg, 0, sizeof(*cfg));
    cfg->alloc = *a;
    toc_arena_init(&cfg->ar, a);
    rc = toc_yaml_parse(yaml, len, &cfg->ar, &cfg->root, &err);
    if (!TOC_OK(rc)) {
        toc_arena_free(&cfg->ar);
        toc_free(a, cfg);
        return rc;
    }
    *out = cfg;
    return TOC_SUCCESS;
}

void toc_config_free(toc_config *cfg) {
    toc_allocator a;
    if (!cfg) return;
    a = cfg->alloc;
    toc_arena_free(&cfg->ar);
    toc_free(&a, cfg);
}

void toc_config_set_file_reader(toc_config *cfg, toc_file_reader fn,
                                void *user) {
    if (!cfg) return;
    cfg->reader = fn;
    cfg->reader_user = user;
}

/* ---- colorspace / role lookups ------------------------------------------- */
static const toc_node *cs_seq(const toc_config *cfg, const char *key) {
    return toc_node_map_get(cfg->root, key);
}

/* The view-transform section is `view_transforms` (plural) in the OCIO v2 spec
 * and in every real config; older/handwritten configs may use the singular
 * `view_transform`. Accept both. */
static const toc_node *vt_seq(const toc_config *cfg) {
    const toc_node *s = cs_seq(cfg, "view_transforms");
    if (!s) s = cs_seq(cfg, "view_transform");
    return s;
}

static int name_matches(const toc_node *cs, const char *name) {
    const toc_node *nm = toc_node_map_get(cs, "name");
    const toc_node *al;
    size_t i;
    const char *s = nm ? toc_node_scalar(nm) : NULL;
    if (s && strcmp(s, name) == 0) return 1;
    al = toc_node_map_get(cs, "aliases");
    if (al && al->kind == TOC_NODE_SEQ) {
        for (i = 0; i < al->n_items; ++i) {
            const char *as = toc_node_scalar(al->items[i]);
            if (as && strcmp(as, name) == 0) return 1;
        }
    } else if (al && al->kind == TOC_NODE_SCALAR) {
        const char *as = toc_node_scalar(al);
        if (as && strcmp(as, name) == 0) return 1;
    }
    return 0;
}

static const toc_node *find_in_seq(const toc_node *seq, const char *name) {
    size_t i;
    if (!seq || seq->kind != TOC_NODE_SEQ) return NULL;
    for (i = 0; i < seq->n_items; ++i)
        if (name_matches(seq->items[i], name)) return seq->items[i];
    return NULL;
}

const toc_node *toc_cfg_find_colorspace(const toc_config *cfg, const char *name) {
    const toc_node *r;
    if (!cfg || !name) return NULL;
    r = find_in_seq(cs_seq(cfg, "colorspaces"), name);
    if (r) return r;
    return find_in_seq(cs_seq(cfg, "display_colorspaces"), name);
}

int toc_cfg_cs_in_display_section(const toc_config *cfg, const char *name) {
    if (!cfg || !name) return 0;
    return find_in_seq(cs_seq(cfg, "display_colorspaces"),
                       toc_cfg_resolve_role(cfg, name)) != NULL;
}

/* ---- look introspection (public API) ------------------------------------- */
int toc_config_num_looks(const toc_config *cfg) {
    const toc_node *s = cfg ? cs_seq(cfg, "looks") : NULL;
    return (s && s->kind == TOC_NODE_SEQ) ? (int)s->n_items : 0;
}
const char *toc_config_look_name(const toc_config *cfg, int index) {
    const toc_node *s = cfg ? cs_seq(cfg, "looks") : NULL;
    if (!s || s->kind != TOC_NODE_SEQ || index < 0 ||
        (size_t)index >= s->n_items)
        return NULL;
    return toc_node_scalar(toc_node_map_get(s->items[index], "name"));
}
int toc_config_num_view_transforms(const toc_config *cfg) {
    const toc_node *s = cfg ? vt_seq(cfg) : NULL;
    return (s && s->kind == TOC_NODE_SEQ) ? (int)s->n_items : 0;
}
const char *toc_config_view_transform_name(const toc_config *cfg, int index) {
    const toc_node *s = cfg ? vt_seq(cfg) : NULL;
    if (!s || s->kind != TOC_NODE_SEQ || index < 0 ||
        (size_t)index >= s->n_items)
        return NULL;
    return toc_node_scalar(toc_node_map_get(s->items[index], "name"));
}

const char *toc_cfg_resolve_role(const toc_config *cfg, const char *name) {
    const toc_node *roles, *r;
    if (!cfg || !name) return name;
    roles = toc_node_map_get(cfg->root, "roles");
    r = roles ? toc_node_map_get(roles, name) : NULL;
    if (r) {
        const char *cs = toc_node_scalar(r);
        if (cs) return cs;
    }
    return name;
}

int toc_cfg_is_data(const toc_node *cs) {
    const toc_node *d = toc_node_map_get(cs, "isdata");
    const char *s = d ? toc_node_scalar(d) : NULL;
    return s && (strcmp(s, "true") == 0 || strcmp(s, "1") == 0);
}

const toc_node *toc_cfg_cs_transform(const toc_node *cs, int want_to_ref,
                                     int *out_invert) {
    /* OCIO v1 used to_reference/from_reference; v2 splits the reference into a
     * scene-referred and a display-referred space (to/from_scene_reference,
     * to/from_display_reference). Recognize all of them so a display colorspace
     * is never silently treated as identity - if its transform is a builtin we
     * don't implement, walk() reports it loudly as UNSUPPORTED. */
    const toc_node *to = toc_node_map_get(cs, "to_reference");
    const toc_node *from = toc_node_map_get(cs, "from_reference");
    if (!to) to = toc_node_map_get(cs, "to_scene_reference");
    if (!from) from = toc_node_map_get(cs, "from_scene_reference");
    if (!to) to = toc_node_map_get(cs, "to_display_reference");
    if (!from) from = toc_node_map_get(cs, "from_display_reference");
    if (out_invert) *out_invert = 0;
    if (want_to_ref) {
        if (to) return to;
        if (from) { if (out_invert) *out_invert = 1; return from; }
    } else {
        if (from) return from;
        if (to) { if (out_invert) *out_invert = 1; return to; }
    }
    return NULL;
}

/* ---- introspection ------------------------------------------------------- */
int toc_config_num_colorspaces(const toc_config *cfg) {
    const toc_node *s = cfg ? cs_seq(cfg, "colorspaces") : NULL;
    return (s && s->kind == TOC_NODE_SEQ) ? (int)s->n_items : 0;
}
const char *toc_config_colorspace_name(const toc_config *cfg, int index) {
    const toc_node *s = cfg ? cs_seq(cfg, "colorspaces") : NULL;
    if (!s || s->kind != TOC_NODE_SEQ || index < 0 ||
        (size_t)index >= s->n_items)
        return NULL;
    return toc_node_scalar(toc_node_map_get(s->items[index], "name"));
}
const char *toc_config_role(const toc_config *cfg, const char *role) {
    const toc_node *roles = cfg ? toc_node_map_get(cfg->root, "roles") : NULL;
    const toc_node *r = roles ? toc_node_map_get(roles, role) : NULL;
    return r ? toc_node_scalar(r) : NULL;
}
int toc_config_num_displays(const toc_config *cfg) {
    const toc_node *d = cfg ? toc_node_map_get(cfg->root, "displays") : NULL;
    return (d && d->kind == TOC_NODE_MAP) ? (int)d->n_pairs : 0;
}
const char *toc_config_display_name(const toc_config *cfg, int index) {
    const toc_node *d = cfg ? toc_node_map_get(cfg->root, "displays") : NULL;
    if (!d || d->kind != TOC_NODE_MAP || index < 0 ||
        (size_t)index >= d->n_pairs)
        return NULL;
    return d->keys[index];
}
int toc_config_num_views(const toc_config *cfg, const char *display) {
    const toc_node *d = cfg ? toc_node_map_get(cfg->root, "displays") : NULL;
    const toc_node *v = d ? toc_node_map_get(d, display) : NULL;
    return (v && v->kind == TOC_NODE_SEQ) ? (int)v->n_items : 0;
}
/* ---- view transform lookup ------------------------------------------------ */
const toc_node *toc_cfg_find_view_transform(const toc_config *cfg,
                                            const char *name) {
    return find_in_seq(vt_seq(cfg), name);
}

/* The view transform OCIO uses to bridge the scene and display references in a
 * plain colorspace<->colorspace conversion: the `default_view_transform` by name,
 * else the first view transform. NULL if there are none. */
const toc_node *toc_cfg_default_view_transform(const toc_config *cfg) {
    const toc_node *vts, *name;
    if (!cfg) return NULL;
    name = toc_node_map_get(cfg->root, "default_view_transform");
    if (name) {
        const char *s = toc_node_scalar(name);
        const toc_node *vt = s ? toc_cfg_find_view_transform(cfg, s) : NULL;
        if (vt) return vt;
    }
    vts = vt_seq(cfg);
    return (vts && vts->kind == TOC_NODE_SEQ && vts->n_items) ? vts->items[0]
                                                              : NULL;
}

/* ---- look lookup --------------------------------------------------------- */
const toc_node *toc_cfg_find_look(const toc_config *cfg, const char *name) {
    return find_in_seq(cs_seq(cfg, "looks"), name);
}
int toc_cfg_view_looks(const toc_node *vnode, const char **names, int max) {
    const toc_node *lk = toc_node_map_get(vnode, "looks");
    const char *s;
    int n = 0;
    if (!lk || lk->kind != TOC_NODE_SCALAR) return 0;
    s = toc_node_scalar(lk);
    while (*s && n < max) {
        while (*s == ' ' || *s == ',') ++s;
        if (!*s) break;
        names[n++] = s;
        while (*s && *s != ',') ++s;
        if (*s) { *((char *)s) = '\0'; ++s; } /* mutate in place (owned by arena) */
    }
    return n;
}

/* ---- active displays/views ------------------------------------------------ */
int toc_config_num_active_displays(const toc_config *cfg) {
    const toc_node *a = cfg ? toc_node_map_get(cfg->root, "active_displays") : NULL;
    if (!a) return -1; /* not set = all active */
    if (a->kind == TOC_NODE_SEQ) return (int)a->n_items;
    return 0;
}
const char *toc_config_active_display_name(const toc_config *cfg, int index) {
    const toc_node *a = cfg ? toc_node_map_get(cfg->root, "active_displays") : NULL;
    if (!a || a->kind != TOC_NODE_SEQ || index < 0 ||
        (size_t)index >= a->n_items)
        return NULL;
    return toc_node_scalar(a->items[index]);
}

int toc_config_num_active_views(const toc_config *cfg) {
    const toc_node *a = cfg ? toc_node_map_get(cfg->root, "active_views") : NULL;
    if (!a) return -1; /* not set = all active */
    if (a->kind == TOC_NODE_SEQ) return (int)a->n_items;
    return 0;
}
const char *toc_config_active_view_name(const toc_config *cfg, int index) {
    const toc_node *a = cfg ? toc_node_map_get(cfg->root, "active_views") : NULL;
    if (!a || a->kind != TOC_NODE_SEQ || index < 0 ||
        (size_t)index >= a->n_items)
        return NULL;
    return toc_node_scalar(a->items[index]);
}

const char *toc_config_view_name(const toc_config *cfg, const char *display,
                                 int index) {
    const toc_node *d = cfg ? toc_node_map_get(cfg->root, "displays") : NULL;
    const toc_node *v = d ? toc_node_map_get(d, display) : NULL;
    if (!v || v->kind != TOC_NODE_SEQ || index < 0 ||
        (size_t)index >= v->n_items)
        return NULL;
    return toc_node_scalar(toc_node_map_get(v->items[index], "name"));
}
