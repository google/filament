/*
 * TinyEXR - Vulkan GPU backend (optional; built only with -DEXR_USE_VULKAN).
 *
 * Mirrors src/exr_gpu_cuda.c but targets Vulkan compute through the vkew loader
 * (runtime dlopen of libvulkan; no SDK at build time, links only -ldl). Compute
 * shaders are precompiled SPIR-V embedded in exr_vk_shaders.spv.inc. All
 * device buffers use HOST_VISIBLE|HOST_COHERENT memory and are mapped directly,
 * so transfers are plain memcpy and each operation synchronises with
 * vkQueueWaitIdle (simple + correct; performance is a later concern).
 *
 * Same hybrid split as the CUDA backend: CPU entropy + GPU
 * predictor/deinterleave/channel-split for ZIP/ZIPS/RLE/uncompressed scanline
 * parts; everything else falls back to the CPU codec.
 *
 * Copyright (c) 2014-2026 Syoyo Fujita and TinyEXR authors
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "exr_vk.h"

#ifndef EXR_USE_VULKAN
/* ===========================================================================
 * Stub backend (Vulkan support OFF): inert, falls back to the CPU API.
 * ========================================================================= */
int exr_vk_available(void) { return 0; }
int exr_vk_device_count(void) { return 0; }
exr_result exr_vk_device_name(int d, char *b, size_t n) {
    (void)d;
    if (b && n) b[0] = '\0';
    return EXR_ERROR_UNSUPPORTED;
}
exr_result exr_vk_context_create(const exr_allocator *a, const exr_vk_options *o,
                                 exr_vk_context **out) {
    (void)a;
    (void)o;
    if (out) *out = NULL;
    return EXR_ERROR_UNSUPPORTED;
}
void exr_vk_context_destroy(exr_vk_context *c) { (void)c; }
exr_result exr_vk_load_from_file(exr_vk_context *c, const char *p,
                                 const exr_allocator *a, exr_image *o) {
    (void)c;
    return exr_load_from_file(p, a, o);
}
exr_result exr_vk_load_from_memory(exr_vk_context *c, const void *d, size_t s,
                                   const exr_allocator *a, exr_image *o) {
    (void)c;
    return exr_load_from_memory(d, s, a, o);
}
exr_result exr_vk_save_to_file(exr_vk_context *c, const char *p,
                               const exr_image *i, exr_compression k) {
    (void)c;
    return exr_save_to_file(p, i, k);
}
exr_result exr_vk_save_to_memory(exr_vk_context *c, void **d, size_t *s,
                                 const exr_allocator *a, const exr_image *i,
                                 exr_compression k) {
    (void)c;
    return exr_save_to_memory(d, s, a, i, k);
}
exr_result exr_vk_resize_float(exr_vk_context *c, const float *src, int sw,
                               int sh, size_t ss, float *dst, int dw, int dh,
                               size_t ds, int ch, exr_resize_filter f,
                               exr_edge_mode e, int ac) {
    (void)c;
    return exr_resize_float(NULL, src, sw, sh, ss, dst, dw, dh, ds, ch, f, e, ac);
}
exr_result exr_vk_convert_pixels(exr_vk_context *c, void *d, exr_pixel_type dt,
                                 const void *s, exr_pixel_type st, size_t n,
                                 exr_convert_mode m) {
    (void)c;
    return exr_convert_pixels(d, dt, s, st, n, m);
}
exr_result exr_vk_color_apply_matrix(exr_vk_context *c, float *d, const float *s,
                                     size_t n, int ch, const float m[9]) {
    (void)c;
    return exr_color_apply_matrix(d, s, n, ch, m);
}
exr_result exr_vk_tonemap_float(exr_vk_context *c, float *d, const float *s,
                                size_t n, int ch, exr_tonemap_op op,
                                const exr_tonemap_params *p) {
    (void)c;
    return exr_tonemap_float(d, s, n, ch, op, p);
}
exr_result exr_vk_encode_transfer(exr_vk_context *c, float *d, const float *s,
                                  size_t n, exr_transfer tf) {
    (void)c;
    return exr_encode_transfer(d, s, n, tf);
}
exr_result exr_vk_decode_transfer(exr_vk_context *c, float *d, const float *s,
                                  size_t n, exr_transfer tf) {
    (void)c;
    return exr_decode_transfer(d, s, n, tf);
}
exr_result exr_vk_lut3d_apply(exr_vk_context *c, float *d, const float *s,
                              size_t n, int ch, const exr_lut3d *l,
                              exr_lut_interp it) {
    (void)c;
    return exr_lut3d_apply(d, s, n, ch, l, it);
}
exr_result exr_vk_part_to_rgba_float(exr_vk_context *c, const exr_allocator *a,
                                     const exr_part *p, float **o, int *w,
                                     int *h, int *ch) {
    (void)c;
    return exr_part_to_rgba_float(a, p, o, w, h, ch);
}
exr_result exr_vk_rgba_float_to_part(exr_vk_context *c, const exr_allocator *a,
                                     const float *rgba, int w, int h, int ch,
                                     exr_pixel_type dt, exr_part *o) {
    (void)c;
    return exr_rgba_float_to_part(a, rgba, w, h, ch, dt, o);
}

#else /* EXR_USE_VULKAN ===================================================== */

#include "exr_internal.h"
#include "vkew.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "exr_vk_shaders.spv.inc"

/* ---- shader registry (must match the .inc array names) ------------------ */
enum {
    SH_resize_h, SH_resize_v, SH_premult, SH_color_matrix, SH_tonemap,
    SH_transfer, SH_lut3d, SH_convert, SH_gather_f32, SH_scatter_f32,
    SH_pred_partials, SH_pred_offsets, SH_pred_apply, SH_deinterleave,
    SH_interleave_encode, SH_predictor_encode, SH_channel_split,
    SH_channel_gather, SH_COUNT
};
typedef struct {
    const uint32_t *code;
    size_t size;
    int nbuf;
} vk_shader_def;
#define SHDEF(n, b) {exr_vk_spv_##n, sizeof(exr_vk_spv_##n), b}
static const vk_shader_def g_shaders[SH_COUNT] = {
    SHDEF(resize_h, 4), SHDEF(resize_v, 4), SHDEF(premult, 1),
    SHDEF(color_matrix, 2), SHDEF(tonemap, 2), SHDEF(transfer, 2),
    SHDEF(lut3d, 3), SHDEF(convert, 2), SHDEF(gather_f32, 2),
    SHDEF(scatter_f32, 2), SHDEF(pred_partials, 2), SHDEF(pred_offsets, 2),
    SHDEF(pred_apply, 3), SHDEF(deinterleave, 2), SHDEF(interleave_encode, 2),
    SHDEF(predictor_encode, 2), SHDEF(channel_split, 2), SHDEF(channel_gather, 3)
};
#undef SHDEF

struct exr_vk_context {
    exr_allocator alloc;
    int verbose;
    VkInstance instance;
    VkPhysicalDevice phys;
    VkDevice dev;
    VkQueue queue;
    uint32_t qfi;
    uint32_t mem_type;          /* host-visible coherent memory type index */
    VkCommandPool cmdpool;
    VkCommandBuffer cmd;
    VkDescriptorPool descpool;
    VkDescriptorSetLayout set_layout[5];  /* index by buffer count 1..4 */
    VkPipelineLayout pipe_layout[5];
    VkShaderModule module[SH_COUNT];
    VkPipeline pipeline[SH_COUNT];
};

typedef struct {
    VkBuffer buf;
    VkDeviceMemory mem;
    void *mapped;
    VkDeviceSize size;
} vkbuf;

/* ---- one-time availability probe ---------------------------------------- */
static int g_probe_done = 0, g_probe_ok = 0;
static void vk_probe(void) {
    if (g_probe_done) return;
    g_probe_done = 1;
    if (vkewInit() != VKEW_SUCCESS) return;
    g_probe_ok = 1; /* a loadable libvulkan; device check happens at create */
}
int exr_vk_available(void) {
    vk_probe();
    return g_probe_ok;
}

/* Minimal instance just to enumerate devices (used by available/name/create). */
static VkInstance make_instance(void) {
    VkApplicationInfo app;
    VkInstanceCreateInfo ici;
    VkInstance inst = VK_NULL_HANDLE;
    memset(&app, 0, sizeof(app));
    app.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app.pApplicationName = "tinyexr";
    app.apiVersion = (1u << 22); /* VK_API_VERSION_1_0 */
    memset(&ici, 0, sizeof(ici));
    ici.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    ici.pApplicationInfo = &app;
    if (vkCreateInstance(&ici, NULL, &inst) != VK_SUCCESS) return VK_NULL_HANDLE;
    vkewLoadInstance(inst);
    return inst;
}

int exr_vk_device_count(void) {
    VkInstance inst;
    uint32_t n = 0;
    vk_probe();
    if (!g_probe_ok) return 0;
    inst = make_instance();
    if (!inst) return 0;
    vkEnumeratePhysicalDevices(inst, &n, NULL);
    vkDestroyInstance(inst, NULL);
    return (int)n;
}

exr_result exr_vk_device_name(int device, char *buf, size_t buf_size) {
    VkInstance inst;
    uint32_t n = 0;
    VkPhysicalDevice *pds;
    VkPhysicalDeviceProperties props;
    if (!buf || buf_size == 0) return EXR_ERROR_INVALID_ARGUMENT;
    buf[0] = '\0';
    vk_probe();
    if (!g_probe_ok) return EXR_ERROR_UNSUPPORTED;
    inst = make_instance();
    if (!inst) return EXR_ERROR_UNSUPPORTED;
    vkEnumeratePhysicalDevices(inst, &n, NULL);
    if (device < 0 || (uint32_t)device >= n) {
        vkDestroyInstance(inst, NULL);
        return EXR_ERROR_INVALID_ARGUMENT;
    }
    pds = (VkPhysicalDevice *)malloc(n * sizeof(*pds));
    if (!pds) { vkDestroyInstance(inst, NULL); return EXR_ERROR_OUT_OF_MEMORY; }
    vkEnumeratePhysicalDevices(inst, &n, pds);
    vkGetPhysicalDeviceProperties(pds[device], &props);
    snprintf(buf, buf_size, "%s", props.deviceName);
    free(pds);
    vkDestroyInstance(inst, NULL);
    return EXR_SUCCESS;
}

/* ---- context create ----------------------------------------------------- */
static void destroy_ctx(exr_vk_context *c);

static exr_result build_pipelines(exr_vk_context *c) {
    int nb, i;
    /* descriptor set layouts + pipeline layouts for 1..4 storage buffers */
    for (nb = 1; nb <= 4; ++nb) {
        VkDescriptorSetLayoutBinding b[4];
        VkDescriptorSetLayoutCreateInfo dli;
        VkPushConstantRange pcr;
        VkPipelineLayoutCreateInfo pli;
        int k;
        memset(b, 0, sizeof(b));
        for (k = 0; k < nb; ++k) {
            b[k].binding = (uint32_t)k;
            b[k].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            b[k].descriptorCount = 1;
            b[k].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        }
        memset(&dli, 0, sizeof(dli));
        dli.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        dli.bindingCount = (uint32_t)nb;
        dli.pBindings = b;
        if (vkCreateDescriptorSetLayout(c->dev, &dli, NULL,
                                        &c->set_layout[nb]) != VK_SUCCESS)
            return EXR_ERROR_IO;
        memset(&pcr, 0, sizeof(pcr));
        pcr.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        pcr.offset = 0;
        pcr.size = 128;
        memset(&pli, 0, sizeof(pli));
        pli.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pli.setLayoutCount = 1;
        pli.pSetLayouts = &c->set_layout[nb];
        pli.pushConstantRangeCount = 1;
        pli.pPushConstantRanges = &pcr;
        if (vkCreatePipelineLayout(c->dev, &pli, NULL, &c->pipe_layout[nb]) !=
            VK_SUCCESS)
            return EXR_ERROR_IO;
    }
    for (i = 0; i < SH_COUNT; ++i) {
        VkShaderModuleCreateInfo smi;
        VkComputePipelineCreateInfo cpi;
        int nbuf = g_shaders[i].nbuf;
        memset(&smi, 0, sizeof(smi));
        smi.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        smi.codeSize = g_shaders[i].size;
        smi.pCode = g_shaders[i].code;
        if (vkCreateShaderModule(c->dev, &smi, NULL, &c->module[i]) != VK_SUCCESS)
            return EXR_ERROR_IO;
        memset(&cpi, 0, sizeof(cpi));
        cpi.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        cpi.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        cpi.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        cpi.stage.module = c->module[i];
        cpi.stage.pName = "main";
        cpi.layout = c->pipe_layout[nbuf];
        if (vkCreateComputePipelines(c->dev, VK_NULL_HANDLE, 1, &cpi, NULL,
                                     &c->pipeline[i]) != VK_SUCCESS)
            return EXR_ERROR_IO;
    }
    return EXR_SUCCESS;
}

exr_result exr_vk_context_create(const exr_allocator *alloc,
                                 const exr_vk_options *opts,
                                 exr_vk_context **out) {
    exr_vk_context *c;
    int device = (opts && opts->device >= 0) ? opts->device : 0;
    uint32_t n = 0, i;
    VkPhysicalDevice *pds = NULL;
    VkPhysicalDeviceMemoryProperties mp;
    VkQueueFamilyProperties *qf = NULL;
    uint32_t nq = 0;
    float prio = 1.0f;
    VkDeviceQueueCreateInfo qci;
    VkDeviceCreateInfo dci;
    VkCommandPoolCreateInfo cpci;
    VkCommandBufferAllocateInfo cba;
    VkDescriptorPoolSize ps;
    VkDescriptorPoolCreateInfo dpi;
    exr_result rc;

    if (!out) return EXR_ERROR_INVALID_ARGUMENT;
    *out = NULL;
    vk_probe();
    if (!g_probe_ok) return EXR_ERROR_UNSUPPORTED;
    if (!alloc) alloc = exr_default_allocator();

    c = (exr_vk_context *)exr_calloc(alloc, 1, sizeof(*c));
    if (!c) return EXR_ERROR_OUT_OF_MEMORY;
    c->alloc = *alloc;
    c->verbose = opts ? opts->verbose : 0;

    c->instance = make_instance();
    if (!c->instance) { rc = EXR_ERROR_UNSUPPORTED; goto fail; }
    vkEnumeratePhysicalDevices(c->instance, &n, NULL);
    if (n == 0 || (uint32_t)device >= n) { rc = EXR_ERROR_UNSUPPORTED; goto fail; }
    pds = (VkPhysicalDevice *)malloc(n * sizeof(*pds));
    if (!pds) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }
    vkEnumeratePhysicalDevices(c->instance, &n, pds);
    c->phys = pds[device];
    free(pds);
    pds = NULL;

    /* compute queue family */
    vkGetPhysicalDeviceQueueFamilyProperties(c->phys, &nq, NULL);
    qf = (VkQueueFamilyProperties *)malloc(nq * sizeof(*qf));
    if (!qf) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }
    vkGetPhysicalDeviceQueueFamilyProperties(c->phys, &nq, qf);
    c->qfi = (uint32_t)-1;
    for (i = 0; i < nq; ++i)
        if (qf[i].queueFlags & VK_QUEUE_COMPUTE_BIT) { c->qfi = i; break; }
    free(qf);
    qf = NULL;
    if (c->qfi == (uint32_t)-1) { rc = EXR_ERROR_UNSUPPORTED; goto fail; }

    memset(&qci, 0, sizeof(qci));
    qci.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qci.queueFamilyIndex = c->qfi;
    qci.queueCount = 1;
    qci.pQueuePriorities = &prio;
    memset(&dci, 0, sizeof(dci));
    dci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dci.queueCreateInfoCount = 1;
    dci.pQueueCreateInfos = &qci;
    if (vkCreateDevice(c->phys, &dci, NULL, &c->dev) != VK_SUCCESS) {
        rc = EXR_ERROR_IO;
        goto fail;
    }
    vkGetDeviceQueue(c->dev, c->qfi, 0, &c->queue);

    /* host-visible coherent memory type */
    vkGetPhysicalDeviceMemoryProperties(c->phys, &mp);
    c->mem_type = (uint32_t)-1;
    for (i = 0; i < mp.memoryTypeCount; ++i) {
        VkFlags want = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        if ((mp.memoryTypes[i].propertyFlags & want) == want) {
            c->mem_type = i;
            break;
        }
    }
    if (c->mem_type == (uint32_t)-1) { rc = EXR_ERROR_UNSUPPORTED; goto fail; }

    memset(&cpci, 0, sizeof(cpci));
    cpci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cpci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cpci.queueFamilyIndex = c->qfi;
    if (vkCreateCommandPool(c->dev, &cpci, NULL, &c->cmdpool) != VK_SUCCESS) {
        rc = EXR_ERROR_IO;
        goto fail;
    }
    memset(&cba, 0, sizeof(cba));
    cba.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cba.commandPool = c->cmdpool;
    cba.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cba.commandBufferCount = 1;
    if (vkAllocateCommandBuffers(c->dev, &cba, &c->cmd) != VK_SUCCESS) {
        rc = EXR_ERROR_IO;
        goto fail;
    }

    ps.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    ps.descriptorCount = 16;
    memset(&dpi, 0, sizeof(dpi));
    dpi.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    dpi.maxSets = 4;
    dpi.poolSizeCount = 1;
    dpi.pPoolSizes = &ps;
    if (vkCreateDescriptorPool(c->dev, &dpi, NULL, &c->descpool) != VK_SUCCESS) {
        rc = EXR_ERROR_IO;
        goto fail;
    }

    rc = build_pipelines(c);
    if (!EXR_OK(rc)) goto fail;

    *out = c;
    return EXR_SUCCESS;
fail:
    free(pds);
    free(qf);
    destroy_ctx(c);
    return rc;
}

static void destroy_ctx(exr_vk_context *c) {
    int i;
    exr_allocator a;
    if (!c) return;
    a = c->alloc;
    if (c->dev) {
        vkDeviceWaitIdle(c->dev);
        for (i = 0; i < SH_COUNT; ++i) {
            if (c->pipeline[i]) vkDestroyPipeline(c->dev, c->pipeline[i], NULL);
            if (c->module[i]) vkDestroyShaderModule(c->dev, c->module[i], NULL);
        }
        for (i = 1; i <= 4; ++i) {
            if (c->pipe_layout[i])
                vkDestroyPipelineLayout(c->dev, c->pipe_layout[i], NULL);
            if (c->set_layout[i])
                vkDestroyDescriptorSetLayout(c->dev, c->set_layout[i], NULL);
        }
        if (c->descpool) vkDestroyDescriptorPool(c->dev, c->descpool, NULL);
        if (c->cmdpool) vkDestroyCommandPool(c->dev, c->cmdpool, NULL);
        vkDestroyDevice(c->dev, NULL);
    }
    if (c->instance) vkDestroyInstance(c->instance, NULL);
    exr_free(&a, c);
}

void exr_vk_context_destroy(exr_vk_context *c) { destroy_ctx(c); }

/* ---- buffer helpers (host-visible coherent, persistently mapped) -------- */
static exr_result vkbuf_create(exr_vk_context *c, size_t bytes, vkbuf *out) {
    VkBufferCreateInfo bci;
    VkMemoryRequirements mr;
    VkMemoryAllocateInfo mai;
    memset(out, 0, sizeof(*out));
    if (bytes == 0) bytes = 4;
    bytes = (bytes + 3u) & ~(size_t)3u;
    out->size = bytes;
    memset(&bci, 0, sizeof(bci));
    bci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bci.size = bytes;
    bci.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if (vkCreateBuffer(c->dev, &bci, NULL, &out->buf) != VK_SUCCESS)
        return EXR_ERROR_OUT_OF_MEMORY;
    vkGetBufferMemoryRequirements(c->dev, out->buf, &mr);
    memset(&mai, 0, sizeof(mai));
    mai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mai.allocationSize = mr.size;
    mai.memoryTypeIndex = c->mem_type;
    if (vkAllocateMemory(c->dev, &mai, NULL, &out->mem) != VK_SUCCESS) {
        vkDestroyBuffer(c->dev, out->buf, NULL);
        out->buf = VK_NULL_HANDLE;
        return EXR_ERROR_OUT_OF_MEMORY;
    }
    if (vkBindBufferMemory(c->dev, out->buf, out->mem, 0) != VK_SUCCESS ||
        vkMapMemory(c->dev, out->mem, 0, VK_WHOLE_SIZE, 0, &out->mapped) !=
            VK_SUCCESS) {
        vkFreeMemory(c->dev, out->mem, NULL);
        vkDestroyBuffer(c->dev, out->buf, NULL);
        memset(out, 0, sizeof(*out));
        return EXR_ERROR_IO;
    }
    return EXR_SUCCESS;
}
static void vkbuf_destroy(exr_vk_context *c, vkbuf *b) {
    if (b->mem) { vkUnmapMemory(c->dev, b->mem); vkFreeMemory(c->dev, b->mem, NULL); }
    if (b->buf) vkDestroyBuffer(c->dev, b->buf, NULL);
    memset(b, 0, sizeof(*b));
}

/* Record + submit one compute dispatch, then wait. */
static exr_result vk_dispatch(exr_vk_context *c, int shader, const vkbuf *bufs,
                              int nbuf, const void *push, uint32_t push_size,
                              uint32_t groups) {
    VkDescriptorSetAllocateInfo dsa;
    VkDescriptorSet set;
    VkDescriptorBufferInfo dbi[4];
    VkWriteDescriptorSet w[4];
    VkCommandBufferBeginInfo bi;
    VkSubmitInfo si;
    int i;
    if (groups == 0) return EXR_SUCCESS;

    if (vkResetDescriptorPool(c->dev, c->descpool, 0) != VK_SUCCESS)
        return EXR_ERROR_IO;
    memset(&dsa, 0, sizeof(dsa));
    dsa.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    dsa.descriptorPool = c->descpool;
    dsa.descriptorSetCount = 1;
    dsa.pSetLayouts = &c->set_layout[nbuf];
    if (vkAllocateDescriptorSets(c->dev, &dsa, &set) != VK_SUCCESS)
        return EXR_ERROR_IO;
    memset(dbi, 0, sizeof(dbi));
    memset(w, 0, sizeof(w));
    for (i = 0; i < nbuf; ++i) {
        dbi[i].buffer = bufs[i].buf;
        dbi[i].offset = 0;
        dbi[i].range = VK_WHOLE_SIZE;
        w[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        w[i].dstSet = set;
        w[i].dstBinding = (uint32_t)i;
        w[i].descriptorCount = 1;
        w[i].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        w[i].pBufferInfo = &dbi[i];
    }
    vkUpdateDescriptorSets(c->dev, (uint32_t)nbuf, w, 0, NULL);

    if (vkResetCommandPool(c->dev, c->cmdpool, 0) != VK_SUCCESS)
        return EXR_ERROR_IO;
    memset(&bi, 0, sizeof(bi));
    bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    if (vkBeginCommandBuffer(c->cmd, &bi) != VK_SUCCESS) return EXR_ERROR_IO;
    vkCmdBindPipeline(c->cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                      c->pipeline[shader]);
    vkCmdBindDescriptorSets(c->cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                            c->pipe_layout[nbuf], 0, 1, &set, 0, NULL);
    if (push && push_size)
        vkCmdPushConstants(c->cmd, c->pipe_layout[nbuf],
                           VK_SHADER_STAGE_COMPUTE_BIT, 0, push_size, push);
    vkCmdDispatch(c->cmd, groups, 1, 1);
    if (vkEndCommandBuffer(c->cmd) != VK_SUCCESS) return EXR_ERROR_IO;
    memset(&si, 0, sizeof(si));
    si.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    si.commandBufferCount = 1;
    si.pCommandBuffers = &c->cmd;
    if (vkQueueSubmit(c->queue, 1, &si, VK_NULL_HANDLE) != VK_SUCCESS)
        return EXR_ERROR_IO;
    if (vkQueueWaitIdle(c->queue) != VK_SUCCESS) return EXR_ERROR_IO;
    return EXR_SUCCESS;
}

#define GROUPS(n) (((uint32_t)(n) + 63u) / 64u)

/* ===========================================================================
 * Eligibility (shared with the CUDA backend's policy)
 * ========================================================================= */
static int part_vk_eligible(const exr_header *h) {
    int c;
    if (h->part_type != EXR_PART_SCANLINE) return 0;
    for (c = 0; c < h->num_channels; ++c)
        if (h->channels[c].x_sampling != 1 || h->channels[c].y_sampling != 1)
            return 0;
    return 1;
}
static int image_vk_eligible(const exr_image *img) {
    int p;
    if (img->num_parts <= 0) return 0;
    for (p = 0; p < img->num_parts; ++p) {
        if (img->parts[p].is_deep) return 0;
        if (!part_vk_eligible(&img->parts[p].header)) return 0;
    }
    return 1;
}

/* ===========================================================================
 * Processing
 * ========================================================================= */
/* push-constant structs (match the GLSL std430 push_constant blocks) */
typedef struct { uint32_t n, tf, decode; } pc_transfer;
typedef struct { uint32_t n, ch; float m[9]; } pc_color;
typedef struct { uint32_t n, ch, op; float exposure, white; } pc_tonemap;
typedef struct { uint32_t n, ch, sz; float dmn[3], dmx[3]; } pc_lut;
typedef struct { uint32_t n, dtype, stype, norm; } pc_convert;
typedef struct { uint32_t n, outCh, cdst, stype; } pc_gather;
typedef struct { uint32_t n, inCh, csrc, dtype; } pc_scatter;
typedef struct { uint32_t n, ch, ac, undo; } pc_premult;
typedef struct { uint32_t sw, sh, sstride, dw, dstride, ch, support; } pc_rh;
typedef struct { uint32_t dw, dh, sstride, dstride, ch, support; } pc_rv;

static exr_result run_transfer(exr_vk_context *c, float *dst, const float *src,
                               size_t n, exr_transfer tf, int decode) {
    vkbuf bd, bs;
    pc_transfer pc;
    exr_result rc;
    rc = vkbuf_create(c, n * sizeof(float), &bd); if (!EXR_OK(rc)) return rc;
    rc = vkbuf_create(c, n * sizeof(float), &bs); if (!EXR_OK(rc)) { vkbuf_destroy(c, &bd); return rc; }
    memcpy(bs.mapped, src, n * sizeof(float));
    pc.n = (uint32_t)n; pc.tf = (uint32_t)tf; pc.decode = (uint32_t)decode;
    { vkbuf b[2]; b[0] = bd; b[1] = bs;
      rc = vk_dispatch(c, SH_transfer, b, 2, &pc, sizeof(pc), GROUPS(n)); }
    if (EXR_OK(rc)) memcpy(dst, bd.mapped, n * sizeof(float));
    vkbuf_destroy(c, &bd); vkbuf_destroy(c, &bs);
    return rc;
}
exr_result exr_vk_encode_transfer(exr_vk_context *c, float *d, const float *s,
                                  size_t n, exr_transfer tf) {
    if (!c || !exr_vk_available()) return exr_encode_transfer(d, s, n, tf);
    return run_transfer(c, d, s, n, tf, 0);
}
exr_result exr_vk_decode_transfer(exr_vk_context *c, float *d, const float *s,
                                  size_t n, exr_transfer tf) {
    if (!c || !exr_vk_available()) return exr_decode_transfer(d, s, n, tf);
    return run_transfer(c, d, s, n, tf, 1);
}

exr_result exr_vk_color_apply_matrix(exr_vk_context *c, float *dst,
                                     const float *src, size_t pixels, int ch,
                                     const float m[9]) {
    vkbuf bd, bs;
    pc_color pc;
    exr_result rc;
    size_t bytes = pixels * (size_t)ch * sizeof(float);
    if (!c || !exr_vk_available()) return exr_color_apply_matrix(dst, src, pixels, ch, m);
    rc = vkbuf_create(c, bytes, &bd); if (!EXR_OK(rc)) return rc;
    rc = vkbuf_create(c, bytes, &bs); if (!EXR_OK(rc)) { vkbuf_destroy(c, &bd); return rc; }
    memcpy(bs.mapped, src, bytes);
    pc.n = (uint32_t)pixels; pc.ch = (uint32_t)ch;
    memcpy(pc.m, m, sizeof(pc.m));
    { vkbuf b[2]; b[0] = bd; b[1] = bs;
      rc = vk_dispatch(c, SH_color_matrix, b, 2, &pc, sizeof(pc), GROUPS(pixels)); }
    if (EXR_OK(rc)) memcpy(dst, bd.mapped, bytes);
    vkbuf_destroy(c, &bd); vkbuf_destroy(c, &bs);
    return rc;
}

exr_result exr_vk_tonemap_float(exr_vk_context *c, float *dst, const float *src,
                                size_t pixels, int ch, exr_tonemap_op op,
                                const exr_tonemap_params *p) {
    vkbuf bd, bs;
    pc_tonemap pc;
    exr_result rc;
    size_t bytes = pixels * (size_t)ch * sizeof(float);
    if (!c || !exr_vk_available()) return exr_tonemap_float(dst, src, pixels, ch, op, p);
    rc = vkbuf_create(c, bytes, &bd); if (!EXR_OK(rc)) return rc;
    rc = vkbuf_create(c, bytes, &bs); if (!EXR_OK(rc)) { vkbuf_destroy(c, &bd); return rc; }
    memcpy(bs.mapped, src, bytes);
    pc.n = (uint32_t)pixels; pc.ch = (uint32_t)ch; pc.op = (uint32_t)op;
    pc.exposure = (p && p->exposure != 0.0f) ? p->exposure : 1.0f;
    pc.white = p ? p->white_point : 0.0f;
    { vkbuf b[2]; b[0] = bd; b[1] = bs;
      rc = vk_dispatch(c, SH_tonemap, b, 2, &pc, sizeof(pc), GROUPS(pixels)); }
    if (EXR_OK(rc)) memcpy(dst, bd.mapped, bytes);
    vkbuf_destroy(c, &bd); vkbuf_destroy(c, &bs);
    return rc;
}

exr_result exr_vk_lut3d_apply(exr_vk_context *c, float *dst, const float *src,
                              size_t pixels, int ch, const exr_lut3d *lut,
                              exr_lut_interp interp) {
    vkbuf bd, bs, bl;
    pc_lut pc;
    exr_result rc;
    size_t bytes = pixels * (size_t)ch * sizeof(float);
    size_t lbytes;
    (void)interp;
    if (!c || !exr_vk_available()) return exr_lut3d_apply(dst, src, pixels, ch, lut, interp);
    if (!lut || lut->size < 2) return EXR_ERROR_INVALID_ARGUMENT;
    lbytes = (size_t)lut->size * lut->size * lut->size * 3 * sizeof(float);
    rc = vkbuf_create(c, bytes, &bd); if (!EXR_OK(rc)) return rc;
    rc = vkbuf_create(c, bytes, &bs); if (!EXR_OK(rc)) { vkbuf_destroy(c, &bd); return rc; }
    rc = vkbuf_create(c, lbytes, &bl); if (!EXR_OK(rc)) { vkbuf_destroy(c, &bd); vkbuf_destroy(c, &bs); return rc; }
    memcpy(bs.mapped, src, bytes);
    memcpy(bl.mapped, lut->data, lbytes);
    pc.n = (uint32_t)pixels; pc.ch = (uint32_t)ch; pc.sz = (uint32_t)lut->size;
    pc.dmn[0] = lut->domain_min[0]; pc.dmn[1] = lut->domain_min[1]; pc.dmn[2] = lut->domain_min[2];
    pc.dmx[0] = lut->domain_max[0]; pc.dmx[1] = lut->domain_max[1]; pc.dmx[2] = lut->domain_max[2];
    { vkbuf b[3]; b[0] = bd; b[1] = bs; b[2] = bl;
      rc = vk_dispatch(c, SH_lut3d, b, 3, &pc, sizeof(pc), GROUPS(pixels)); }
    if (EXR_OK(rc)) memcpy(dst, bd.mapped, bytes);
    vkbuf_destroy(c, &bd); vkbuf_destroy(c, &bs); vkbuf_destroy(c, &bl);
    return rc;
}

exr_result exr_vk_convert_pixels(exr_vk_context *c, void *dst,
                                 exr_pixel_type dt, const void *src,
                                 exr_pixel_type st, size_t n,
                                 exr_convert_mode mode) {
    vkbuf bd, bs;
    pc_convert pc;
    exr_result rc;
    size_t sps = exr_pixel_size(st), dps = exr_pixel_size(dt);
    if (!c || !exr_vk_available()) return exr_convert_pixels(dst, dt, src, st, n, mode);
    rc = vkbuf_create(c, n * dps, &bd); if (!EXR_OK(rc)) return rc;
    rc = vkbuf_create(c, n * sps, &bs); if (!EXR_OK(rc)) { vkbuf_destroy(c, &bd); return rc; }
    memcpy(bs.mapped, src, n * sps);
    pc.n = (uint32_t)n; pc.dtype = (uint32_t)dt; pc.stype = (uint32_t)st;
    pc.norm = (mode == EXR_CONVERT_NORMALIZED);
    { vkbuf b[2]; b[0] = bd; b[1] = bs;
      rc = vk_dispatch(c, SH_convert, b, 2, &pc, sizeof(pc), GROUPS(n)); }
    if (EXR_OK(rc)) memcpy(dst, bd.mapped, n * dps);
    vkbuf_destroy(c, &bd); vkbuf_destroy(c, &bs);
    return rc;
}

/* ---- channel gather/scatter --------------------------------------------- */
exr_result exr_vk_part_to_rgba_float(exr_vk_context *c, const exr_allocator *a,
                                     const exr_part *part, float **out, int *ow,
                                     int *oh, int *oc) {
    const exr_header *h;
    int nch, outc, cc;
    long n;
    float *host;
    vkbuf bout;
    exr_result rc = EXR_SUCCESS;
    if (!c || !exr_vk_available()) return exr_part_to_rgba_float(a, part, out, ow, oh, oc);
    if (!part || !out) return EXR_ERROR_INVALID_ARGUMENT;
    h = &part->header;
    if (!part_vk_eligible(h)) return exr_part_to_rgba_float(a, part, out, ow, oh, oc);
    a = a ? a : exr_default_allocator();
    nch = h->num_channels;
    outc = nch < 4 ? nch : 4;
    n = (long)part->width * part->height;
    host = (float *)exr_malloc(a, (size_t)n * outc * sizeof(float) + 1);
    if (!host) return EXR_ERROR_OUT_OF_MEMORY;
    rc = vkbuf_create(c, (size_t)n * outc * sizeof(float), &bout);
    if (!EXR_OK(rc)) { exr_free(a, host); return rc; }
    for (cc = 0; cc < outc; ++cc) {
        size_t ps = exr_pixel_size(h->channels[cc].pixel_type);
        pc_gather pc;
        vkbuf bsrc;
        rc = vkbuf_create(c, (size_t)n * ps, &bsrc); if (!EXR_OK(rc)) goto done;
        memcpy(bsrc.mapped, part->images[cc], (size_t)n * ps);
        pc.n = (uint32_t)n; pc.outCh = (uint32_t)outc; pc.cdst = (uint32_t)cc;
        pc.stype = (uint32_t)h->channels[cc].pixel_type;
        { vkbuf b[2]; b[0] = bout; b[1] = bsrc;
          rc = vk_dispatch(c, SH_gather_f32, b, 2, &pc, sizeof(pc), GROUPS(n)); }
        vkbuf_destroy(c, &bsrc);
        if (!EXR_OK(rc)) goto done;
    }
    memcpy(host, bout.mapped, (size_t)n * outc * sizeof(float));
done:
    vkbuf_destroy(c, &bout);
    if (EXR_OK(rc)) {
        *out = host;
        if (ow) *ow = part->width;
        if (oh) *oh = part->height;
        if (oc) *oc = outc;
    } else {
        exr_free(a, host);
    }
    return rc;
}

exr_result exr_vk_rgba_float_to_part(exr_vk_context *c, const exr_allocator *a,
                                     const float *rgba, int width, int height,
                                     int channels, exr_pixel_type dt,
                                     exr_part *out) {
    static const char *names[4] = {"R", "G", "B", "A"};
    long n = (long)width * height;
    int cc;
    size_t dps;
    vkbuf bin;
    exr_result rc;
    if (!c || !exr_vk_available())
        return exr_rgba_float_to_part(a, rgba, width, height, channels, dt, out);
    if (!rgba || !out || channels < 1 || channels > 4)
        return EXR_ERROR_INVALID_ARGUMENT;
    a = a ? a : exr_default_allocator();
    dps = exr_pixel_size(dt);
    memset(out, 0, sizeof(*out));
    out->width = width;
    out->height = height;
    out->header.num_channels = channels;
    out->header.compression = EXR_COMPRESSION_ZIP;
    out->header.part_type = EXR_PART_SCANLINE;
    out->header.line_order = EXR_LINEORDER_INCREASING_Y;
    out->header.data_window.min_x = 0;
    out->header.data_window.min_y = 0;
    out->header.data_window.max_x = width - 1;
    out->header.data_window.max_y = height - 1;
    out->header.display_window = out->header.data_window;
    out->header.pixel_aspect_ratio = 1.0f;
    out->header.screen_window_width = 1.0f;
    out->header.channels =
        (exr_channel *)exr_calloc(a, (size_t)channels, sizeof(exr_channel));
    out->images = (void **)exr_calloc(a, (size_t)channels, sizeof(void *));
    if (!out->header.channels || !out->images) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }
    rc = vkbuf_create(c, (size_t)n * channels * sizeof(float), &bin);
    if (!EXR_OK(rc)) goto fail;
    memcpy(bin.mapped, rgba, (size_t)n * channels * sizeof(float));
    for (cc = 0; cc < channels; ++cc) {
        int sidx = 0, k;
        pc_scatter pc;
        vkbuf bdst;
        for (k = 0; k < channels; ++k)
            if (k != cc && strcmp(names[k], names[cc]) < 0) sidx++;
        snprintf(out->header.channels[sidx].name,
                 sizeof(out->header.channels[sidx].name), "%s", names[cc]);
        out->header.channels[sidx].pixel_type = dt;
        out->header.channels[sidx].x_sampling = 1;
        out->header.channels[sidx].y_sampling = 1;
        out->images[sidx] = exr_malloc(a, (size_t)n * dps + 1);
        if (!out->images[sidx]) { rc = EXR_ERROR_OUT_OF_MEMORY; vkbuf_destroy(c, &bin); goto fail; }
        rc = vkbuf_create(c, (size_t)n * dps, &bdst);
        if (!EXR_OK(rc)) { vkbuf_destroy(c, &bin); goto fail; }
        pc.n = (uint32_t)n; pc.inCh = (uint32_t)channels; pc.csrc = (uint32_t)cc;
        pc.dtype = (uint32_t)dt;
        { vkbuf b[2]; b[0] = bdst; b[1] = bin;
          rc = vk_dispatch(c, SH_scatter_f32, b, 2, &pc, sizeof(pc), GROUPS(n)); }
        if (EXR_OK(rc)) memcpy(out->images[sidx], bdst.mapped, (size_t)n * dps);
        vkbuf_destroy(c, &bdst);
        if (!EXR_OK(rc)) { vkbuf_destroy(c, &bin); goto fail; }
    }
    vkbuf_destroy(c, &bin);
    return EXR_SUCCESS;
fail:
    exr_part_free(a, out);
    return rc;
}

/* ---- resize (filter tables built on host; same math as exr_resize.c) ---- */
static float rk_cubic(float a, float B, float C) {
    if (a < 1.0f)
        return ((12.0f - 9.0f * B - 6.0f * C) * a * a * a +
                (-18.0f + 12.0f * B + 6.0f * C) * a * a + (6.0f - 2.0f * B)) / 6.0f;
    if (a < 2.0f)
        return ((-B - 6.0f * C) * a * a * a + (6.0f * B + 30.0f * C) * a * a +
                (-12.0f * B - 48.0f * C) * a + (8.0f * B + 24.0f * C)) / 6.0f;
    return 0.0f;
}
static float rk_weight(exr_resize_filter f, float t) {
    float a = t < 0.0f ? -t : t;
    switch (f) {
        case EXR_RESIZE_BOX: return a <= 0.5f ? 1.0f : 0.0f;
        case EXR_RESIZE_TRIANGLE: return a < 1.0f ? 1.0f - a : 0.0f;
        case EXR_RESIZE_CATMULL_ROM: return rk_cubic(a, 0.0f, 0.5f);
        default: return rk_cubic(a, 1.0f / 3.0f, 1.0f / 3.0f);
    }
}
static float rk_radius(exr_resize_filter f) {
    if (f == EXR_RESIZE_BOX) return 0.5f;
    if (f == EXR_RESIZE_TRIANGLE) return 1.0f;
    return 2.0f;
}
static int rk_edge(int j, int n, exr_edge_mode m) {
    if (n == 1) return 0;
    if (j >= 0 && j < n) return j;
    if (m == EXR_EDGE_CLAMP) return j < 0 ? 0 : n - 1;
    if (m == EXR_EDGE_WRAP) { j %= n; if (j < 0) j += n; return j; }
    { int p = 2 * n - 2; j %= p; if (j < 0) j += p; return j < n ? j : p - j; }
}
static int rk_ifloor(float x) { int i = (int)x; return ((float)i > x) ? i - 1 : i; }
static int rk_iceil(float x) { int i = (int)x; return ((float)i < x) ? i + 1 : i; }

static exr_result build_axis(const exr_allocator *a, int s, int d,
                             exr_resize_filter filt, exr_edge_mode edge,
                             float **out_w, uint32_t **out_i, int *out_support) {
    float fscale = (d < s) ? (float)s / (float)d : 1.0f;
    float support = rk_radius(filt) * fscale;
    float invscale = 1.0f / fscale;
    int sup = (int)(2.0f * support) + 4;
    float *W;
    uint32_t *I;
    int i, k;
    W = (float *)exr_calloc(a, (size_t)d * sup, sizeof(float));
    I = (uint32_t *)exr_calloc(a, (size_t)d * sup, sizeof(uint32_t));
    if (!W || !I) { exr_free(a, W); exr_free(a, I); return EXR_ERROR_OUT_OF_MEMORY; }
    for (i = 0; i < d; ++i) {
        float center = ((float)i + 0.5f) * ((float)s / (float)d) - 0.5f;
        int lo = rk_iceil(center - support), hi = rk_ifloor(center + support), t;
        float sum = 0.0f;
        int idx = 0;
        float *w = W + (size_t)i * sup;
        uint32_t *ix = I + (size_t)i * sup;
        if (hi < lo) hi = lo;
        for (t = lo; t <= hi && idx < sup; ++t) {
            int mi = rk_edge(t, s, edge);
            float wt = rk_weight(filt, (center - (float)t) * invscale);
            ix[idx] = (uint32_t)mi; w[idx] = wt; sum += wt; idx++;
        }
        for (; idx < sup; ++idx) { ix[idx] = (uint32_t)((lo >= 0 && lo < s) ? lo : 0); w[idx] = 0.0f; }
        if (sum == 0.0f) { w[0] = 1.0f; }
        else { float inv = 1.0f / sum; for (k = 0; k < sup; ++k) w[k] *= inv; }
    }
    *out_w = W; *out_i = I; *out_support = sup;
    return EXR_SUCCESS;
}

exr_result exr_vk_resize_float(exr_vk_context *c, const float *src, int sw,
                               int sh, size_t ss, float *dst, int dw, int dh,
                               size_t ds, int ch, exr_resize_filter filter,
                               exr_edge_mode edge, int alpha_channel) {
    const exr_allocator *a;
    float *wx = NULL, *wy = NULL;
    uint32_t *ix = NULL, *iy = NULL;
    int supx = 0, supy = 0;
    vkbuf bsrc = {0}, btmp = {0}, bdst = {0}, bwx = {0}, bix = {0}, bwy = {0}, biy = {0};
    exr_result rc;
    long y;
    int sstride_e = sw * ch, dstride_e = dw * ch;

    if (!c || !exr_vk_available())
        return exr_resize_float(NULL, src, sw, sh, ss, dst, dw, dh, ds, ch, filter, edge, alpha_channel);
    if (!src || !dst || sw <= 0 || sh <= 0 || dw <= 0 || dh <= 0 || ch < 1 || ch > 4)
        return EXR_ERROR_INVALID_ARGUMENT;
    a = &c->alloc;
    if (ss == 0) ss = (size_t)sw * ch;
    if (ds == 0) ds = (size_t)dw * ch;

    rc = build_axis(a, sw, dw, filter, edge, &wx, &ix, &supx); if (!EXR_OK(rc)) goto done;
    rc = build_axis(a, sh, dh, filter, edge, &wy, &iy, &supy); if (!EXR_OK(rc)) goto done;

    rc = vkbuf_create(c, (size_t)sw * sh * ch * sizeof(float), &bsrc); if (!EXR_OK(rc)) goto done;
    rc = vkbuf_create(c, (size_t)dw * sh * ch * sizeof(float), &btmp); if (!EXR_OK(rc)) goto done;
    rc = vkbuf_create(c, (size_t)dw * dh * ch * sizeof(float), &bdst); if (!EXR_OK(rc)) goto done;
    rc = vkbuf_create(c, (size_t)dw * supx * sizeof(float), &bwx); if (!EXR_OK(rc)) goto done;
    rc = vkbuf_create(c, (size_t)dw * supx * sizeof(uint32_t), &bix); if (!EXR_OK(rc)) goto done;
    rc = vkbuf_create(c, (size_t)dh * supy * sizeof(float), &bwy); if (!EXR_OK(rc)) goto done;
    rc = vkbuf_create(c, (size_t)dh * supy * sizeof(uint32_t), &biy); if (!EXR_OK(rc)) goto done;

    for (y = 0; y < sh; ++y)
        memcpy((float *)bsrc.mapped + y * sstride_e, src + (size_t)y * ss,
               (size_t)sstride_e * sizeof(float));
    memcpy(bwx.mapped, wx, (size_t)dw * supx * sizeof(float));
    memcpy(bix.mapped, ix, (size_t)dw * supx * sizeof(uint32_t));
    memcpy(bwy.mapped, wy, (size_t)dh * supy * sizeof(float));
    memcpy(biy.mapped, iy, (size_t)dh * supy * sizeof(uint32_t));

    if (alpha_channel >= 0 && alpha_channel < ch) {
        pc_premult pc; pc.n = (uint32_t)(sw * sh); pc.ch = (uint32_t)ch;
        pc.ac = (uint32_t)alpha_channel; pc.undo = 0;
        { vkbuf b[1]; b[0] = bsrc;
          rc = vk_dispatch(c, SH_premult, b, 1, &pc, sizeof(pc), GROUPS(sw * sh)); }
        if (!EXR_OK(rc)) goto done;
    }
    { pc_rh pc; vkbuf b[4];
      pc.sw = (uint32_t)sw; pc.sh = (uint32_t)sh; pc.sstride = (uint32_t)sstride_e;
      pc.dw = (uint32_t)dw; pc.dstride = (uint32_t)dstride_e; pc.ch = (uint32_t)ch;
      pc.support = (uint32_t)supx;
      b[0] = bsrc; b[1] = btmp; b[2] = bwx; b[3] = bix;
      rc = vk_dispatch(c, SH_resize_h, b, 4, &pc, sizeof(pc), GROUPS((long)dw * sh)); }
    if (!EXR_OK(rc)) goto done;
    { pc_rv pc; vkbuf b[4];
      pc.dw = (uint32_t)dw; pc.dh = (uint32_t)dh; pc.sstride = (uint32_t)dstride_e;
      pc.dstride = (uint32_t)dstride_e; pc.ch = (uint32_t)ch; pc.support = (uint32_t)supy;
      b[0] = btmp; b[1] = bdst; b[2] = bwy; b[3] = biy;
      rc = vk_dispatch(c, SH_resize_v, b, 4, &pc, sizeof(pc), GROUPS((long)dw * dh)); }
    if (!EXR_OK(rc)) goto done;
    if (alpha_channel >= 0 && alpha_channel < ch) {
        pc_premult pc; pc.n = (uint32_t)(dw * dh); pc.ch = (uint32_t)ch;
        pc.ac = (uint32_t)alpha_channel; pc.undo = 1;
        { vkbuf b[1]; b[0] = bdst;
          rc = vk_dispatch(c, SH_premult, b, 1, &pc, sizeof(pc), GROUPS(dw * dh)); }
        if (!EXR_OK(rc)) goto done;
    }
    for (y = 0; y < dh; ++y)
        memcpy(dst + (size_t)y * ds, (float *)bdst.mapped + y * dstride_e,
               (size_t)dstride_e * sizeof(float));
done:
    vkbuf_destroy(c, &bsrc); vkbuf_destroy(c, &btmp); vkbuf_destroy(c, &bdst);
    vkbuf_destroy(c, &bwx); vkbuf_destroy(c, &bix);
    vkbuf_destroy(c, &bwy); vkbuf_destroy(c, &biy);
    exr_free(a, wx); exr_free(a, ix); exr_free(a, wy); exr_free(a, iy);
    return rc;
}

/* ===========================================================================
 * Hybrid decode
 * ========================================================================= */
typedef struct { uint32_t n, tile, nblk; } pc_pred;
typedef struct { uint32_t nblk; } pc_off;
typedef struct { uint32_t n; } pc_n;
typedef struct { uint32_t srcOff, sStride, rowLen, rows; } pc_split;

static exr_result gpu_decode_part(exr_vk_context *c, exr_reader *r, int32_t part,
                                  exr_part *out) {
    const exr_header *h = &out->header;
    int nch = h->num_channels, w = out->width, hgt = out->height;
    int dwy0 = h->data_window.min_y;
    uint32_t nblocks = 0, bi_idx;
    size_t scan_stride = 0;
    size_t *ch_off, *ch_row;
    uint8_t *host_canon = NULL;
    size_t maxblock = 0, maxplane = 0;
    vkbuf bcanon = {0}, bpre = {0}, btmp = {0}, bpart = {0}, boff = {0}, bplane = {0};
    exr_result rc;
    int cc;

    ch_off = (size_t *)exr_calloc(&c->alloc, (size_t)nch, sizeof(size_t));
    ch_row = (size_t *)exr_calloc(&c->alloc, (size_t)nch, sizeof(size_t));
    if (!ch_off || !ch_row) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
    for (cc = 0; cc < nch; ++cc) {
        size_t ps = exr_pixel_size(h->channels[cc].pixel_type);
        ch_off[cc] = scan_stride;
        ch_row[cc] = (size_t)w * ps;
        scan_stride += (size_t)w * ps;
        if (ch_row[cc] * (size_t)hgt > maxplane) maxplane = ch_row[cc] * (size_t)hgt;
    }

    rc = exr_reader_num_blocks(r, part, &nblocks);
    if (!EXR_OK(rc)) goto done;

    for (bi_idx = 0; bi_idx < nblocks; ++bi_idx) {
        exr_block_info bi;
        const uint8_t *cdata;
        size_t csize, unc;
        exr_codec_ctx ctx;
        int need_recon;
        uint32_t n_i;

        rc = exr_reader_block_raw(r, part, bi_idx, &bi, &cdata, &csize, &ctx);
        if (rc != EXR_SUCCESS) goto done;
        unc = bi.uncompressed_size;
        n_i = (uint32_t)unc;

        if (unc > maxblock) {
            vkbuf_destroy(c, &bcanon); vkbuf_destroy(c, &bpre);
            vkbuf_destroy(c, &btmp); exr_free(&c->alloc, host_canon);
            host_canon = (uint8_t *)exr_malloc(&c->alloc, unc ? unc : 1);
            if (!host_canon) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
            rc = vkbuf_create(c, unc, &bcanon); if (!EXR_OK(rc)) goto done;
            rc = vkbuf_create(c, unc, &bpre); if (!EXR_OK(rc)) goto done;
            rc = vkbuf_create(c, unc, &btmp); if (!EXR_OK(rc)) goto done;
            maxblock = unc;
        }
        if (bplane.buf == VK_NULL_HANDLE && maxplane > 0) {
            rc = vkbuf_create(c, maxplane, &bplane); if (!EXR_OK(rc)) goto done;
        }

        need_recon = (csize != unc) &&
                     (ctx.compression == EXR_COMPRESSION_ZIP ||
                      ctx.compression == EXR_COMPRESSION_ZIPS ||
                      ctx.compression == EXR_COMPRESSION_RLE);
        if (need_recon) {
            uint32_t tile = 4096, nblk = (n_i + tile - 1) / tile;
            pc_pred pp; pc_off po; pc_n pn;
            if (ctx.compression == EXR_COMPRESSION_RLE)
                rc = exr_rle_expand_only(cdata, csize, host_canon, unc);
            else
                rc = exr_zip_inflate_only(cdata, csize, host_canon, unc);
            if (!EXR_OK(rc)) goto done;
            memcpy(bpre.mapped, host_canon, unc);
            if (nblk * sizeof(uint32_t) > bpart.size) {
                vkbuf_destroy(c, &bpart); vkbuf_destroy(c, &boff);
                rc = vkbuf_create(c, nblk * sizeof(uint32_t), &bpart); if (!EXR_OK(rc)) goto done;
                rc = vkbuf_create(c, nblk * sizeof(uint32_t), &boff); if (!EXR_OK(rc)) goto done;
            }
            pp.n = n_i; pp.tile = tile; pp.nblk = nblk;
            { vkbuf b[2]; b[0] = bpre; b[1] = bpart;
              rc = vk_dispatch(c, SH_pred_partials, b, 2, &pp, sizeof(pp), GROUPS(nblk)); }
            if (!EXR_OK(rc)) goto done;
            po.nblk = nblk;
            { vkbuf b[2]; b[0] = bpart; b[1] = boff;
              rc = vk_dispatch(c, SH_pred_offsets, b, 2, &po, sizeof(po), 1); }
            if (!EXR_OK(rc)) goto done;
            { vkbuf b[3]; b[0] = bpre; b[1] = boff; b[2] = btmp; /* btmp = predicted */
              rc = vk_dispatch(c, SH_pred_apply, b, 3, &pp, sizeof(pp), GROUPS(nblk)); }
            if (!EXR_OK(rc)) goto done;
            pn.n = n_i;
            { vkbuf b[2]; b[0] = btmp; b[1] = bcanon;
              rc = vk_dispatch(c, SH_deinterleave, b, 2, &pn, sizeof(pn), GROUPS((unc + 3) / 4)); }
            if (!EXR_OK(rc)) goto done;
        } else {
            if (csize == unc || ctx.compression == EXR_COMPRESSION_NONE)
                memcpy(host_canon, cdata, unc);
            else {
                rc = exr_decompress_block(&ctx, cdata, csize, host_canon, unc);
                if (!EXR_OK(rc)) goto done;
            }
            memcpy(bcanon.mapped, host_canon, unc);
        }

        for (cc = 0; cc < nch; ++cc) {
            pc_split sp;
            size_t row_start = (size_t)(bi.y0 - dwy0);
            size_t plane_bytes = ch_row[cc] * (size_t)bi.height;
            sp.srcOff = (uint32_t)ch_off[cc];
            sp.sStride = (uint32_t)scan_stride;
            sp.rowLen = (uint32_t)ch_row[cc];
            sp.rows = (uint32_t)bi.height;
            { vkbuf b[2]; b[0] = bcanon; b[1] = bplane;
              rc = vk_dispatch(c, SH_channel_split, b, 2, &sp, sizeof(sp),
                               GROUPS((plane_bytes + 3) / 4)); }
            if (!EXR_OK(rc)) goto done;
            memcpy((uint8_t *)out->images[cc] + row_start * ch_row[cc],
                   bplane.mapped, plane_bytes);
        }
    }
    rc = EXR_SUCCESS;
done:
    vkbuf_destroy(c, &bcanon); vkbuf_destroy(c, &bpre); vkbuf_destroy(c, &btmp);
    vkbuf_destroy(c, &bpart); vkbuf_destroy(c, &boff); vkbuf_destroy(c, &bplane);
    exr_free(&c->alloc, host_canon);
    exr_free(&c->alloc, ch_off);
    exr_free(&c->alloc, ch_row);
    return rc;
}

static void copy_header(exr_header *dst, const exr_header *src) {
    *dst = *src;
    dst->channels = NULL;
    dst->attrs = NULL;
}

exr_result exr_vk_load_from_memory(exr_vk_context *c, const void *data,
                                   size_t size, const exr_allocator *alloc,
                                   exr_image *out) {
    exr_reader *r = NULL;
    exr_result rc;
    int np, p, eligible = 1;
    const exr_allocator *A;
    if (!out) return EXR_ERROR_INVALID_ARGUMENT;
    if (!c || !exr_vk_available()) return exr_load_from_memory(data, size, alloc, out);
    A = alloc ? alloc : exr_default_allocator();
    rc = exr_reader_open_memory(data, size, alloc, &r);
    if (!EXR_OK(rc)) return rc;
    rc = exr_reader_parse_header(r);
    if (!EXR_OK(rc)) { exr_reader_close(r); return rc; }
    np = exr_reader_num_parts(r);
    for (p = 0; p < np; ++p)
        if (!part_vk_eligible(exr_reader_part_header(r, p))) { eligible = 0; break; }
    if (!eligible || np <= 0) {
        exr_reader_close(r);
        return exr_load_from_memory(data, size, alloc, out);
    }
    memset(out, 0, sizeof(*out));
    if (A) out->alloc = *A;
    out->num_parts = np;
    out->parts = (exr_part *)exr_calloc(A, (size_t)np, sizeof(exr_part));
    if (!out->parts) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }
    for (p = 0; p < np; ++p) {
        const exr_header *h = exr_reader_part_header(r, p);
        exr_part *op = &out->parts[p];
        int nch = h->num_channels, cc;
        copy_header(&op->header, h);
        op->width = h->data_window.max_x - h->data_window.min_x + 1;
        op->height = h->data_window.max_y - h->data_window.min_y + 1;
        op->header.num_channels = nch;
        op->header.channels = (exr_channel *)exr_calloc(A, (size_t)nch, sizeof(exr_channel));
        op->images = (void **)exr_calloc(A, (size_t)nch, sizeof(void *));
        if (!op->header.channels || !op->images) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }
        for (cc = 0; cc < nch; ++cc) {
            size_t ps = exr_pixel_size(h->channels[cc].pixel_type);
            op->header.channels[cc] = h->channels[cc];
            op->images[cc] = exr_malloc(A, (size_t)op->width * op->height * ps + 1);
            if (!op->images[cc]) { rc = EXR_ERROR_OUT_OF_MEMORY; goto fail; }
        }
        rc = gpu_decode_part(c, r, p, op);
        if (!EXR_OK(rc)) goto fail;
    }
    exr_reader_close(r);
    return EXR_SUCCESS;
fail:
    exr_reader_close(r);
    exr_image_free(out);
    return rc;
}

exr_result exr_vk_load_from_file(exr_vk_context *c, const char *path,
                                 const exr_allocator *alloc, exr_image *out) {
    FILE *f;
    long sz;
    void *buf;
    exr_result rc;
    const exr_allocator *A;
    if (!path || !out) return EXR_ERROR_INVALID_ARGUMENT;
    if (!c || !exr_vk_available()) return exr_load_from_file(path, alloc, out);
    A = alloc ? alloc : exr_default_allocator();
    f = fopen(path, "rb");
    if (!f) return EXR_ERROR_IO;
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); return EXR_ERROR_IO; }
    sz = ftell(f);
    if (sz < 0) { fclose(f); return EXR_ERROR_IO; }
    rewind(f);
    buf = exr_malloc(A, (size_t)sz ? (size_t)sz : 1);
    if (!buf) { fclose(f); return EXR_ERROR_OUT_OF_MEMORY; }
    if (fread(buf, 1, (size_t)sz, f) != (size_t)sz) {
        fclose(f); exr_free(A, buf); return EXR_ERROR_IO;
    }
    fclose(f);
    rc = exr_vk_load_from_memory(c, buf, (size_t)sz, alloc, out);
    exr_free(A, buf);
    return rc;
}

/* ===========================================================================
 * Hybrid encode (GPU gathers canonical scanline blocks; CPU entropy+assembly)
 * ========================================================================= */
typedef struct { uint32_t scanStride, nch, blockBytes; } pc_gthr;

typedef struct {
    uint8_t *buf;
    size_t len, cap, pos;
    const exr_allocator *a;
    int err;
} mem_sink;
static exr_result mem_sink_write(void *user, const void *data, size_t n) {
    mem_sink *s = (mem_sink *)user;
    if (s->pos + n > s->cap) {
        size_t nc = s->cap ? s->cap * 2 : 65536;
        uint8_t *nb;
        while (nc < s->pos + n) nc *= 2;
        nb = (uint8_t *)exr_malloc(s->a, nc);
        if (!nb) { s->err = 1; return EXR_ERROR_OUT_OF_MEMORY; }
        if (s->buf) { memcpy(nb, s->buf, s->len); exr_free(s->a, s->buf); }
        s->buf = nb; s->cap = nc;
    }
    memcpy(s->buf + s->pos, data, n);
    s->pos += n;
    if (s->pos > s->len) s->len = s->pos;
    return EXR_SUCCESS;
}
static exr_result mem_sink_seek(void *user, uint64_t off) {
    ((mem_sink *)user)->pos = (size_t)off;
    return EXR_SUCCESS;
}

/* GPU gather of one part's scanline blocks into canonical form -> CPU writer. */
static exr_result gpu_encode_parts(exr_vk_context *c, exr_writer *w,
                                   const exr_image *img, exr_compression comp) {
    int np = img->num_parts, p;
    exr_result rc = EXR_SUCCESS;
    for (p = 0; p < np; ++p) {
        const exr_part *pt = &img->parts[p];
        const exr_header *h = &pt->header;
        const int *order = exr_writer_sorted_order(w, p);
        int nch = h->num_channels, oi;
        int lpb = exr_lines_per_block(comp);
        int ymin = h->data_window.min_y, ymax = h->data_window.max_y, y0;
        int W = pt->width, Hh = pt->height;
        size_t scan_stride = 0;
        size_t *src_base, *ch_off, *ch_row;
        uint32_t *layout;
        size_t allbytes = 0;
        vkbuf bsrc = {0}, blay = {0}, bblock = {0};
        uint8_t *host_block = NULL;
        size_t maxblk = 0;

        src_base = (size_t *)exr_calloc(&c->alloc, (size_t)nch, sizeof(size_t));
        ch_off = (size_t *)exr_calloc(&c->alloc, (size_t)nch, sizeof(size_t));
        ch_row = (size_t *)exr_calloc(&c->alloc, (size_t)nch, sizeof(size_t));
        layout = (uint32_t *)exr_calloc(&c->alloc, (size_t)nch * 3, sizeof(uint32_t));
        if (!src_base || !ch_off || !ch_row || !layout) { rc = EXR_ERROR_OUT_OF_MEMORY; goto cleanp; }
        /* concatenate all channels (sorted order) into one source buffer */
        for (oi = 0; oi < nch; ++oi) {
            int cc = order ? order[oi] : oi;
            size_t ps = exr_pixel_size(h->channels[cc].pixel_type);
            src_base[oi] = allbytes;
            ch_off[oi] = scan_stride;
            ch_row[oi] = (size_t)W * ps;
            scan_stride += (size_t)W * ps;
            allbytes += (size_t)W * Hh * ps;
        }
        rc = vkbuf_create(c, allbytes, &bsrc); if (!EXR_OK(rc)) goto cleanp;
        for (oi = 0; oi < nch; ++oi) {
            int cc = order ? order[oi] : oi;
            size_t ps = exr_pixel_size(h->channels[cc].pixel_type);
            memcpy((uint8_t *)bsrc.mapped + src_base[oi], pt->images[cc],
                   (size_t)W * Hh * ps);
            layout[oi * 3 + 0] = (uint32_t)src_base[oi];
            layout[oi * 3 + 1] = (uint32_t)ch_off[oi];
            layout[oi * 3 + 2] = (uint32_t)ch_row[oi];
        }
        rc = vkbuf_create(c, (size_t)nch * 3 * sizeof(uint32_t), &blay); if (!EXR_OK(rc)) goto cleanp;
        memcpy(blay.mapped, layout, (size_t)nch * 3 * sizeof(uint32_t));

        for (y0 = ymin; y0 <= ymax; y0 += lpb) {
            int nlines = (y0 + lpb - 1 > ymax) ? (ymax - y0 + 1) : lpb;
            size_t blk_size;
            pc_gthr pc;
            int row_start = y0 - ymin;
            rc = exr_block_uncompressed_size(h->channels, nch, h->data_window.min_x,
                                             y0, W, nlines, &blk_size);
            if (!EXR_OK(rc)) goto cleanp;
            if (blk_size > maxblk) {
                vkbuf_destroy(c, &bblock); exr_free(&c->alloc, host_block);
                rc = vkbuf_create(c, blk_size, &bblock); if (!EXR_OK(rc)) goto cleanp;
                host_block = (uint8_t *)exr_malloc(&c->alloc, blk_size ? blk_size : 1);
                if (!host_block) { rc = EXR_ERROR_OUT_OF_MEMORY; goto cleanp; }
                maxblk = blk_size;
            }
            /* per-block source view starts at this block's first row; pass a
             * layout whose srcBase points at the block's first row of each ch */
            {
                uint32_t *bl = (uint32_t *)blay.mapped;
                for (oi = 0; oi < nch; ++oi)
                    bl[oi * 3 + 0] = (uint32_t)(src_base[oi] + (size_t)row_start * ch_row[oi]);
            }
            pc.scanStride = (uint32_t)scan_stride;
            pc.nch = (uint32_t)nch;
            pc.blockBytes = (uint32_t)blk_size;
            { vkbuf b[3]; b[0] = bblock; b[1] = bsrc; b[2] = blay;
              rc = vk_dispatch(c, SH_channel_gather, b, 3, &pc, sizeof(pc),
                               GROUPS((blk_size + 3) / 4)); }
            if (!EXR_OK(rc)) goto cleanp;
            memcpy(host_block, bblock.mapped, blk_size);
            rc = exr_writer_write_scanline_block_canon(w, p, y0, host_block, blk_size);
            if (!EXR_OK(rc)) goto cleanp;
        }
    cleanp:
        vkbuf_destroy(c, &bsrc); vkbuf_destroy(c, &blay); vkbuf_destroy(c, &bblock);
        exr_free(&c->alloc, host_block);
        exr_free(&c->alloc, src_base); exr_free(&c->alloc, ch_off);
        exr_free(&c->alloc, ch_row); exr_free(&c->alloc, layout);
        if (!EXR_OK(rc)) return rc;
    }
    return EXR_SUCCESS;
}

exr_result exr_vk_save_to_memory(exr_vk_context *c, void **out_data,
                                 size_t *out_size, const exr_allocator *alloc,
                                 const exr_image *img, exr_compression comp) {
    exr_writer *w = NULL;
    exr_result rc;
    int np, p;
    mem_sink ms;
    exr_data_sink sink;
    const exr_allocator *A;
    if (!out_data || !out_size || !img) return EXR_ERROR_INVALID_ARGUMENT;
    if (!c || !exr_vk_available() || !image_vk_eligible(img))
        return exr_save_to_memory(out_data, out_size, alloc, img, comp);
    A = alloc ? alloc : exr_default_allocator();
    np = img->num_parts;
    memset(&ms, 0, sizeof(ms));
    ms.a = A;
    sink.user = &ms; sink.write = mem_sink_write; sink.seek = mem_sink_seek; sink.close = NULL;
    rc = exr_writer_create(alloc, &w);
    if (!EXR_OK(rc)) return rc;
    for (p = 0; p < np; ++p) {
        rc = exr_writer_add_part(w, &img->parts[p].header, NULL);
        if (!EXR_OK(rc)) goto done;
    }
    rc = exr_writer_begin_stream(w, &sink, comp);
    if (!EXR_OK(rc)) goto done;
    rc = gpu_encode_parts(c, w, img, comp);
    if (!EXR_OK(rc)) goto done;
    rc = exr_writer_end_stream(w);
    if (!EXR_OK(rc)) goto done;
    if (ms.err) { rc = EXR_ERROR_OUT_OF_MEMORY; goto done; }
    *out_data = ms.buf; *out_size = ms.len; ms.buf = NULL;
done:
    exr_writer_destroy(w);
    if (ms.buf) exr_free(A, ms.buf);
    return rc;
}

exr_result exr_vk_save_to_file(exr_vk_context *c, const char *path,
                               const exr_image *img, exr_compression comp) {
    exr_writer *w = NULL;
    exr_result rc;
    int np, p;
    if (!path || !img) return EXR_ERROR_INVALID_ARGUMENT;
    if (!c || !exr_vk_available() || !image_vk_eligible(img))
        return exr_save_to_file(path, img, comp);
    np = img->num_parts;
    rc = exr_writer_create(NULL, &w);
    if (!EXR_OK(rc)) return rc;
    for (p = 0; p < np; ++p) {
        rc = exr_writer_add_part(w, &img->parts[p].header, NULL);
        if (!EXR_OK(rc)) { exr_writer_destroy(w); return rc; }
    }
    rc = exr_writer_begin_stream_file(w, path, comp);
    if (!EXR_OK(rc)) { exr_writer_destroy(w); return rc; }
    rc = gpu_encode_parts(c, w, img, comp);
    if (EXR_OK(rc)) rc = exr_writer_end_stream(w);
    exr_writer_destroy(w);
    return rc;
}

#endif /* EXR_USE_VULKAN */
