/*
 * DRM based mode setting test program
 * Copyright 2008 Tungsten Graphics
 *   Jakob Bornecrantz <jakob@tungstengraphics.com>
 * Copyright 2008 Intel Corporation
 *   Jesse Barnes <jesse.barnes@intel.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/*
 * This fairly simple test program dumps output in a similar format to the
 * "xrandr" tool everyone knows & loves.  It's necessarily slightly different
 * since the kernel separates outputs into encoder and connector structures,
 * each with their own unique ID.  The program also allows test testing of the
 * memory management and mode setting APIs by allowing the user to specify a
 * connector and mode to use for mode setting.  If all works as expected, a
 * blue background should be painted on the monitor attached to the specified
 * connector after the selected mode is set.
 *
 * TODO: use cairo to write the mode info on the selected output once
 *       the mode has been programmed, along with possible test patterns.
 */

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <poll.h>
#include <sys/time.h>
#if HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif
#include <math.h>

#include "xf86drm.h"
#include "xf86drmMode.h"
#include "drm_fourcc.h"

#include "util/common.h"
#include "util/format.h"
#include "util/kms.h"
#include "util/pattern.h"

#include "buffers.h"
#include "cursor.h"

static enum util_fill_pattern primary_fill = UTIL_PATTERN_SMPTE;
static enum util_fill_pattern secondary_fill = UTIL_PATTERN_TILES;
static drmModeModeInfo user_mode;

struct crtc {
	drmModeCrtc *crtc;
	drmModeObjectProperties *props;
	drmModePropertyRes **props_info;
	drmModeModeInfo *mode;
};

struct encoder {
	drmModeEncoder *encoder;
};

struct connector {
	drmModeConnector *connector;
	drmModeObjectProperties *props;
	drmModePropertyRes **props_info;
	char *name;
};

struct fb {
	drmModeFB *fb;
};

struct plane {
	drmModePlane *plane;
	drmModeObjectProperties *props;
	drmModePropertyRes **props_info;
};

struct resources {
	struct crtc *crtcs;
	int count_crtcs;
	struct encoder *encoders;
	int count_encoders;
	struct connector *connectors;
	int count_connectors;
	struct fb *fbs;
	int count_fbs;
	struct plane *planes;
	uint32_t count_planes;
};

struct device {
	int fd;

	struct resources *resources;

	struct {
		unsigned int width;
		unsigned int height;

		unsigned int fb_id;
		struct bo *bo;
		struct bo *cursor_bo;
	} mode;

	int use_atomic;
	drmModeAtomicReq *req;
	int32_t writeback_fence_fd;
};

static inline int64_t U642I64(uint64_t val)
{
	return (int64_t)*((int64_t *)&val);
}

static float mode_vrefresh(drmModeModeInfo *mode)
{
	unsigned int num, den;

	num = mode->clock;
	den = mode->htotal * mode->vtotal;

	if (mode->flags & DRM_MODE_FLAG_INTERLACE)
		num *= 2;
	if (mode->flags & DRM_MODE_FLAG_DBLSCAN)
		den *= 2;
	if (mode->vscan > 1)
		den *= mode->vscan;

	return num * 1000.00 / den;
}

#define bit_name_fn(res)					\
const char * res##_str(int type) {				\
	unsigned int i;						\
	const char *sep = "";					\
	for (i = 0; i < ARRAY_SIZE(res##_names); i++) {		\
		if (type & (1 << i)) {				\
			printf("%s%s", sep, res##_names[i]);	\
			sep = ", ";				\
		}						\
	}							\
	return NULL;						\
}

static const char *mode_type_names[] = {
	"builtin",
	"clock_c",
	"crtc_c",
	"preferred",
	"default",
	"userdef",
	"driver",
};

static bit_name_fn(mode_type)

static const char *mode_flag_names[] = {
	"phsync",
	"nhsync",
	"pvsync",
	"nvsync",
	"interlace",
	"dblscan",
	"csync",
	"pcsync",
	"ncsync",
	"hskew",
	"bcast",
	"pixmux",
	"dblclk",
	"clkdiv2"
};

static bit_name_fn(mode_flag)

static void dump_fourcc(uint32_t fourcc)
{
	char *name = drmGetFormatName(fourcc);
	printf(" %s", name);
	free(name);
}

static void dump_encoders(struct device *dev)
{
	drmModeEncoder *encoder;
	int i;

	printf("Encoders:\n");
	printf("id\tcrtc\ttype\tpossible crtcs\tpossible clones\t\n");
	for (i = 0; i < dev->resources->count_encoders; i++) {
		encoder = dev->resources->encoders[i].encoder;
		if (!encoder)
			continue;

		printf("%d\t%d\t%s\t0x%08x\t0x%08x\n",
		       encoder->encoder_id,
		       encoder->crtc_id,
		       util_lookup_encoder_type_name(encoder->encoder_type),
		       encoder->possible_crtcs,
		       encoder->possible_clones);
	}
	printf("\n");
}

static void dump_mode(drmModeModeInfo *mode, int index)
{
	printf("  #%i %s %.2f %d %d %d %d %d %d %d %d %d",
	       index,
	       mode->name,
	       mode_vrefresh(mode),
	       mode->hdisplay,
	       mode->hsync_start,
	       mode->hsync_end,
	       mode->htotal,
	       mode->vdisplay,
	       mode->vsync_start,
	       mode->vsync_end,
	       mode->vtotal,
	       mode->clock);

	printf(" flags: ");
	mode_flag_str(mode->flags);
	printf("; type: ");
	mode_type_str(mode->type);
	printf("\n");
}

static void dump_blob(struct device *dev, uint32_t blob_id)
{
	uint32_t i;
	unsigned char *blob_data;
	drmModePropertyBlobPtr blob;

	blob = drmModeGetPropertyBlob(dev->fd, blob_id);
	if (!blob) {
		printf("\n");
		return;
	}

	blob_data = blob->data;

	for (i = 0; i < blob->length; i++) {
		if (i % 16 == 0)
			printf("\n\t\t\t");
		printf("%.2hhx", blob_data[i]);
	}
	printf("\n");

	drmModeFreePropertyBlob(blob);
}

static const char *modifier_to_string(uint64_t modifier)
{
	static char mod_string[4096];

	char *modifier_name = drmGetFormatModifierName(modifier);
	char *vendor_name = drmGetFormatModifierVendor(modifier);
	memset(mod_string, 0x00, sizeof(mod_string));

	if (!modifier_name) {
		if (vendor_name)
			snprintf(mod_string, sizeof(mod_string), "%s_%s",
				 vendor_name, "UNKNOWN_MODIFIER");
		else
			snprintf(mod_string, sizeof(mod_string), "%s_%s",
				 "UNKNOWN_VENDOR", "UNKNOWN_MODIFIER");
		/* safe, as free is no-op for NULL */
		free(vendor_name);
		return mod_string;
	}

	if (modifier == DRM_FORMAT_MOD_LINEAR) {
		snprintf(mod_string, sizeof(mod_string), "%s", modifier_name);
		free(modifier_name);
		free(vendor_name);
		return mod_string;
	}

	snprintf(mod_string, sizeof(mod_string), "%s_%s",
		 vendor_name, modifier_name);

	free(modifier_name);
	free(vendor_name);
	return mod_string;
}

static void dump_in_formats(struct device *dev, uint32_t blob_id)
{
	drmModeFormatModifierIterator iter = {0};
	drmModePropertyBlobPtr blob;
	uint32_t fmt = 0;

	printf("\t\tin_formats blob decoded:\n");
	blob = drmModeGetPropertyBlob(dev->fd, blob_id);
	if (!blob) {
		printf("\n");
		return;
	}

	while (drmModeFormatModifierBlobIterNext(blob, &iter)) {
		if (!fmt || fmt != iter.fmt) {
			printf("%s\t\t\t", !fmt ? "" : "\n");
			fmt = iter.fmt;
			dump_fourcc(fmt);
			printf(": ");
		}

		printf(" %s(0x%"PRIx64")", modifier_to_string(iter.mod), iter.mod);
	}

	printf("\n");

	drmModeFreePropertyBlob(blob);
}

static void dump_prop(struct device *dev, drmModePropertyPtr prop,
		      uint32_t prop_id, uint64_t value)
{
	int i;
	printf("\t%d", prop_id);
	if (!prop) {
		printf("\n");
		return;
	}

	printf(" %s:\n", prop->name);

	printf("\t\tflags:");
	if (prop->flags & DRM_MODE_PROP_PENDING)
		printf(" pending");
	if (prop->flags & DRM_MODE_PROP_IMMUTABLE)
		printf(" immutable");
	if (drm_property_type_is(prop, DRM_MODE_PROP_SIGNED_RANGE))
		printf(" signed range");
	if (drm_property_type_is(prop, DRM_MODE_PROP_RANGE))
		printf(" range");
	if (drm_property_type_is(prop, DRM_MODE_PROP_ENUM))
		printf(" enum");
	if (drm_property_type_is(prop, DRM_MODE_PROP_BITMASK))
		printf(" bitmask");
	if (drm_property_type_is(prop, DRM_MODE_PROP_BLOB))
		printf(" blob");
	if (drm_property_type_is(prop, DRM_MODE_PROP_OBJECT))
		printf(" object");
	printf("\n");

	if (drm_property_type_is(prop, DRM_MODE_PROP_SIGNED_RANGE)) {
		printf("\t\tvalues:");
		for (i = 0; i < prop->count_values; i++)
			printf(" %"PRId64, U642I64(prop->values[i]));
		printf("\n");
	}

	if (drm_property_type_is(prop, DRM_MODE_PROP_RANGE)) {
		printf("\t\tvalues:");
		for (i = 0; i < prop->count_values; i++)
			printf(" %"PRIu64, prop->values[i]);
		printf("\n");
	}

	if (drm_property_type_is(prop, DRM_MODE_PROP_ENUM)) {
		printf("\t\tenums:");
		for (i = 0; i < prop->count_enums; i++)
			printf(" %s=%"PRIu64, prop->enums[i].name,
			       (uint64_t)prop->enums[i].value);
		printf("\n");
	} else if (drm_property_type_is(prop, DRM_MODE_PROP_BITMASK)) {
		printf("\t\tvalues:");
		for (i = 0; i < prop->count_enums; i++)
			printf(" %s=0x%llx", prop->enums[i].name,
			       (1LL << prop->enums[i].value));
		printf("\n");
	} else {
		assert(prop->count_enums == 0);
	}

	if (drm_property_type_is(prop, DRM_MODE_PROP_BLOB)) {
		printf("\t\tblobs:\n");
		for (i = 0; i < prop->count_blobs; i++)
			dump_blob(dev, prop->blob_ids[i]);
		printf("\n");
	} else {
		assert(prop->count_blobs == 0);
	}

	printf("\t\tvalue:");
	if (drm_property_type_is(prop, DRM_MODE_PROP_BLOB))
		dump_blob(dev, value);
	else if (drm_property_type_is(prop, DRM_MODE_PROP_SIGNED_RANGE))
		printf(" %"PRId64"\n", value);
	else
		printf(" %"PRIu64"\n", value);

	if (strcmp(prop->name, "IN_FORMATS") == 0)
		dump_in_formats(dev, value);
}

static void dump_connectors(struct device *dev)
{
	int i, j;

	printf("Connectors:\n");
	printf("id\tencoder\tstatus\t\tname\t\tsize (mm)\tmodes\tencoders\n");
	for (i = 0; i < dev->resources->count_connectors; i++) {
		struct connector *_connector = &dev->resources->connectors[i];
		drmModeConnector *connector = _connector->connector;
		if (!connector)
			continue;

		printf("%d\t%d\t%s\t%-15s\t%dx%d\t\t%d\t",
		       connector->connector_id,
		       connector->encoder_id,
		       util_lookup_connector_status_name(connector->connection),
		       _connector->name,
		       connector->mmWidth, connector->mmHeight,
		       connector->count_modes);

		for (j = 0; j < connector->count_encoders; j++)
			printf("%s%d", j > 0 ? ", " : "", connector->encoders[j]);
		printf("\n");

		if (connector->count_modes) {
			printf("  modes:\n");
			printf("\tindex name refresh (Hz) hdisp hss hse htot vdisp "
			       "vss vse vtot\n");
			for (j = 0; j < connector->count_modes; j++)
				dump_mode(&connector->modes[j], j);
		}

		if (_connector->props) {
			printf("  props:\n");
			for (j = 0; j < (int)_connector->props->count_props; j++)
				dump_prop(dev, _connector->props_info[j],
					  _connector->props->props[j],
					  _connector->props->prop_values[j]);
		}
	}
	printf("\n");
}

static void dump_crtcs(struct device *dev)
{
	int i;
	uint32_t j;

	printf("CRTCs:\n");
	printf("id\tfb\tpos\tsize\n");
	for (i = 0; i < dev->resources->count_crtcs; i++) {
		struct crtc *_crtc = &dev->resources->crtcs[i];
		drmModeCrtc *crtc = _crtc->crtc;
		if (!crtc)
			continue;

		printf("%d\t%d\t(%d,%d)\t(%dx%d)\n",
		       crtc->crtc_id,
		       crtc->buffer_id,
		       crtc->x, crtc->y,
		       crtc->width, crtc->height);
		dump_mode(&crtc->mode, 0);

		if (_crtc->props) {
			printf("  props:\n");
			for (j = 0; j < _crtc->props->count_props; j++)
				dump_prop(dev, _crtc->props_info[j],
					  _crtc->props->props[j],
					  _crtc->props->prop_values[j]);
		} else {
			printf("  no properties found\n");
		}
	}
	printf("\n");
}

static void dump_framebuffers(struct device *dev)
{
	drmModeFB *fb;
	int i;

	printf("Frame buffers:\n");
	printf("id\tsize\tpitch\n");
	for (i = 0; i < dev->resources->count_fbs; i++) {
		fb = dev->resources->fbs[i].fb;
		if (!fb)
			continue;

		printf("%u\t(%ux%u)\t%u\n",
		       fb->fb_id,
		       fb->width, fb->height,
		       fb->pitch);
	}
	printf("\n");
}

static void dump_planes(struct device *dev)
{
	unsigned int i, j;

	printf("Planes:\n");
	printf("id\tcrtc\tfb\tCRTC x,y\tx,y\tgamma size\tpossible crtcs\n");

	for (i = 0; i < dev->resources->count_planes; i++) {
		struct plane *plane = &dev->resources->planes[i];
		drmModePlane *ovr = plane->plane;
		if (!ovr)
			continue;

		printf("%d\t%d\t%d\t%d,%d\t\t%d,%d\t%-8d\t0x%08x\n",
		       ovr->plane_id, ovr->crtc_id, ovr->fb_id,
		       ovr->crtc_x, ovr->crtc_y, ovr->x, ovr->y,
		       ovr->gamma_size, ovr->possible_crtcs);

		if (!ovr->count_formats)
			continue;

		printf("  formats:");
		for (j = 0; j < ovr->count_formats; j++)
			dump_fourcc(ovr->formats[j]);
		printf("\n");

		if (plane->props) {
			printf("  props:\n");
			for (j = 0; j < plane->props->count_props; j++)
				dump_prop(dev, plane->props_info[j],
					  plane->props->props[j],
					  plane->props->prop_values[j]);
		} else {
			printf("  no properties found\n");
		}
	}
	printf("\n");

	return;
}

static void free_resources(struct resources *res)
{
	int i;

	if (!res)
		return;

#define free_resource(_res, type, Type)					\
	do {									\
		if (!(_res)->type##s)						\
			break;							\
		for (i = 0; i < (int)(_res)->count_##type##s; ++i) {	\
			if (!(_res)->type##s[i].type)				\
				break;						\
			drmModeFree##Type((_res)->type##s[i].type);		\
		}								\
		free((_res)->type##s);						\
	} while (0)

#define free_properties(_res, type)					\
	do {									\
		for (i = 0; i < (int)(_res)->count_##type##s; ++i) {	\
			unsigned int j;										\
			for (j = 0; j < res->type##s[i].props->count_props; ++j)\
				drmModeFreeProperty(res->type##s[i].props_info[j]);\
			free(res->type##s[i].props_info);			\
			drmModeFreeObjectProperties(res->type##s[i].props);	\
		}								\
	} while (0)

	free_properties(res, plane);
	free_resource(res, plane, Plane);

	free_properties(res, connector);
	free_properties(res, crtc);

	for (i = 0; i < res->count_connectors; i++)
		free(res->connectors[i].name);

	free_resource(res, fb, FB);
	free_resource(res, connector, Connector);
	free_resource(res, encoder, Encoder);
	free_resource(res, crtc, Crtc);

	free(res);
}

static struct resources *get_resources(struct device *dev)
{
	drmModeRes *_res;
	drmModePlaneRes *plane_res;
	struct resources *res;
	int i;

	res = calloc(1, sizeof(*res));
	if (res == 0)
		return NULL;

	drmSetClientCap(dev->fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);

	_res = drmModeGetResources(dev->fd);
	if (!_res) {
		fprintf(stderr, "drmModeGetResources failed: %s\n",
			strerror(errno));
		free(res);
		return NULL;
	}

	res->count_crtcs = _res->count_crtcs;
	res->count_encoders = _res->count_encoders;
	res->count_connectors = _res->count_connectors;
	res->count_fbs = _res->count_fbs;

	res->crtcs = calloc(res->count_crtcs, sizeof(*res->crtcs));
	res->encoders = calloc(res->count_encoders, sizeof(*res->encoders));
	res->connectors = calloc(res->count_connectors, sizeof(*res->connectors));
	res->fbs = calloc(res->count_fbs, sizeof(*res->fbs));

	if (!res->crtcs || !res->encoders || !res->connectors || !res->fbs) {
	    drmModeFreeResources(_res);
		goto error;
    }

#define get_resource(_res, __res, type, Type)					\
	do {									\
		for (i = 0; i < (int)(_res)->count_##type##s; ++i) {	\
			uint32_t type##id = (__res)->type##s[i];			\
			(_res)->type##s[i].type =							\
				drmModeGet##Type(dev->fd, type##id);			\
			if (!(_res)->type##s[i].type)						\
				fprintf(stderr, "could not get %s %i: %s\n",	\
					#type, type##id,							\
					strerror(errno));			\
		}								\
	} while (0)

	get_resource(res, _res, crtc, Crtc);
	get_resource(res, _res, encoder, Encoder);
	get_resource(res, _res, connector, Connector);
	get_resource(res, _res, fb, FB);

	drmModeFreeResources(_res);

	/* Set the name of all connectors based on the type name and the per-type ID. */
	for (i = 0; i < res->count_connectors; i++) {
		struct connector *connector = &res->connectors[i];
		drmModeConnector *conn = connector->connector;
		int num;

		num = asprintf(&connector->name, "%s-%u",
			 drmModeGetConnectorTypeName(conn->connector_type),
			 conn->connector_type_id);
		if (num < 0)
			goto error;
	}

#define get_properties(_res, type, Type)					\
	do {									\
		for (i = 0; i < (int)(_res)->count_##type##s; ++i) {	\
			struct type *obj = &res->type##s[i];			\
			unsigned int j;						\
			obj->props =						\
				drmModeObjectGetProperties(dev->fd, obj->type->type##_id, \
							   DRM_MODE_OBJECT_##Type); \
			if (!obj->props) {					\
				fprintf(stderr,					\
					"could not get %s %i properties: %s\n", \
					#type, obj->type->type##_id,		\
					strerror(errno));			\
				continue;					\
			}							\
			obj->props_info = calloc(obj->props->count_props,	\
						 sizeof(*obj->props_info));	\
			if (!obj->props_info)					\
				continue;					\
			for (j = 0; j < obj->props->count_props; ++j)		\
				obj->props_info[j] =				\
					drmModeGetProperty(dev->fd, obj->props->props[j]); \
		}								\
	} while (0)

	get_properties(res, crtc, CRTC);
	get_properties(res, connector, CONNECTOR);

	for (i = 0; i < res->count_crtcs; ++i)
		res->crtcs[i].mode = &res->crtcs[i].crtc->mode;

	plane_res = drmModeGetPlaneResources(dev->fd);
	if (!plane_res) {
		fprintf(stderr, "drmModeGetPlaneResources failed: %s\n",
			strerror(errno));
		return res;
	}

	res->count_planes = plane_res->count_planes;

	res->planes = calloc(res->count_planes, sizeof(*res->planes));
	if (!res->planes) {
		drmModeFreePlaneResources(plane_res);
		goto error;
	}

	get_resource(res, plane_res, plane, Plane);
	drmModeFreePlaneResources(plane_res);
	get_properties(res, plane, PLANE);

	return res;

error:
	free_resources(res);
	return NULL;
}

static struct crtc *get_crtc_by_id(struct device *dev, uint32_t id)
{
	int i;

	for (i = 0; i < dev->resources->count_crtcs; ++i) {
		drmModeCrtc *crtc = dev->resources->crtcs[i].crtc;
		if (crtc && crtc->crtc_id == id)
			return &dev->resources->crtcs[i];
	}

	return NULL;
}

static uint32_t get_crtc_mask(struct device *dev, struct crtc *crtc)
{
	unsigned int i;

	for (i = 0; i < (unsigned int)dev->resources->count_crtcs; i++) {
		if (crtc->crtc->crtc_id == dev->resources->crtcs[i].crtc->crtc_id)
			return 1 << i;
	}
    /* Unreachable: crtc->crtc is one of resources->crtcs[] */
    /* Don't return zero or static analysers will complain */
	abort();
	return 0;
}

static drmModeConnector *get_connector_by_name(struct device *dev, const char *name)
{
	struct connector *connector;
	int i;

	for (i = 0; i < dev->resources->count_connectors; i++) {
		connector = &dev->resources->connectors[i];

		if (strcmp(connector->name, name) == 0)
			return connector->connector;
	}

	return NULL;
}

static drmModeConnector *get_connector_by_id(struct device *dev, uint32_t id)
{
	drmModeConnector *connector;
	int i;

	for (i = 0; i < dev->resources->count_connectors; i++) {
		connector = dev->resources->connectors[i].connector;
		if (connector && connector->connector_id == id)
			return connector;
	}

	return NULL;
}

static drmModeEncoder *get_encoder_by_id(struct device *dev, uint32_t id)
{
	drmModeEncoder *encoder;
	int i;

	for (i = 0; i < dev->resources->count_encoders; i++) {
		encoder = dev->resources->encoders[i].encoder;
		if (encoder && encoder->encoder_id == id)
			return encoder;
	}

	return NULL;
}

/* -----------------------------------------------------------------------------
 * Pipes and planes
 */

/*
 * Mode setting with the kernel interfaces is a bit of a chore.
 * First you have to find the connector in question and make sure the
 * requested mode is available.
 * Then you need to find the encoder attached to that connector so you
 * can bind it with a free crtc.
 */
struct pipe_arg {
	const char **cons;
	uint32_t *con_ids;
	unsigned int num_cons;
	uint32_t crtc_id;
	char mode_str[64];
	char format_str[8]; /* need to leave room for "_BE" and terminating \0 */
	float vrefresh;
	unsigned int fourcc;
	drmModeModeInfo *mode;
	struct crtc *crtc;
	unsigned int fb_id[2], current_fb_id;
	struct timeval start;
	unsigned int out_fb_id;
	struct bo *out_bo;

	int swap_count;
};

struct plane_arg {
	uint32_t plane_id;  /* the id of plane to use */
	uint32_t crtc_id;  /* the id of CRTC to bind to */
	bool has_position;
	int32_t x, y;
	uint32_t w, h;
	double scale;
	unsigned int fb_id;
	unsigned int old_fb_id;
	struct bo *bo;
	struct bo *old_bo;
	char format_str[8]; /* need to leave room for "_BE" and terminating \0 */
	unsigned int fourcc;
};

static drmModeModeInfo *
connector_find_mode(struct device *dev, uint32_t con_id, const char *mode_str,
	const float vrefresh)
{
	drmModeConnector *connector;
	drmModeModeInfo *mode;
	int i;

	connector = get_connector_by_id(dev, con_id);
	if (!connector)
		return NULL;

	if (strchr(mode_str, ',')) {
		i = sscanf(mode_str, "%hu,%hu,%hu,%hu,%hu,%hu,%hu,%hu",
			     &user_mode.hdisplay, &user_mode.hsync_start,
			     &user_mode.hsync_end, &user_mode.htotal,
			     &user_mode.vdisplay, &user_mode.vsync_start,
			     &user_mode.vsync_end, &user_mode.vtotal);
		if (i == 8) {
			user_mode.clock = roundf(user_mode.htotal * user_mode.vtotal * vrefresh / 1000);
			user_mode.vrefresh = roundf(vrefresh);
			snprintf(user_mode.name, sizeof(user_mode.name), "custom%dx%d", user_mode.hdisplay, user_mode.vdisplay);

			return &user_mode;
		}
	}

	if (!connector->count_modes)
		return NULL;

	/* Pick by Index */
	if (mode_str[0] == '#') {
		int index = atoi(mode_str + 1);

		if (index >= connector->count_modes || index < 0)
			return NULL;
		return &connector->modes[index];
	}

	/* Pick by Name */
	for (i = 0; i < connector->count_modes; i++) {
		mode = &connector->modes[i];
		if (!strcmp(mode->name, mode_str)) {
			/* If the vertical refresh frequency is not specified
			 * then return the first mode that match with the name.
			 * Else, return the mode that match the name and
			 * the specified vertical refresh frequency.
			 */
			if (vrefresh == 0)
				return mode;
			else if (fabs(mode_vrefresh(mode) - vrefresh) < 0.005)
				return mode;
		}
	}

	return NULL;
}

static struct crtc *pipe_find_crtc(struct device *dev, struct pipe_arg *pipe)
{
	uint32_t possible_crtcs = ~0;
	uint32_t active_crtcs = 0;
	unsigned int crtc_idx;
	unsigned int i;
	int j;

	for (i = 0; i < pipe->num_cons; ++i) {
		uint32_t crtcs_for_connector = 0;
		drmModeConnector *connector;
		drmModeEncoder *encoder;
		struct crtc *crtc;

		connector = get_connector_by_id(dev, pipe->con_ids[i]);
		if (!connector)
			return NULL;

		for (j = 0; j < connector->count_encoders; ++j) {
			encoder = get_encoder_by_id(dev, connector->encoders[j]);
			if (!encoder)
				continue;

			crtcs_for_connector |= encoder->possible_crtcs;
			crtc = get_crtc_by_id(dev, encoder->crtc_id);
			if (!crtc)
				continue;
			active_crtcs |= get_crtc_mask(dev, crtc);
		}

		possible_crtcs &= crtcs_for_connector;
	}

	if (!possible_crtcs)
		return NULL;

	/* Return the first possible and active CRTC if one exists, or the first
	 * possible CRTC otherwise.
	 */
	if (possible_crtcs & active_crtcs)
		crtc_idx = ffs(possible_crtcs & active_crtcs);
	else
		crtc_idx = ffs(possible_crtcs);

	return &dev->resources->crtcs[crtc_idx - 1];
}

static int pipe_find_crtc_and_mode(struct device *dev, struct pipe_arg *pipe)
{
	drmModeModeInfo *mode = NULL;
	int i;

	pipe->mode = NULL;

	for (i = 0; i < (int)pipe->num_cons; i++) {
		mode = connector_find_mode(dev, pipe->con_ids[i],
					   pipe->mode_str, pipe->vrefresh);
		if (mode == NULL) {
			if (pipe->vrefresh)
				fprintf(stderr,
				"failed to find mode "
				"\"%s-%.2fHz\" for connector %s\n",
				pipe->mode_str, pipe->vrefresh, pipe->cons[i]);
			else
				fprintf(stderr,
				"failed to find mode \"%s\" for connector %s\n",
				pipe->mode_str, pipe->cons[i]);
			return -EINVAL;
		}
	}

	/* If the CRTC ID was specified, get the corresponding CRTC. Otherwise
	 * locate a CRTC that can be attached to all the connectors.
	 */
	if (pipe->crtc_id != (uint32_t)-1) {
		pipe->crtc = get_crtc_by_id(dev, pipe->crtc_id);
	} else {
		pipe->crtc = pipe_find_crtc(dev, pipe);
		pipe->crtc_id = pipe->crtc->crtc->crtc_id;
	}

	if (!pipe->crtc) {
		fprintf(stderr, "failed to find CRTC for pipe\n");
		return -EINVAL;
	}

	pipe->mode = mode;
	pipe->crtc->mode = mode;

	return 0;
}

/* -----------------------------------------------------------------------------
 * Properties
 */

struct property_arg {
	uint32_t obj_id;
	uint32_t obj_type;
	char name[DRM_PROP_NAME_LEN+1];
	uint32_t prop_id;
	uint64_t value;
	bool optional;
};

static bool set_property(struct device *dev, struct property_arg *p)
{
	drmModeObjectProperties *props = NULL;
	drmModePropertyRes **props_info = NULL;
	const char *obj_type;
	int ret;
	int i;

	p->obj_type = 0;
	p->prop_id = 0;

#define find_object(_res, type, Type)					\
	do {									\
		for (i = 0; i < (int)(_res)->count_##type##s; ++i) {	\
			struct type *obj = &(_res)->type##s[i];			\
			if (obj->type->type##_id != p->obj_id)			\
				continue;					\
			p->obj_type = DRM_MODE_OBJECT_##Type;			\
			obj_type = #Type;					\
			props = obj->props;					\
			props_info = obj->props_info;				\
		}								\
	} while(0)								\

	find_object(dev->resources, crtc, CRTC);
	if (p->obj_type == 0)
		find_object(dev->resources, connector, CONNECTOR);
	if (p->obj_type == 0)
		find_object(dev->resources, plane, PLANE);
	if (p->obj_type == 0) {
		fprintf(stderr, "Object %i not found, can't set property\n",
			p->obj_id);
		return false;
	}

	if (!props) {
		fprintf(stderr, "%s %i has no properties\n",
			obj_type, p->obj_id);
		return false;
	}

	for (i = 0; i < (int)props->count_props; ++i) {
		if (!props_info[i])
			continue;
		if (strcmp(props_info[i]->name, p->name) == 0)
			break;
	}

	if (i == (int)props->count_props) {
		if (!p->optional)
			fprintf(stderr, "%s %i has no %s property\n",
				obj_type, p->obj_id, p->name);
		return false;
	}

	p->prop_id = props->props[i];

	if (!dev->use_atomic)
		ret = drmModeObjectSetProperty(dev->fd, p->obj_id, p->obj_type,
									   p->prop_id, p->value);
	else
		ret = drmModeAtomicAddProperty(dev->req, p->obj_id, p->prop_id, p->value);

	if (ret < 0)
		fprintf(stderr, "failed to set %s %i property %s to %" PRIu64 ": %s\n",
			obj_type, p->obj_id, p->name, p->value, strerror(-ret));

	return true;
}

/* -------------------------------------------------------------------------- */

static void
page_flip_handler(int fd, unsigned int frame,
		  unsigned int sec, unsigned int usec, void *data)
{
	struct pipe_arg *pipe;
	unsigned int new_fb_id;
	struct timeval end;
	double t;

	pipe = data;
	if (pipe->current_fb_id == pipe->fb_id[0])
		new_fb_id = pipe->fb_id[1];
	else
		new_fb_id = pipe->fb_id[0];

	drmModePageFlip(fd, pipe->crtc_id, new_fb_id,
			DRM_MODE_PAGE_FLIP_EVENT, pipe);
	pipe->current_fb_id = new_fb_id;
	pipe->swap_count++;
	if (pipe->swap_count == 60) {
		gettimeofday(&end, NULL);
		t = end.tv_sec + end.tv_usec * 1e-6 -
			(pipe->start.tv_sec + pipe->start.tv_usec * 1e-6);
		fprintf(stderr, "freq: %.02fHz\n", pipe->swap_count / t);
		pipe->swap_count = 0;
		pipe->start = end;
	}
}

static bool format_support(const drmModePlanePtr ovr, uint32_t fmt)
{
	unsigned int i;

	for (i = 0; i < ovr->count_formats; ++i) {
		if (ovr->formats[i] == fmt)
			return true;
	}

	return false;
}

static void add_property(struct device *dev, uint32_t obj_id,
			       const char *name, uint64_t value)
{
	struct property_arg p;

	p.obj_id = obj_id;
	strcpy(p.name, name);
	p.value = value;

	set_property(dev, &p);
}

static bool add_property_optional(struct device *dev, uint32_t obj_id,
				  const char *name, uint64_t value)
{
	struct property_arg p;

	p.obj_id = obj_id;
	strcpy(p.name, name);
	p.value = value;
	p.optional = true;

	return set_property(dev, &p);
}

static void set_gamma(struct device *dev, unsigned crtc_id, unsigned fourcc)
{
	unsigned blob_id = 0;
	const struct util_format_info *info;
	/* TODO: support 1024-sized LUTs, when the use-case arises */
	struct drm_color_lut gamma_lut[256];
	int i, ret;

	info = util_format_info_find(fourcc);
	if (info->ncolors) {
		memset(gamma_lut, 0, sizeof(gamma_lut));
		/* TODO: Add index support for more patterns */
		util_smpte_fill_lut(info->ncolors, gamma_lut);
		drmModeCreatePropertyBlob(dev->fd, gamma_lut, sizeof(gamma_lut), &blob_id);
	} else {
		/*
		 * Initialize gamma_lut to a linear table for the legacy API below.
		 * The modern property API resets to a linear/pass-thru table if blob_id
		 * is 0, hence no PropertyBlob is created here.
		 */
		for (i = 0; i < 256; i++) {
			gamma_lut[i].red =
			gamma_lut[i].green =
			gamma_lut[i].blue = i << 8;
		}
	}

	add_property_optional(dev, crtc_id, "DEGAMMA_LUT", 0);
	add_property_optional(dev, crtc_id, "CTM", 0);
	if (!add_property_optional(dev, crtc_id, "GAMMA_LUT", blob_id)) {
		/* If we can't add the GAMMA_LUT property, try the legacy API. */
		uint16_t r[256], g[256], b[256];

		for (i = 0; i < 256; i++) {
			r[i] = gamma_lut[i].red;
			g[i] = gamma_lut[i].green;
			b[i] = gamma_lut[i].blue;
		}

		ret = drmModeCrtcSetGamma(dev->fd, crtc_id, 256, r, g, b);
		if (ret && errno != ENOSYS)
			fprintf(stderr, "failed to set gamma: %s\n", strerror(errno));
	}
}

static int
bo_fb_create(int fd, unsigned int fourcc, const uint32_t w, const uint32_t h,
             enum util_fill_pattern pat, struct bo **out_bo, unsigned int *out_fb_id)
{
	uint32_t handles[4] = {0}, pitches[4] = {0}, offsets[4] = {0};
	struct bo *bo;
	unsigned int fb_id;

	bo = bo_create(fd, fourcc, w, h, handles, pitches, offsets, pat);

	if (bo == NULL)
		return -1;

	if (drmModeAddFB2(fd, w, h, fourcc, handles, pitches, offsets, &fb_id, 0)) {
		fprintf(stderr, "failed to add fb (%ux%u): %s\n", w, h, strerror(errno));
		bo_destroy(bo);
		return -1;
	}
	*out_bo = bo;
	*out_fb_id = fb_id;
	return 0;
}

static int atomic_set_plane(struct device *dev, struct plane_arg *p,
							int pattern, bool update)
{
	struct bo *plane_bo;
	int crtc_x, crtc_y, crtc_w, crtc_h;
	struct crtc *crtc = NULL;
	unsigned int old_fb_id;

	/* Find an unused plane which can be connected to our CRTC. Find the
	 * CRTC index first, then iterate over available planes.
	 */
	crtc = get_crtc_by_id(dev, p->crtc_id);
	if (!crtc) {
		fprintf(stderr, "CRTC %u not found\n", p->crtc_id);
		return -1;
	}

	if (!update)
		fprintf(stderr, "testing %dx%d@%s on plane %u, crtc %u\n",
			p->w, p->h, p->format_str, p->plane_id, p->crtc_id);

	plane_bo = p->old_bo;
	p->old_bo = p->bo;

	if (!plane_bo) {
		if (bo_fb_create(dev->fd, p->fourcc, p->w, p->h,
                         pattern, &plane_bo, &p->fb_id))
			return -1;
	}

	p->bo = plane_bo;

	old_fb_id = p->fb_id;
	p->old_fb_id = old_fb_id;

	crtc_w = p->w * p->scale;
	crtc_h = p->h * p->scale;
	if (!p->has_position) {
		/* Default to the middle of the screen */
		crtc_x = (crtc->mode->hdisplay - crtc_w) / 2;
		crtc_y = (crtc->mode->vdisplay - crtc_h) / 2;
	} else {
		crtc_x = p->x;
		crtc_y = p->y;
	}

	add_property(dev, p->plane_id, "FB_ID", p->fb_id);
	add_property(dev, p->plane_id, "CRTC_ID", p->crtc_id);
	add_property(dev, p->plane_id, "SRC_X", 0);
	add_property(dev, p->plane_id, "SRC_Y", 0);
	add_property(dev, p->plane_id, "SRC_W", p->w << 16);
	add_property(dev, p->plane_id, "SRC_H", p->h << 16);
	add_property(dev, p->plane_id, "CRTC_X", crtc_x);
	add_property(dev, p->plane_id, "CRTC_Y", crtc_y);
	add_property(dev, p->plane_id, "CRTC_W", crtc_w);
	add_property(dev, p->plane_id, "CRTC_H", crtc_h);

	return 0;
}

static int set_plane(struct device *dev, struct plane_arg *p)
{
	drmModePlane *ovr;
	uint32_t plane_id;
	int crtc_x, crtc_y, crtc_w, crtc_h;
	struct crtc *crtc = NULL;
	unsigned int i, crtc_mask;

	/* Find an unused plane which can be connected to our CRTC. Find the
	 * CRTC index first, then iterate over available planes.
	 */
	crtc = get_crtc_by_id(dev, p->crtc_id);
	if (!crtc) {
		fprintf(stderr, "CRTC %u not found\n", p->crtc_id);
		return -1;
	}
	crtc_mask = get_crtc_mask(dev, crtc);
	plane_id = p->plane_id;

	for (i = 0; i < dev->resources->count_planes; i++) {
		ovr = dev->resources->planes[i].plane;
		if (!ovr)
			continue;

		if (plane_id && plane_id != ovr->plane_id)
			continue;

		if (!format_support(ovr, p->fourcc))
			continue;

		if ((ovr->possible_crtcs & crtc_mask) &&
		    (ovr->crtc_id == 0 || ovr->crtc_id == p->crtc_id)) {
			plane_id = ovr->plane_id;
			break;
		}
	}

	if (i == dev->resources->count_planes) {
		fprintf(stderr, "no unused plane available for CRTC %u\n",
			p->crtc_id);
		return -1;
	}

	fprintf(stderr, "testing %dx%d@%s overlay plane %u\n",
		p->w, p->h, p->format_str, plane_id);

	/* just use single plane format for now.. */
	if (bo_fb_create(dev->fd, p->fourcc, p->w, p->h,
	                 secondary_fill, &p->bo, &p->fb_id))
		return -1;

	crtc_w = p->w * p->scale;
	crtc_h = p->h * p->scale;
	if (!p->has_position) {
		/* Default to the middle of the screen */
		crtc_x = (crtc->mode->hdisplay - crtc_w) / 2;
		crtc_y = (crtc->mode->vdisplay - crtc_h) / 2;
	} else {
		crtc_x = p->x;
		crtc_y = p->y;
	}

	/* note src coords (last 4 args) are in Q16 format */
	if (drmModeSetPlane(dev->fd, plane_id, p->crtc_id, p->fb_id,
			    0, crtc_x, crtc_y, crtc_w, crtc_h,
			    0, 0, p->w << 16, p->h << 16)) {
		fprintf(stderr, "failed to enable plane: %s\n",
			strerror(errno));
		return -1;
	}

	ovr->crtc_id = p->crtc_id;

	return 0;
}

static void atomic_set_planes(struct device *dev, struct plane_arg *p,
			      unsigned int count, bool update)
{
	unsigned int i, pattern = primary_fill;

	/* set up planes */
	for (i = 0; i < count; i++) {
		if (i > 0)
			pattern = secondary_fill;
		else
			set_gamma(dev, p[i].crtc_id, p[i].fourcc);

		if (atomic_set_plane(dev, &p[i], pattern, update))
			return;
	}
}

static void
atomic_test_page_flip(struct device *dev, struct pipe_arg *pipe_args,
              struct plane_arg *plane_args, unsigned int plane_count)
{
    int ret;

	gettimeofday(&pipe_args->start, NULL);
	pipe_args->swap_count = 0;

	while (true) {
		drmModeAtomicFree(dev->req);
		dev->req = drmModeAtomicAlloc();
		atomic_set_planes(dev, plane_args, plane_count, true);

		ret = drmModeAtomicCommit(dev->fd, dev->req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
		if (ret) {
			fprintf(stderr, "Atomic Commit failed [2]\n");
			return;
		}

		pipe_args->swap_count++;
		if (pipe_args->swap_count == 60) {
			struct timeval end;
			double t;

			gettimeofday(&end, NULL);
			t = end.tv_sec + end.tv_usec * 1e-6 -
			    (pipe_args->start.tv_sec + pipe_args->start.tv_usec * 1e-6);
			fprintf(stderr, "freq: %.02fHz\n", pipe_args->swap_count / t);
			pipe_args->swap_count = 0;
			pipe_args->start = end;
		}
	}
}

static void atomic_clear_planes(struct device *dev, struct plane_arg *p, unsigned int count)
{
	unsigned int i;

	for (i = 0; i < count; i++) {
		add_property(dev, p[i].plane_id, "FB_ID", 0);
		add_property(dev, p[i].plane_id, "CRTC_ID", 0);
		add_property(dev, p[i].plane_id, "SRC_X", 0);
		add_property(dev, p[i].plane_id, "SRC_Y", 0);
		add_property(dev, p[i].plane_id, "SRC_W", 0);
		add_property(dev, p[i].plane_id, "SRC_H", 0);
		add_property(dev, p[i].plane_id, "CRTC_X", 0);
		add_property(dev, p[i].plane_id, "CRTC_Y", 0);
		add_property(dev, p[i].plane_id, "CRTC_W", 0);
		add_property(dev, p[i].plane_id, "CRTC_H", 0);
	}
}

static void atomic_clear_FB(struct device *dev, struct plane_arg *p, unsigned int count)
{
	unsigned int i;

	for (i = 0; i < count; i++) {
		if (p[i].fb_id) {
			drmModeRmFB(dev->fd, p[i].fb_id);
			p[i].fb_id = 0;
		}
		if (p[i].old_fb_id) {
			drmModeRmFB(dev->fd, p[i].old_fb_id);
			p[i].old_fb_id = 0;
		}
		if (p[i].bo) {
			bo_destroy(p[i].bo);
			p[i].bo = NULL;
		}
		if (p[i].old_bo) {
			bo_destroy(p[i].old_bo);
			p[i].old_bo = NULL;
		}

	}
}

static void clear_planes(struct device *dev, struct plane_arg *p, unsigned int count)
{
	unsigned int i;

	for (i = 0; i < count; i++) {
		if (p[i].fb_id)
			drmModeRmFB(dev->fd, p[i].fb_id);
		if (p[i].bo)
			bo_destroy(p[i].bo);
	}
}

static int pipe_resolve_connectors(struct device *dev, struct pipe_arg *pipe)
{
	drmModeConnector *connector;
	unsigned int i;
	uint32_t id;
	char *endp;

	for (i = 0; i < pipe->num_cons; i++) {
		id = strtoul(pipe->cons[i], &endp, 10);
		if (endp == pipe->cons[i]) {
			connector = get_connector_by_name(dev, pipe->cons[i]);
			if (!connector) {
				fprintf(stderr, "no connector named '%s'\n",
					pipe->cons[i]);
				return -ENODEV;
			}

			id = connector->connector_id;
		}

		pipe->con_ids[i] = id;
	}

	return 0;
}

static bool pipe_has_writeback_connector(struct device *dev, struct pipe_arg *pipes,
		unsigned int count)
{
	drmModeConnector *connector;
	unsigned int i, j;

	for (j = 0; j < count; j++) {
		struct pipe_arg *pipe = &pipes[j];

		for (i = 0; i < pipe->num_cons; i++) {
			connector = get_connector_by_id(dev, pipe->con_ids[i]);
			if (connector && connector->connector_type == DRM_MODE_CONNECTOR_WRITEBACK)
				return true;
		}
	}
	return false;
}

static int pipe_attempt_connector(struct device *dev, drmModeConnector *con,
		struct pipe_arg *pipe)
{
	char *con_str;
	int i;

	con_str = calloc(8, sizeof(char));
	if (!con_str)
		return -1;

	sprintf(con_str, "%d", con->connector_id);
	strcpy(pipe->format_str, "XR24");
	pipe->fourcc = util_format_fourcc(pipe->format_str);
	pipe->num_cons = 1;
	pipe->con_ids = calloc(1, sizeof(*pipe->con_ids));
	pipe->cons = calloc(1, sizeof(*pipe->cons));

	if (!pipe->con_ids || !pipe->cons)
		goto free_con_str;

	pipe->con_ids[0] = con->connector_id;
	pipe->cons[0] = (const char*)con_str;

	pipe->crtc = pipe_find_crtc(dev, pipe);
	if (!pipe->crtc)
		goto free_all;

	pipe->crtc_id = pipe->crtc->crtc->crtc_id;

	/* Return the first mode if no preferred. */
	pipe->mode = &con->modes[0];

	for (i = 0; i < con->count_modes; i++) {
		drmModeModeInfo *current_mode = &con->modes[i];

		if (current_mode->type & DRM_MODE_TYPE_PREFERRED) {
			pipe->mode = current_mode;
			break;
		}
	}

	sprintf(pipe->mode_str, "%dx%d", pipe->mode->hdisplay, pipe->mode->vdisplay);

	return 0;

free_all:
	free(pipe->cons);
	free(pipe->con_ids);
free_con_str:
	free(con_str);
	return -1;
}

static int pipe_find_preferred(struct device *dev, struct pipe_arg **out_pipes)
{
	struct pipe_arg *pipes;
	struct resources *res = dev->resources;
	drmModeConnector *con = NULL;
	int i, connected = 0, attempted = 0;

	for (i = 0; i < res->count_connectors; i++) {
		con = res->connectors[i].connector;
		if (!con || con->connection != DRM_MODE_CONNECTED ||
		    con->connector_type == DRM_MODE_CONNECTOR_WRITEBACK)
			continue;
		connected++;
	}
	if (!connected) {
		printf("no connected connector!\n");
		return 0;
	}

	pipes = calloc(connected, sizeof(struct pipe_arg));
	if (!pipes)
		return 0;

	for (i = 0; i < res->count_connectors && attempted < connected; i++) {
		con = res->connectors[i].connector;
		if (!con || con->connection != DRM_MODE_CONNECTED)
			continue;

		if (pipe_attempt_connector(dev, con, &pipes[attempted]) < 0) {
			printf("failed fetching preferred mode for connector\n");
			continue;
		}
		attempted++;
	}

	*out_pipes = pipes;
	return attempted;
}

static struct plane *get_primary_plane_by_crtc(struct device *dev, struct crtc *crtc)
{
	unsigned int i;

	for (i = 0; i < dev->resources->count_planes; i++) {
		struct plane *plane = &dev->resources->planes[i];
		drmModePlane *ovr = plane->plane;
		if (!ovr)
			continue;

		// XXX: add is_primary_plane and (?) format checks

		if (ovr->possible_crtcs & get_crtc_mask(dev, crtc))
            return plane;
	}
	return NULL;
}

static unsigned int set_mode(struct device *dev, struct pipe_arg **pipe_args, unsigned int count)
{
	unsigned int i, j;
	int ret, x = 0;
	int preferred = count == 0;
	struct pipe_arg *pipes;

	if (preferred) {
		count = pipe_find_preferred(dev, pipe_args);
		if (!count) {
			fprintf(stderr, "can't find any preferred connector/mode.\n");
			return 0;
		}

		pipes = *pipe_args;
	} else {
		pipes = *pipe_args;

		for (i = 0; i < count; i++) {
			struct pipe_arg *pipe = &pipes[i];

			ret = pipe_resolve_connectors(dev, pipe);
			if (ret < 0)
				return 0;

			ret = pipe_find_crtc_and_mode(dev, pipe);
			if (ret < 0)
				continue;
		}
	}

	if (!dev->use_atomic) {
		for (i = 0; i < count; i++) {
			struct pipe_arg *pipe = &pipes[i];

			if (pipe->mode == NULL)
				continue;

			if (!preferred) {
				dev->mode.width += pipe->mode->hdisplay;
				if (dev->mode.height < pipe->mode->vdisplay)
					dev->mode.height = pipe->mode->vdisplay;
			} else {
				/* XXX: Use a clone mode, more like atomic. We could do per
				 * connector bo/fb, so we don't have the stretched image.
				 */
				if (dev->mode.width < pipe->mode->hdisplay)
					dev->mode.width = pipe->mode->hdisplay;
				if (dev->mode.height < pipe->mode->vdisplay)
					dev->mode.height = pipe->mode->vdisplay;
			}
		}

		if (bo_fb_create(dev->fd, pipes[0].fourcc, dev->mode.width, dev->mode.height,
			             primary_fill, &dev->mode.bo, &dev->mode.fb_id))
			return 0;
	}

	for (i = 0; i < count; i++) {
		struct pipe_arg *pipe = &pipes[i];
		uint32_t blob_id;

		if (pipe->mode == NULL)
			continue;

		printf("setting mode %s-%.2fHz on connectors ",
		       pipe->mode->name, mode_vrefresh(pipe->mode));
		for (j = 0; j < pipe->num_cons; ++j) {
			printf("%s, ", pipe->cons[j]);
			if (dev->use_atomic)
				add_property(dev, pipe->con_ids[j], "CRTC_ID", pipe->crtc_id);
		}
		printf("crtc %d\n", pipe->crtc_id);

		if (!dev->use_atomic) {
			ret = drmModeSetCrtc(dev->fd, pipe->crtc_id, dev->mode.fb_id,
								 x, 0, pipe->con_ids, pipe->num_cons,
								 pipe->mode);

			/* XXX: Actually check if this is needed */
			drmModeDirtyFB(dev->fd, dev->mode.fb_id, NULL, 0);

			if (!preferred)
				x += pipe->mode->hdisplay;

			if (ret) {
				fprintf(stderr, "failed to set mode: %s\n", strerror(errno));
				return 0;
			}

			set_gamma(dev, pipe->crtc_id, pipe->fourcc);
		} else {
			drmModeCreatePropertyBlob(dev->fd, pipe->mode, sizeof(*pipe->mode), &blob_id);
			add_property(dev, pipe->crtc_id, "MODE_ID", blob_id);
			add_property(dev, pipe->crtc_id, "ACTIVE", 1);

			/* By default atomic modeset does not set a primary plane, shrug */
			if (preferred) {
				struct plane *plane = get_primary_plane_by_crtc(dev, pipe->crtc);
				struct plane_arg plane_args = {
					.plane_id = plane->plane->plane_id,
					.crtc_id = pipe->crtc_id,
					.w = pipe->mode->hdisplay,
					.h = pipe->mode->vdisplay,
					.scale = 1.0,
					.format_str = "XR24",
					.fourcc = util_format_fourcc(pipe->format_str),
				};

				atomic_set_planes(dev, &plane_args, 1, false);
			}
		}
	}

	return count;
}

static void writeback_config(struct device *dev, struct pipe_arg *pipes, unsigned int count)
{
	drmModeConnector *connector;
	unsigned int i, j;

	for (j = 0; j < count; j++) {
		struct pipe_arg *pipe = &pipes[j];

		for (i = 0; i < pipe->num_cons; i++) {
			connector = get_connector_by_id(dev, pipe->con_ids[i]);
			if (connector->connector_type == DRM_MODE_CONNECTOR_WRITEBACK) {
				if (!pipe->mode) {
					fprintf(stderr, "no mode for writeback\n");
					return;
				}
				bo_fb_create(dev->fd, pipes[j].fourcc,
					     pipe->mode->hdisplay, pipe->mode->vdisplay,
					     UTIL_PATTERN_PLAIN,
					     &pipe->out_bo, &pipe->out_fb_id);
				add_property(dev, pipe->con_ids[i], "WRITEBACK_FB_ID",
					     pipe->out_fb_id);
				add_property(dev, pipe->con_ids[i], "WRITEBACK_OUT_FENCE_PTR",
					     (uintptr_t)(&dev->writeback_fence_fd));
			}
		}
	}
}

static int poll_writeback_fence(int fd, int timeout)
{
	struct pollfd fds = { fd, POLLIN };
	int ret;

	do {
		ret = poll(&fds, 1, timeout);
		if (ret > 0) {
			if (fds.revents & (POLLERR | POLLNVAL))
				return -EINVAL;

			return 0;
		} else if (ret == 0) {
			return -ETIMEDOUT;
		} else {
			ret = -errno;
			if (ret == -EINTR || ret == -EAGAIN)
				continue;
			return ret;
		}
	} while (1);

}

static void dump_output_fb(struct device *dev, struct pipe_arg *pipes, char *dump_path,
			   unsigned int count)
{
	drmModeConnector *connector;
	unsigned int i, j;

	for (j = 0; j < count; j++) {
		struct pipe_arg *pipe = &pipes[j];

		for (i = 0; i < pipe->num_cons; i++) {
			connector = get_connector_by_id(dev, pipe->con_ids[i]);
			if (connector->connector_type == DRM_MODE_CONNECTOR_WRITEBACK)
				bo_dump(pipe->out_bo, dump_path);
		}
	}
}

static void atomic_clear_mode(struct device *dev, struct pipe_arg *pipes, unsigned int count)
{
	unsigned int i;
	unsigned int j;

	for (i = 0; i < count; i++) {
		struct pipe_arg *pipe = &pipes[i];

		if (pipe->mode == NULL)
			continue;

		for (j = 0; j < pipe->num_cons; ++j)
			add_property(dev, pipe->con_ids[j], "CRTC_ID",0);

		add_property(dev, pipe->crtc_id, "MODE_ID", 0);
		add_property(dev, pipe->crtc_id, "ACTIVE", 0);
	}
}

static void clear_mode(struct device *dev)
{
	if (dev->mode.fb_id)
		drmModeRmFB(dev->fd, dev->mode.fb_id);
	if (dev->mode.bo)
		bo_destroy(dev->mode.bo);
}

static void set_planes(struct device *dev, struct plane_arg *p, unsigned int count)
{
	unsigned int i;

	/* set up planes/overlays */
	for (i = 0; i < count; i++)
		if (set_plane(dev, &p[i]))
			return;
}

static void set_cursors(struct device *dev, struct pipe_arg *pipes, unsigned int count)
{
	uint32_t handles[4] = {0}, pitches[4] = {0}, offsets[4] = {0};
	uint32_t cw = 64;
	uint32_t ch = 64;
	struct bo *bo;
	uint64_t value;
	unsigned int i;
	int ret;

	ret = drmGetCap(dev->fd, DRM_CAP_CURSOR_WIDTH, &value);
	if (!ret)
		cw = value;

	ret = drmGetCap(dev->fd, DRM_CAP_CURSOR_HEIGHT, &value);
	if (!ret)
		ch = value;


	/* create cursor bo.. just using PATTERN_PLAIN as it has
	 * translucent alpha
	 */
	bo = bo_create(dev->fd, DRM_FORMAT_ARGB8888, cw, ch, handles, pitches,
		       offsets, UTIL_PATTERN_PLAIN);
	if (bo == NULL)
		return;

	dev->mode.cursor_bo = bo;

	for (i = 0; i < count; i++) {
		struct pipe_arg *pipe = &pipes[i];
		ret = cursor_init(dev->fd, handles[0],
				pipe->crtc_id,
				pipe->mode->hdisplay, pipe->mode->vdisplay,
				cw, ch);
		if (ret) {
			fprintf(stderr, "failed to init cursor for CRTC[%u]\n",
					pipe->crtc_id);
			return;
		}
	}

	cursor_start();
}

static void clear_cursors(struct device *dev)
{
	cursor_stop();

	if (dev->mode.cursor_bo)
		bo_destroy(dev->mode.cursor_bo);
}

static void test_page_flip(struct device *dev, struct pipe_arg *pipes, unsigned int count)
{
	unsigned int other_fb_id;
	struct bo *other_bo;
	drmEventContext evctx;
	unsigned int i;
	int ret;

	if (bo_fb_create(dev->fd, pipes[0].fourcc, dev->mode.width, dev->mode.height,
	                 UTIL_PATTERN_PLAIN, &other_bo, &other_fb_id))
		return;

	for (i = 0; i < count; i++) {
		struct pipe_arg *pipe = &pipes[i];

		if (pipe->mode == NULL)
			continue;

		ret = drmModePageFlip(dev->fd, pipe->crtc_id,
				      other_fb_id, DRM_MODE_PAGE_FLIP_EVENT,
				      pipe);
		if (ret) {
			fprintf(stderr, "failed to page flip: %s\n", strerror(errno));
			goto err_rmfb;
		}
		gettimeofday(&pipe->start, NULL);
		pipe->swap_count = 0;
		pipe->fb_id[0] = dev->mode.fb_id;
		pipe->fb_id[1] = other_fb_id;
		pipe->current_fb_id = other_fb_id;
	}

	memset(&evctx, 0, sizeof evctx);
	evctx.version = DRM_EVENT_CONTEXT_VERSION;
	evctx.vblank_handler = NULL;
	evctx.page_flip_handler = page_flip_handler;

	while (1) {
#if 0
		struct pollfd pfd[2];

		pfd[0].fd = 0;
		pfd[0].events = POLLIN;
		pfd[1].fd = fd;
		pfd[1].events = POLLIN;

		if (poll(pfd, 2, -1) < 0) {
			fprintf(stderr, "poll error\n");
			break;
		}

		if (pfd[0].revents)
			break;
#else
		struct timeval timeout = { .tv_sec = 3, .tv_usec = 0 };
		fd_set fds;

		FD_ZERO(&fds);
		FD_SET(0, &fds);
		FD_SET(dev->fd, &fds);
		ret = select(dev->fd + 1, &fds, NULL, NULL, &timeout);

		if (ret <= 0) {
			fprintf(stderr, "select timed out or error (ret %d)\n",
				ret);
			continue;
		} else if (FD_ISSET(0, &fds)) {
			break;
		}
#endif

		drmHandleEvent(dev->fd, &evctx);
	}

err_rmfb:
	drmModeRmFB(dev->fd, other_fb_id);
	bo_destroy(other_bo);
}

#define min(a, b)	((a) < (b) ? (a) : (b))

static int parse_connector(struct pipe_arg *pipe, const char *arg)
{
	unsigned int len;
	unsigned int i;
	const char *p;
	char *endp;

	pipe->vrefresh = 0;
	pipe->crtc_id = (uint32_t)-1;
	strcpy(pipe->format_str, "XR24");

	/* Count the number of connectors and allocate them. */
	pipe->num_cons = 1;
	for (p = arg; *p && *p != ':' && *p != '@'; ++p) {
		if (*p == ',')
			pipe->num_cons++;
	}

	pipe->con_ids = calloc(pipe->num_cons, sizeof(*pipe->con_ids));
	pipe->cons = calloc(pipe->num_cons, sizeof(*pipe->cons));
	if (pipe->con_ids == NULL || pipe->cons == NULL)
		return -1;

	/* Parse the connectors. */
	for (i = 0, p = arg; i < pipe->num_cons; ++i, p = endp + 1) {
		endp = strpbrk(p, ",@:");
		if (!endp)
			break;

		pipe->cons[i] = strndup(p, endp - p);

		if (*endp != ',')
			break;
	}

	if (i != pipe->num_cons - 1)
		return -1;

	/* Parse the remaining parameters. */
	if (!endp)
		return -1;
	if (*endp == '@') {
		arg = endp + 1;
		pipe->crtc_id = strtoul(arg, &endp, 10);
	}
	if (*endp != ':')
		return -1;

	arg = endp + 1;

	/* Search for the vertical refresh or the format. */
	p = strpbrk(arg, "-@");
	if (p == NULL)
		p = arg + strlen(arg);
	len = min(sizeof pipe->mode_str - 1, (unsigned int)(p - arg));
	strncpy(pipe->mode_str, arg, len);
	pipe->mode_str[len] = '\0';

	if (*p == '-') {
		pipe->vrefresh = strtof(p + 1, &endp);
		p = endp;
	}

	if (*p == '@') {
		len = sizeof(pipe->format_str) - 1;
		strncpy(pipe->format_str, p + 1, len);
		pipe->format_str[len] = '\0';
	}

	pipe->fourcc = util_format_fourcc(pipe->format_str);
	if (pipe->fourcc == 0)  {
		fprintf(stderr, "unknown format %s\n", pipe->format_str);
		return -1;
	}

	return 0;
}

static int parse_plane(struct plane_arg *plane, const char *p)
{
	unsigned int len;
	char *end;

	plane->plane_id = strtoul(p, &end, 10);
	if (*end != '@')
		return -EINVAL;

	p = end + 1;
	plane->crtc_id = strtoul(p, &end, 10);
	if (*end != ':')
		return -EINVAL;

	p = end + 1;
	plane->w = strtoul(p, &end, 10);
	if (*end != 'x')
		return -EINVAL;

	p = end + 1;
	plane->h = strtoul(p, &end, 10);

	if (*end == '+' || *end == '-') {
		plane->x = strtol(end, &end, 10);
		if (*end != '+' && *end != '-')
			return -EINVAL;
		plane->y = strtol(end, &end, 10);

		plane->has_position = true;
	}

	if (*end == '*') {
		p = end + 1;
		plane->scale = strtod(p, &end);
		if (plane->scale <= 0.0)
			return -EINVAL;
	} else {
		plane->scale = 1.0;
	}

	if (*end == '@') {
		len = sizeof(plane->format_str) - 1;
		strncpy(plane->format_str, end + 1, len);
		plane->format_str[len] = '\0';
	} else {
		strcpy(plane->format_str, "XR24");
	}

	plane->fourcc = util_format_fourcc(plane->format_str);
	if (plane->fourcc == 0) {
		fprintf(stderr, "unknown format %s\n", plane->format_str);
		return -EINVAL;
	}

	return 0;
}

static int parse_property(struct property_arg *p, const char *arg)
{
	if (sscanf(arg, "%d:%32[^:]:%" SCNu64, &p->obj_id, p->name, &p->value) != 3)
		return -1;

	p->obj_type = 0;
	p->name[DRM_PROP_NAME_LEN] = '\0';

	return 0;
}

static void parse_fill_patterns(char *arg)
{
	char *fill = strtok(arg, ",");
	if (!fill)
		return;
	primary_fill = util_pattern_enum(fill);
	fill = strtok(NULL, ",");
	if (!fill)
		return;
	secondary_fill = util_pattern_enum(fill);
}

static void usage(char *name)
{
	fprintf(stderr, "usage: %s [-acDdefMoPpsCvrw]\n", name);

	fprintf(stderr, "\n Query options:\n\n");
	fprintf(stderr, "\t-c\tlist connectors\n");
	fprintf(stderr, "\t-e\tlist encoders\n");
	fprintf(stderr, "\t-f\tlist framebuffers\n");
	fprintf(stderr, "\t-p\tlist CRTCs and planes (pipes)\n");

	fprintf(stderr, "\n Test options:\n\n");
	fprintf(stderr, "\t-P <plane_id>@<crtc_id>:<w>x<h>[+<x>+<y>][*<scale>][@<format>]\tset a plane, see 'plane-topology'\n");
	fprintf(stderr, "\t-s <connector_id>[,<connector_id>][@<crtc_id>]:mode[@<format>]\tset a mode, see 'mode-topology'\n");
	fprintf(stderr, "\t\twhere mode can be specified as:\n");
	fprintf(stderr, "\t\t<hdisp>x<vdisp>[-<vrefresh>]\n");
	fprintf(stderr, "\t\t<hdisp>,<hss>,<hse>,<htot>,<vdisp>,<vss>,<vse>,<vtot>-<vrefresh>\n");
	fprintf(stderr, "\t\t#<mode index>\n");
	fprintf(stderr, "\t-C\ttest hw cursor\n");
	fprintf(stderr, "\t-v\ttest vsynced page flipping\n");
	fprintf(stderr, "\t-r\tset the preferred mode for all connectors\n");
	fprintf(stderr, "\t-w <obj_id>:<prop_name>:<value>\tset property, see 'property'\n");
	fprintf(stderr, "\t-a \tuse atomic API\n");
	fprintf(stderr, "\t-F pattern1,pattern2\tspecify fill patterns\n");
	fprintf(stderr, "\t-o <desired file path> \t Dump writeback output buffer to file\n");

	fprintf(stderr, "\n Generic options:\n\n");
	fprintf(stderr, "\t-d\tdrop master after mode set\n");
	fprintf(stderr, "\t-M module\tuse the given driver\n");
	fprintf(stderr, "\t-D device\tuse the given device\n");

	fprintf(stderr, "\n\tDefault is to dump all info.\n");

	fprintf(stderr, "\n");
	fprintf(stderr, "Plane Topology is defined as:\n");
	fprintf(stderr, "\tplane-topology\t::= plane-id '@' crtc-id ':' width 'x' height ( <plane-offsets> )? ;\n");
	fprintf(stderr, "\tplane-offsets\t::= '+' x-offset '+' y-offset ( <plane-scale> )? ;\n");
	fprintf(stderr, "\tplane-scale\t::= '*' scale ( <plane-format> )? ;\n");
	fprintf(stderr, "\tplane-format\t::= '@' format ;\n");

	fprintf(stderr, "\n");
	fprintf(stderr, "Mode Topology is defined as:\n");
	fprintf(stderr, "\tmode-topology\t::= connector-id ( ',' connector-id )* ( '@' crtc-id )? ':' <mode-selection> ( '@' format )? ;\n");
	fprintf(stderr, "\tmode-selection\t::=  <indexed-mode> | <named-mode> | <custom-mode> ;\n");
	fprintf(stderr, "\tindexed-mode\t::=  '#' mode-index ;\n");
	fprintf(stderr, "\tnamed-mode\t::=  width 'x' height ( '-' vrefresh )? ;\n");
	fprintf(stderr, "\tcustom-mode\t::=  hdisplay ',' hsyncstart ',' hsyncend ',' htotal ',' vdisplay ',' vsyncstart ',' vsyncend ',' vtotal '-' vrefresh ;\n");

	fprintf(stderr, "\n");
	fprintf(stderr, "Property is defined as:\n");
	fprintf(stderr, "\tproperty\t::= object-id ':' property-name ':' value ;\n");
	exit(0);
}

static char optstr[] = "acdD:efF:M:P:ps:Cvrw:o:";

int main(int argc, char **argv)
{
	struct device dev;

	int c;
	int encoders = 0, connectors = 0, crtcs = 0, planes = 0, framebuffers = 0;
	int drop_master = 0;
	int test_vsync = 0;
	int test_cursor = 0;
	int set_preferred = 0;
	int use_atomic = 0;
	char *device = NULL;
	char *module = NULL;
	unsigned int i;
	unsigned int count = 0, plane_count = 0;
	unsigned int prop_count = 0;
	struct pipe_arg *pipe_args = NULL;
	struct plane_arg *plane_args = NULL;
	struct property_arg *prop_args = NULL;
	unsigned int args = 0;
	int ret;
	char *dump_path = NULL;

	memset(&dev, 0, sizeof dev);

	opterr = 0;
	while ((c = getopt(argc, argv, optstr)) != -1) {
		args++;

		switch (c) {
		case 'a':
			use_atomic = 1;
			/* Preserve the default behaviour of dumping all information. */
			args--;
			break;
		case 'c':
			connectors = 1;
			break;
		case 'D':
			device = optarg;
			/* Preserve the default behaviour of dumping all information. */
			args--;
			break;
		case 'd':
			drop_master = 1;
			break;
		case 'e':
			encoders = 1;
			break;
		case 'f':
			framebuffers = 1;
			break;
		case 'F':
			parse_fill_patterns(optarg);
			break;
		case 'M':
			module = optarg;
			/* Preserve the default behaviour of dumping all information. */
			args--;
			break;
		case 'o':
			dump_path = optarg;
			break;
		case 'P':
			plane_args = realloc(plane_args,
					     (plane_count + 1) * sizeof *plane_args);
			if (plane_args == NULL) {
				fprintf(stderr, "memory allocation failed\n");
				return 1;
			}
			memset(&plane_args[plane_count], 0, sizeof(*plane_args));

			if (parse_plane(&plane_args[plane_count], optarg) < 0)
				usage(argv[0]);

			plane_count++;
			break;
		case 'p':
			crtcs = 1;
			planes = 1;
			break;
		case 's':
			pipe_args = realloc(pipe_args,
					    (count + 1) * sizeof *pipe_args);
			if (pipe_args == NULL) {
				fprintf(stderr, "memory allocation failed\n");
				return 1;
			}
			memset(&pipe_args[count], 0, sizeof(*pipe_args));

			if (parse_connector(&pipe_args[count], optarg) < 0)
				usage(argv[0]);

			count++;
			break;
		case 'C':
			test_cursor = 1;
			break;
		case 'v':
			test_vsync = 1;
			break;
		case 'r':
			set_preferred = 1;
			break;
		case 'w':
			prop_args = realloc(prop_args,
					   (prop_count + 1) * sizeof *prop_args);
			if (prop_args == NULL) {
				fprintf(stderr, "memory allocation failed\n");
				return 1;
			}
			memset(&prop_args[prop_count], 0, sizeof(*prop_args));

			if (parse_property(&prop_args[prop_count], optarg) < 0)
				usage(argv[0]);

			prop_count++;
			break;
		default:
			usage(argv[0]);
			break;
		}
	}

	/* Dump all the details when no* arguments are provided. */
	if (!args)
		encoders = connectors = crtcs = planes = framebuffers = 1;

	if (test_vsync && !count && !set_preferred) {
		fprintf(stderr, "page flipping requires at least one -s or -r option.\n");
		return -1;
	}
	if (set_preferred && count) {
		fprintf(stderr, "cannot use -r (preferred) when -s (mode) is set\n");
		return -1;
	}

	dev.fd = util_open(device, module);
	if (dev.fd < 0)
		return -1;

	if (use_atomic) {
		ret = drmSetClientCap(dev.fd, DRM_CLIENT_CAP_ATOMIC, 1);
		drmSetClientCap(dev.fd, DRM_CLIENT_CAP_WRITEBACK_CONNECTORS, 1);
		if (ret) {
			fprintf(stderr, "no atomic modesetting support: %s\n", strerror(errno));
			drmClose(dev.fd);
			return -1;
		}
	}

	dev.use_atomic = use_atomic;

	dev.resources = get_resources(&dev);
	if (!dev.resources) {
		drmClose(dev.fd);
		return 1;
	}

#define dump_resource(dev, res) if (res) dump_##res(dev)

	dump_resource(&dev, encoders);
	dump_resource(&dev, connectors);
	dump_resource(&dev, crtcs);
	dump_resource(&dev, planes);
	dump_resource(&dev, framebuffers);

	if (dev.use_atomic)
		dev.req = drmModeAtomicAlloc();

	for (i = 0; i < prop_count; ++i)
		set_property(&dev, &prop_args[i]);

	if (dev.use_atomic) {
		if (set_preferred || (count && plane_count)) {
			uint64_t cap = 0;

			ret = drmGetCap(dev.fd, DRM_CAP_DUMB_BUFFER, &cap);
			if (ret || cap == 0) {
				fprintf(stderr, "driver doesn't support the dumb buffer API\n");
				return 1;
			}

			if (set_preferred || count)
				count = set_mode(&dev, &pipe_args, count);

			if (dump_path) {
				if (!pipe_has_writeback_connector(&dev, pipe_args, count)) {
					fprintf(stderr, "No writeback connector found, can not dump.\n");
					return 1;
				}

				writeback_config(&dev, pipe_args, count);
			}

			if (plane_count)
				atomic_set_planes(&dev, plane_args, plane_count, false);

			ret = drmModeAtomicCommit(dev.fd, dev.req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
			if (ret) {
				fprintf(stderr, "Atomic Commit failed [1]\n");
				return 1;
			}

			/*
			 * Since only writeback connectors have an output fb, this should only be
			 * called for writeback.
			 */
			if (dump_path) {
				ret = poll_writeback_fence(dev.writeback_fence_fd, 1000);
				if (ret)
					fprintf(stderr, "Poll for writeback error: %d. Skipping Dump.\n",
							ret);
				dump_output_fb(&dev, pipe_args, dump_path, count);
			}

			if (test_vsync)
				atomic_test_page_flip(&dev, pipe_args, plane_args, plane_count);

			if (drop_master)
				drmDropMaster(dev.fd);

			getchar();

			drmModeAtomicFree(dev.req);
			dev.req = drmModeAtomicAlloc();

			/* XXX: properly teardown the preferred mode/plane state */
			if (plane_count)
				atomic_clear_planes(&dev, plane_args, plane_count);

			if (count)
				atomic_clear_mode(&dev, pipe_args, count);
		}

		ret = drmModeAtomicCommit(dev.fd, dev.req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
		if (ret)
			fprintf(stderr, "Atomic Commit failed\n");

		if (count && plane_count)
			atomic_clear_FB(&dev, plane_args, plane_count);

		drmModeAtomicFree(dev.req);
	} else {
		if (dump_path) {
			fprintf(stderr, "writeback / dump is only supported in atomic mode\n");
			return 1;
		}

		if (set_preferred || count || plane_count) {
			uint64_t cap = 0;

			ret = drmGetCap(dev.fd, DRM_CAP_DUMB_BUFFER, &cap);
			if (ret || cap == 0) {
				fprintf(stderr, "driver doesn't support the dumb buffer API\n");
				return 1;
			}

			if (set_preferred || count)
				count = set_mode(&dev, &pipe_args, count);

			if (plane_count)
				set_planes(&dev, plane_args, plane_count);

			if (test_cursor)
				set_cursors(&dev, pipe_args, count);

			if (test_vsync)
				test_page_flip(&dev, pipe_args, count);

			if (drop_master)
				drmDropMaster(dev.fd);

			getchar();

			if (test_cursor)
				clear_cursors(&dev);

			if (plane_count)
				clear_planes(&dev, plane_args, plane_count);

			if (set_preferred || count)
				clear_mode(&dev);
		}
	}

	free_resources(dev.resources);
	drmClose(dev.fd);

	return 0;
}
