/*
 * Copyright 2014 Advanced Micro Devices, Inc.
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
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <sys/ioctl.h>
#if HAVE_ALLOCA_H
# include <alloca.h>
#endif

#include "xf86drm.h"
#include "amdgpu_drm.h"
#include "amdgpu_internal.h"

static int amdgpu_cs_unreference_sem(amdgpu_semaphore_handle sem);
static int amdgpu_cs_reset_sem(amdgpu_semaphore_handle sem);

/**
 * Create command submission context
 *
 * \param   dev      - \c [in] Device handle. See #amdgpu_device_initialize()
 * \param   priority - \c [in] Context creation flags. See AMDGPU_CTX_PRIORITY_*
 * \param   context  - \c [out] GPU Context handle
 *
 * \return  0 on success otherwise POSIX Error code
*/
drm_public int amdgpu_cs_ctx_create2(amdgpu_device_handle dev,
				     uint32_t priority,
				     amdgpu_context_handle *context)
{
	struct amdgpu_context *gpu_context;
	union drm_amdgpu_ctx args;
	int i, j, k;
	int r;
	char *override_priority;

	if (!dev || !context)
		return -EINVAL;

	override_priority = getenv("AMD_PRIORITY");
	if (override_priority) {
		/* The priority is a signed integer. The variable type is
		 * wrong. If parsing fails, priority is unchanged.
		 */
		if (sscanf(override_priority, "%i", &priority) == 1) {
			printf("amdgpu: context priority changed to %i\n",
			       priority);
		}
	}

	gpu_context = calloc(1, sizeof(struct amdgpu_context));
	if (!gpu_context)
		return -ENOMEM;

	gpu_context->dev = dev;

	r = pthread_mutex_init(&gpu_context->sequence_mutex, NULL);
	if (r)
		goto error;

	/* Create the context */
	memset(&args, 0, sizeof(args));
	args.in.op = AMDGPU_CTX_OP_ALLOC_CTX;
	args.in.priority = priority;

	r = drmCommandWriteRead(dev->fd, DRM_AMDGPU_CTX, &args, sizeof(args));
	if (r)
		goto error;

	gpu_context->id = args.out.alloc.ctx_id;
	for (i = 0; i < AMDGPU_HW_IP_NUM; i++)
		for (j = 0; j < AMDGPU_HW_IP_INSTANCE_MAX_COUNT; j++)
			for (k = 0; k < AMDGPU_CS_MAX_RINGS; k++)
				list_inithead(&gpu_context->sem_list[i][j][k]);
	*context = (amdgpu_context_handle)gpu_context;

	return 0;

error:
	pthread_mutex_destroy(&gpu_context->sequence_mutex);
	free(gpu_context);
	return r;
}

drm_public int amdgpu_cs_ctx_create(amdgpu_device_handle dev,
				    amdgpu_context_handle *context)
{
	return amdgpu_cs_ctx_create2(dev, AMDGPU_CTX_PRIORITY_NORMAL, context);
}

/**
 * Release command submission context
 *
 * \param   dev - \c [in] amdgpu device handle
 * \param   context - \c [in] amdgpu context handle
 *
 * \return  0 on success otherwise POSIX Error code
*/
drm_public int amdgpu_cs_ctx_free(amdgpu_context_handle context)
{
	union drm_amdgpu_ctx args;
	int i, j, k;
	int r;

	if (!context)
		return -EINVAL;

	pthread_mutex_destroy(&context->sequence_mutex);

	/* now deal with kernel side */
	memset(&args, 0, sizeof(args));
	args.in.op = AMDGPU_CTX_OP_FREE_CTX;
	args.in.ctx_id = context->id;
	r = drmCommandWriteRead(context->dev->fd, DRM_AMDGPU_CTX,
				&args, sizeof(args));
	for (i = 0; i < AMDGPU_HW_IP_NUM; i++) {
		for (j = 0; j < AMDGPU_HW_IP_INSTANCE_MAX_COUNT; j++) {
			for (k = 0; k < AMDGPU_CS_MAX_RINGS; k++) {
				amdgpu_semaphore_handle sem, tmp;
				LIST_FOR_EACH_ENTRY_SAFE(sem, tmp, &context->sem_list[i][j][k], list) {
					list_del(&sem->list);
					amdgpu_cs_reset_sem(sem);
					amdgpu_cs_unreference_sem(sem);
				}
			}
		}
	}
	free(context);

	return r;
}

drm_public int amdgpu_cs_ctx_override_priority(amdgpu_device_handle dev,
                                               amdgpu_context_handle context,
                                               int master_fd,
                                               unsigned priority)
{
	union drm_amdgpu_sched args;
	int r;

	if (!dev || !context || master_fd < 0)
		return -EINVAL;

	memset(&args, 0, sizeof(args));

	args.in.op = AMDGPU_SCHED_OP_CONTEXT_PRIORITY_OVERRIDE;
	args.in.fd = dev->fd;
	args.in.priority = priority;
	args.in.ctx_id = context->id;

	r = drmCommandWrite(master_fd, DRM_AMDGPU_SCHED, &args, sizeof(args));
	if (r)
		return r;

	return 0;
}

drm_public int amdgpu_cs_ctx_stable_pstate(amdgpu_context_handle context,
					   uint32_t op,
					   uint32_t flags,
					   uint32_t *out_flags)
{
	union drm_amdgpu_ctx args;
	int r;

	if (!context)
		return -EINVAL;

	memset(&args, 0, sizeof(args));
	args.in.op = op;
	args.in.ctx_id = context->id;
	args.in.flags = flags;
	r = drmCommandWriteRead(context->dev->fd, DRM_AMDGPU_CTX,
				&args, sizeof(args));
	if (!r && out_flags)
		*out_flags = args.out.pstate.flags;
	return r;
}

drm_public int amdgpu_cs_query_reset_state(amdgpu_context_handle context,
					   uint32_t *state, uint32_t *hangs)
{
	union drm_amdgpu_ctx args;
	int r;

	if (!context)
		return -EINVAL;

	memset(&args, 0, sizeof(args));
	args.in.op = AMDGPU_CTX_OP_QUERY_STATE;
	args.in.ctx_id = context->id;
	r = drmCommandWriteRead(context->dev->fd, DRM_AMDGPU_CTX,
				&args, sizeof(args));
	if (!r) {
		*state = args.out.state.reset_status;
		*hangs = args.out.state.hangs;
	}
	return r;
}

drm_public int amdgpu_cs_query_reset_state2(amdgpu_context_handle context,
					    uint64_t *flags)
{
	union drm_amdgpu_ctx args;
	int r;

	if (!context)
		return -EINVAL;

	memset(&args, 0, sizeof(args));
	args.in.op = AMDGPU_CTX_OP_QUERY_STATE2;
	args.in.ctx_id = context->id;
	r = drmCommandWriteRead(context->dev->fd, DRM_AMDGPU_CTX,
				&args, sizeof(args));
	if (!r)
		*flags = args.out.state.flags;
	return r;
}

/**
 * Submit command to kernel DRM
 * \param   dev - \c [in]  Device handle
 * \param   context - \c [in]  GPU Context
 * \param   ibs_request - \c [in]  Pointer to submission requests
 * \param   fence - \c [out] return fence for this submission
 *
 * \return  0 on success otherwise POSIX Error code
 * \sa amdgpu_cs_submit()
*/
static int amdgpu_cs_submit_one(amdgpu_context_handle context,
				struct amdgpu_cs_request *ibs_request)
{
	struct drm_amdgpu_cs_chunk *chunks;
	struct drm_amdgpu_cs_chunk_data *chunk_data;
	struct drm_amdgpu_cs_chunk_dep *dependencies = NULL;
	struct drm_amdgpu_cs_chunk_dep *sem_dependencies = NULL;
	amdgpu_device_handle dev = context->dev;
	struct list_head *sem_list;
	amdgpu_semaphore_handle sem, tmp;
	uint32_t i, size, num_chunks, bo_list_handle = 0, sem_count = 0;
	uint64_t seq_no;
	bool user_fence;
	int r = 0;

	if (ibs_request->ip_type >= AMDGPU_HW_IP_NUM)
		return -EINVAL;
	if (ibs_request->ring >= AMDGPU_CS_MAX_RINGS)
		return -EINVAL;
	if (ibs_request->number_of_ibs == 0) {
		ibs_request->seq_no = AMDGPU_NULL_SUBMIT_SEQ;
		return 0;
	}
	user_fence = (ibs_request->fence_info.handle != NULL);

	size = ibs_request->number_of_ibs + (user_fence ? 2 : 1) + 1;

	chunks = alloca(sizeof(struct drm_amdgpu_cs_chunk) * size);

	size = ibs_request->number_of_ibs + (user_fence ? 1 : 0);

	chunk_data = alloca(sizeof(struct drm_amdgpu_cs_chunk_data) * size);

	if (ibs_request->resources)
		bo_list_handle = ibs_request->resources->handle;
	num_chunks = ibs_request->number_of_ibs;
	/* IB chunks */
	for (i = 0; i < ibs_request->number_of_ibs; i++) {
		struct amdgpu_cs_ib_info *ib;
		chunks[i].chunk_id = AMDGPU_CHUNK_ID_IB;
		chunks[i].length_dw = sizeof(struct drm_amdgpu_cs_chunk_ib) / 4;
		chunks[i].chunk_data = (uint64_t)(uintptr_t)&chunk_data[i];

		ib = &ibs_request->ibs[i];

		chunk_data[i].ib_data._pad = 0;
		chunk_data[i].ib_data.va_start = ib->ib_mc_address;
		chunk_data[i].ib_data.ib_bytes = ib->size * 4;
		chunk_data[i].ib_data.ip_type = ibs_request->ip_type;
		chunk_data[i].ib_data.ip_instance = ibs_request->ip_instance;
		chunk_data[i].ib_data.ring = ibs_request->ring;
		chunk_data[i].ib_data.flags = ib->flags;
	}

	pthread_mutex_lock(&context->sequence_mutex);

	if (user_fence) {
		i = num_chunks++;

		/* fence chunk */
		chunks[i].chunk_id = AMDGPU_CHUNK_ID_FENCE;
		chunks[i].length_dw = sizeof(struct drm_amdgpu_cs_chunk_fence) / 4;
		chunks[i].chunk_data = (uint64_t)(uintptr_t)&chunk_data[i];

		/* fence bo handle */
		chunk_data[i].fence_data.handle = ibs_request->fence_info.handle->handle;
		/* offset */
		chunk_data[i].fence_data.offset = 
			ibs_request->fence_info.offset * sizeof(uint64_t);
	}

	if (ibs_request->number_of_dependencies) {
		dependencies = alloca(sizeof(struct drm_amdgpu_cs_chunk_dep) *
			ibs_request->number_of_dependencies);
		if (!dependencies) {
			r = -ENOMEM;
			goto error_unlock;
		}

		for (i = 0; i < ibs_request->number_of_dependencies; ++i) {
			struct amdgpu_cs_fence *info = &ibs_request->dependencies[i];
			struct drm_amdgpu_cs_chunk_dep *dep = &dependencies[i];
			dep->ip_type = info->ip_type;
			dep->ip_instance = info->ip_instance;
			dep->ring = info->ring;
			dep->ctx_id = info->context->id;
			dep->handle = info->fence;
		}

		i = num_chunks++;

		/* dependencies chunk */
		chunks[i].chunk_id = AMDGPU_CHUNK_ID_DEPENDENCIES;
		chunks[i].length_dw = sizeof(struct drm_amdgpu_cs_chunk_dep) / 4
			* ibs_request->number_of_dependencies;
		chunks[i].chunk_data = (uint64_t)(uintptr_t)dependencies;
	}

	sem_list = &context->sem_list[ibs_request->ip_type][ibs_request->ip_instance][ibs_request->ring];
	LIST_FOR_EACH_ENTRY(sem, sem_list, list)
		sem_count++;
	if (sem_count) {
		sem_dependencies = alloca(sizeof(struct drm_amdgpu_cs_chunk_dep) * sem_count);
		if (!sem_dependencies) {
			r = -ENOMEM;
			goto error_unlock;
		}
		sem_count = 0;
		LIST_FOR_EACH_ENTRY_SAFE(sem, tmp, sem_list, list) {
			struct amdgpu_cs_fence *info = &sem->signal_fence;
			struct drm_amdgpu_cs_chunk_dep *dep = &sem_dependencies[sem_count++];
			dep->ip_type = info->ip_type;
			dep->ip_instance = info->ip_instance;
			dep->ring = info->ring;
			dep->ctx_id = info->context->id;
			dep->handle = info->fence;

			list_del(&sem->list);
			amdgpu_cs_reset_sem(sem);
			amdgpu_cs_unreference_sem(sem);
		}
		i = num_chunks++;

		/* dependencies chunk */
		chunks[i].chunk_id = AMDGPU_CHUNK_ID_DEPENDENCIES;
		chunks[i].length_dw = sizeof(struct drm_amdgpu_cs_chunk_dep) / 4 * sem_count;
		chunks[i].chunk_data = (uint64_t)(uintptr_t)sem_dependencies;
	}

	r = amdgpu_cs_submit_raw2(dev, context, bo_list_handle, num_chunks,
				  chunks, &seq_no);
	if (r)
		goto error_unlock;

	ibs_request->seq_no = seq_no;
	context->last_seq[ibs_request->ip_type][ibs_request->ip_instance][ibs_request->ring] = ibs_request->seq_no;
error_unlock:
	pthread_mutex_unlock(&context->sequence_mutex);
	return r;
}

drm_public int amdgpu_cs_submit(amdgpu_context_handle context,
				uint64_t flags,
				struct amdgpu_cs_request *ibs_request,
				uint32_t number_of_requests)
{
	uint32_t i;
	int r;

	if (!context || !ibs_request)
		return -EINVAL;

	r = 0;
	for (i = 0; i < number_of_requests; i++) {
		r = amdgpu_cs_submit_one(context, ibs_request);
		if (r)
			break;
		ibs_request++;
	}

	return r;
}

/**
 * Calculate absolute timeout.
 *
 * \param   timeout - \c [in] timeout in nanoseconds.
 *
 * \return  absolute timeout in nanoseconds
*/
drm_private uint64_t amdgpu_cs_calculate_timeout(uint64_t timeout)
{
	int r;

	if (timeout != AMDGPU_TIMEOUT_INFINITE) {
		struct timespec current;
		uint64_t current_ns;
		r = clock_gettime(CLOCK_MONOTONIC, &current);
		if (r) {
			fprintf(stderr, "clock_gettime() returned error (%d)!", errno);
			return AMDGPU_TIMEOUT_INFINITE;
		}

		current_ns = ((uint64_t)current.tv_sec) * 1000000000ull;
		current_ns += current.tv_nsec;
		timeout += current_ns;
		if (timeout < current_ns)
			timeout = AMDGPU_TIMEOUT_INFINITE;
	}
	return timeout;
}

static int amdgpu_ioctl_wait_cs(amdgpu_context_handle context,
				unsigned ip,
				unsigned ip_instance,
				uint32_t ring,
				uint64_t handle,
				uint64_t timeout_ns,
				uint64_t flags,
				bool *busy)
{
	amdgpu_device_handle dev = context->dev;
	union drm_amdgpu_wait_cs args;
	int r;

	memset(&args, 0, sizeof(args));
	args.in.handle = handle;
	args.in.ip_type = ip;
	args.in.ip_instance = ip_instance;
	args.in.ring = ring;
	args.in.ctx_id = context->id;

	if (flags & AMDGPU_QUERY_FENCE_TIMEOUT_IS_ABSOLUTE)
		args.in.timeout = timeout_ns;
	else
		args.in.timeout = amdgpu_cs_calculate_timeout(timeout_ns);

	r = drmIoctl(dev->fd, DRM_IOCTL_AMDGPU_WAIT_CS, &args);
	if (r)
		return -errno;

	*busy = args.out.status;
	return 0;
}

drm_public int amdgpu_cs_query_fence_status(struct amdgpu_cs_fence *fence,
					    uint64_t timeout_ns,
					    uint64_t flags,
					    uint32_t *expired)
{
	bool busy = true;
	int r;

	if (!fence || !expired || !fence->context)
		return -EINVAL;
	if (fence->ip_type >= AMDGPU_HW_IP_NUM)
		return -EINVAL;
	if (fence->ring >= AMDGPU_CS_MAX_RINGS)
		return -EINVAL;
	if (fence->fence == AMDGPU_NULL_SUBMIT_SEQ) {
		*expired = true;
		return 0;
	}

	*expired = false;

	r = amdgpu_ioctl_wait_cs(fence->context, fence->ip_type,
				fence->ip_instance, fence->ring,
			       	fence->fence, timeout_ns, flags, &busy);

	if (!r && !busy)
		*expired = true;

	return r;
}

static int amdgpu_ioctl_wait_fences(struct amdgpu_cs_fence *fences,
				    uint32_t fence_count,
				    bool wait_all,
				    uint64_t timeout_ns,
				    uint32_t *status,
				    uint32_t *first)
{
	struct drm_amdgpu_fence *drm_fences;
	amdgpu_device_handle dev = fences[0].context->dev;
	union drm_amdgpu_wait_fences args;
	int r;
	uint32_t i;

	drm_fences = alloca(sizeof(struct drm_amdgpu_fence) * fence_count);
	for (i = 0; i < fence_count; i++) {
		drm_fences[i].ctx_id = fences[i].context->id;
		drm_fences[i].ip_type = fences[i].ip_type;
		drm_fences[i].ip_instance = fences[i].ip_instance;
		drm_fences[i].ring = fences[i].ring;
		drm_fences[i].seq_no = fences[i].fence;
	}

	memset(&args, 0, sizeof(args));
	args.in.fences = (uint64_t)(uintptr_t)drm_fences;
	args.in.fence_count = fence_count;
	args.in.wait_all = wait_all;
	args.in.timeout_ns = amdgpu_cs_calculate_timeout(timeout_ns);

	r = drmIoctl(dev->fd, DRM_IOCTL_AMDGPU_WAIT_FENCES, &args);
	if (r)
		return -errno;

	*status = args.out.status;

	if (first)
		*first = args.out.first_signaled;

	return 0;
}

drm_public int amdgpu_cs_wait_fences(struct amdgpu_cs_fence *fences,
				     uint32_t fence_count,
				     bool wait_all,
				     uint64_t timeout_ns,
				     uint32_t *status,
				     uint32_t *first)
{
	uint32_t i;

	/* Sanity check */
	if (!fences || !status || !fence_count)
		return -EINVAL;

	for (i = 0; i < fence_count; i++) {
		if (NULL == fences[i].context)
			return -EINVAL;
		if (fences[i].ip_type >= AMDGPU_HW_IP_NUM)
			return -EINVAL;
		if (fences[i].ring >= AMDGPU_CS_MAX_RINGS)
			return -EINVAL;
	}

	*status = 0;

	return amdgpu_ioctl_wait_fences(fences, fence_count, wait_all,
					timeout_ns, status, first);
}

drm_public int amdgpu_cs_create_semaphore(amdgpu_semaphore_handle *sem)
{
	struct amdgpu_semaphore *gpu_semaphore;

	if (!sem)
		return -EINVAL;

	gpu_semaphore = calloc(1, sizeof(struct amdgpu_semaphore));
	if (!gpu_semaphore)
		return -ENOMEM;

	atomic_set(&gpu_semaphore->refcount, 1);
	*sem = gpu_semaphore;

	return 0;
}

drm_public int amdgpu_cs_signal_semaphore(amdgpu_context_handle ctx,
					  uint32_t ip_type,
			       uint32_t ip_instance,
			       uint32_t ring,
			       amdgpu_semaphore_handle sem)
{
	int ret;

	if (!ctx || !sem)
		return -EINVAL;
	if (ip_type >= AMDGPU_HW_IP_NUM)
		return -EINVAL;
	if (ring >= AMDGPU_CS_MAX_RINGS)
		return -EINVAL;

	pthread_mutex_lock(&ctx->sequence_mutex);
	/* sem has been signaled */
	if (sem->signal_fence.context) {
		ret = -EINVAL;
		goto unlock;
	}
	sem->signal_fence.context = ctx;
	sem->signal_fence.ip_type = ip_type;
	sem->signal_fence.ip_instance = ip_instance;
	sem->signal_fence.ring = ring;
	sem->signal_fence.fence = ctx->last_seq[ip_type][ip_instance][ring];
	update_references(NULL, &sem->refcount);
	ret = 0;
unlock:
	pthread_mutex_unlock(&ctx->sequence_mutex);
	return ret;
}

drm_public int amdgpu_cs_wait_semaphore(amdgpu_context_handle ctx,
					uint32_t ip_type,
			     uint32_t ip_instance,
			     uint32_t ring,
			     amdgpu_semaphore_handle sem)
{
	if (!ctx || !sem)
		return -EINVAL;
	if (ip_type >= AMDGPU_HW_IP_NUM)
		return -EINVAL;
	if (ring >= AMDGPU_CS_MAX_RINGS)
		return -EINVAL;
	/* must signal first */
	if (!sem->signal_fence.context)
		return -EINVAL;

	pthread_mutex_lock(&ctx->sequence_mutex);
	list_add(&sem->list, &ctx->sem_list[ip_type][ip_instance][ring]);
	pthread_mutex_unlock(&ctx->sequence_mutex);
	return 0;
}

static int amdgpu_cs_reset_sem(amdgpu_semaphore_handle sem)
{
	if (!sem || !sem->signal_fence.context)
		return -EINVAL;

	sem->signal_fence.context = NULL;
	sem->signal_fence.ip_type = 0;
	sem->signal_fence.ip_instance = 0;
	sem->signal_fence.ring = 0;
	sem->signal_fence.fence = 0;

	return 0;
}

static int amdgpu_cs_unreference_sem(amdgpu_semaphore_handle sem)
{
	if (!sem)
		return -EINVAL;

	if (update_references(&sem->refcount, NULL))
		free(sem);
	return 0;
}

drm_public int amdgpu_cs_destroy_semaphore(amdgpu_semaphore_handle sem)
{
	return amdgpu_cs_unreference_sem(sem);
}

drm_public int amdgpu_cs_create_syncobj2(amdgpu_device_handle dev,
					 uint32_t  flags,
					 uint32_t *handle)
{
	if (NULL == dev)
		return -EINVAL;

	return drmSyncobjCreate(dev->fd, flags, handle);
}

drm_public int amdgpu_cs_create_syncobj(amdgpu_device_handle dev,
					uint32_t *handle)
{
	if (NULL == dev)
		return -EINVAL;

	return drmSyncobjCreate(dev->fd, 0, handle);
}

drm_public int amdgpu_cs_destroy_syncobj(amdgpu_device_handle dev,
					 uint32_t handle)
{
	if (NULL == dev)
		return -EINVAL;

	return drmSyncobjDestroy(dev->fd, handle);
}

drm_public int amdgpu_cs_syncobj_reset(amdgpu_device_handle dev,
				       const uint32_t *syncobjs,
				       uint32_t syncobj_count)
{
	if (NULL == dev)
		return -EINVAL;

	return drmSyncobjReset(dev->fd, syncobjs, syncobj_count);
}

drm_public int amdgpu_cs_syncobj_signal(amdgpu_device_handle dev,
					const uint32_t *syncobjs,
					uint32_t syncobj_count)
{
	if (NULL == dev)
		return -EINVAL;

	return drmSyncobjSignal(dev->fd, syncobjs, syncobj_count);
}

drm_public int amdgpu_cs_syncobj_timeline_signal(amdgpu_device_handle dev,
						 const uint32_t *syncobjs,
						 uint64_t *points,
						 uint32_t syncobj_count)
{
	if (NULL == dev)
		return -EINVAL;

	return drmSyncobjTimelineSignal(dev->fd, syncobjs,
					points, syncobj_count);
}

drm_public int amdgpu_cs_syncobj_wait(amdgpu_device_handle dev,
				      uint32_t *handles, unsigned num_handles,
				      int64_t timeout_nsec, unsigned flags,
				      uint32_t *first_signaled)
{
	if (NULL == dev)
		return -EINVAL;

	return drmSyncobjWait(dev->fd, handles, num_handles, timeout_nsec,
			      flags, first_signaled);
}

drm_public int amdgpu_cs_syncobj_timeline_wait(amdgpu_device_handle dev,
					       uint32_t *handles, uint64_t *points,
					       unsigned num_handles,
					       int64_t timeout_nsec, unsigned flags,
					       uint32_t *first_signaled)
{
	if (NULL == dev)
		return -EINVAL;

	return drmSyncobjTimelineWait(dev->fd, handles, points, num_handles,
				      timeout_nsec, flags, first_signaled);
}

drm_public int amdgpu_cs_syncobj_query(amdgpu_device_handle dev,
				       uint32_t *handles, uint64_t *points,
				       unsigned num_handles)
{
	if (NULL == dev)
		return -EINVAL;

	return drmSyncobjQuery(dev->fd, handles, points, num_handles);
}

drm_public int amdgpu_cs_syncobj_query2(amdgpu_device_handle dev,
					uint32_t *handles, uint64_t *points,
					unsigned num_handles, uint32_t flags)
{
	if (!dev)
		return -EINVAL;

	return drmSyncobjQuery2(dev->fd, handles, points, num_handles, flags);
}

drm_public int amdgpu_cs_export_syncobj(amdgpu_device_handle dev,
					uint32_t handle,
					int *shared_fd)
{
	if (NULL == dev)
		return -EINVAL;

	return drmSyncobjHandleToFD(dev->fd, handle, shared_fd);
}

drm_public int amdgpu_cs_import_syncobj(amdgpu_device_handle dev,
					int shared_fd,
					uint32_t *handle)
{
	if (NULL == dev)
		return -EINVAL;

	return drmSyncobjFDToHandle(dev->fd, shared_fd, handle);
}

drm_public int amdgpu_cs_syncobj_export_sync_file(amdgpu_device_handle dev,
						  uint32_t syncobj,
						  int *sync_file_fd)
{
	if (NULL == dev)
		return -EINVAL;

	return drmSyncobjExportSyncFile(dev->fd, syncobj, sync_file_fd);
}

drm_public int amdgpu_cs_syncobj_import_sync_file(amdgpu_device_handle dev,
						  uint32_t syncobj,
						  int sync_file_fd)
{
	if (NULL == dev)
		return -EINVAL;

	return drmSyncobjImportSyncFile(dev->fd, syncobj, sync_file_fd);
}

drm_public int amdgpu_cs_syncobj_export_sync_file2(amdgpu_device_handle dev,
						   uint32_t syncobj,
						   uint64_t point,
						   uint32_t flags,
						   int *sync_file_fd)
{
	uint32_t binary_handle;
	int ret;

	if (NULL == dev)
		return -EINVAL;

	if (!point)
		return drmSyncobjExportSyncFile(dev->fd, syncobj, sync_file_fd);

	ret = drmSyncobjCreate(dev->fd, 0, &binary_handle);
	if (ret)
		return ret;

	ret = drmSyncobjTransfer(dev->fd, binary_handle, 0,
				 syncobj, point, flags);
	if (ret)
		goto out;
	ret = drmSyncobjExportSyncFile(dev->fd, binary_handle, sync_file_fd);
out:
	drmSyncobjDestroy(dev->fd, binary_handle);
	return ret;
}

drm_public int amdgpu_cs_syncobj_import_sync_file2(amdgpu_device_handle dev,
						   uint32_t syncobj,
						   uint64_t point,
						   int sync_file_fd)
{
	uint32_t binary_handle;
	int ret;

	if (NULL == dev)
		return -EINVAL;

	if (!point)
		return drmSyncobjImportSyncFile(dev->fd, syncobj, sync_file_fd);

	ret = drmSyncobjCreate(dev->fd, 0, &binary_handle);
	if (ret)
		return ret;
	ret = drmSyncobjImportSyncFile(dev->fd, binary_handle, sync_file_fd);
	if (ret)
		goto out;
	ret = drmSyncobjTransfer(dev->fd, syncobj, point,
				 binary_handle, 0, 0);
out:
	drmSyncobjDestroy(dev->fd, binary_handle);
	return ret;
}

drm_public int amdgpu_cs_syncobj_transfer(amdgpu_device_handle dev,
					  uint32_t dst_handle,
					  uint64_t dst_point,
					  uint32_t src_handle,
					  uint64_t src_point,
					  uint32_t flags)
{
	if (NULL == dev)
		return -EINVAL;

	return drmSyncobjTransfer(dev->fd,
				  dst_handle, dst_point,
				  src_handle, src_point,
				  flags);
}

drm_public int amdgpu_cs_submit_raw(amdgpu_device_handle dev,
				    amdgpu_context_handle context,
				    amdgpu_bo_list_handle bo_list_handle,
				    int num_chunks,
				    struct drm_amdgpu_cs_chunk *chunks,
				    uint64_t *seq_no)
{
	union drm_amdgpu_cs cs;
	uint64_t *chunk_array;
	int i, r;
	if (num_chunks == 0)
		return -EINVAL;

	memset(&cs, 0, sizeof(cs));
	chunk_array = alloca(sizeof(uint64_t) * num_chunks);
	for (i = 0; i < num_chunks; i++)
		chunk_array[i] = (uint64_t)(uintptr_t)&chunks[i];
	cs.in.chunks = (uint64_t)(uintptr_t)chunk_array;
	cs.in.ctx_id = context->id;
	cs.in.bo_list_handle = bo_list_handle ? bo_list_handle->handle : 0;
	cs.in.num_chunks = num_chunks;
	r = drmCommandWriteRead(dev->fd, DRM_AMDGPU_CS,
				&cs, sizeof(cs));
	if (r)
		return r;

	if (seq_no)
		*seq_no = cs.out.handle;
	return 0;
}

drm_public int amdgpu_cs_submit_raw2(amdgpu_device_handle dev,
				     amdgpu_context_handle context,
				     uint32_t bo_list_handle,
				     int num_chunks,
				     struct drm_amdgpu_cs_chunk *chunks,
				     uint64_t *seq_no)
{
	union drm_amdgpu_cs cs;
	uint64_t *chunk_array;
	int i, r;

	memset(&cs, 0, sizeof(cs));
	chunk_array = alloca(sizeof(uint64_t) * num_chunks);
	for (i = 0; i < num_chunks; i++)
		chunk_array[i] = (uint64_t)(uintptr_t)&chunks[i];
	cs.in.chunks = (uint64_t)(uintptr_t)chunk_array;
	cs.in.ctx_id = context->id;
	cs.in.bo_list_handle = bo_list_handle;
	cs.in.num_chunks = num_chunks;
	r = drmCommandWriteRead(dev->fd, DRM_AMDGPU_CS,
				&cs, sizeof(cs));
	if (!r && seq_no)
		*seq_no = cs.out.handle;
	return r;
}

drm_public void amdgpu_cs_chunk_fence_info_to_data(struct amdgpu_cs_fence_info *fence_info,
					struct drm_amdgpu_cs_chunk_data *data)
{
	data->fence_data.handle = fence_info->handle->handle;
	data->fence_data.offset = fence_info->offset * sizeof(uint64_t);
}

drm_public void amdgpu_cs_chunk_fence_to_dep(struct amdgpu_cs_fence *fence,
					struct drm_amdgpu_cs_chunk_dep *dep)
{
	dep->ip_type = fence->ip_type;
	dep->ip_instance = fence->ip_instance;
	dep->ring = fence->ring;
	dep->ctx_id = fence->context->id;
	dep->handle = fence->fence;
}

drm_public int amdgpu_cs_fence_to_handle(amdgpu_device_handle dev,
					 struct amdgpu_cs_fence *fence,
					 uint32_t what,
					 uint32_t *out_handle)
{
	union drm_amdgpu_fence_to_handle fth;
	int r;

	memset(&fth, 0, sizeof(fth));
	fth.in.fence.ctx_id = fence->context->id;
	fth.in.fence.ip_type = fence->ip_type;
	fth.in.fence.ip_instance = fence->ip_instance;
	fth.in.fence.ring = fence->ring;
	fth.in.fence.seq_no = fence->fence;
	fth.in.what = what;

	r = drmCommandWriteRead(dev->fd, DRM_AMDGPU_FENCE_TO_HANDLE,
				&fth, sizeof(fth));
	if (r == 0)
		*out_handle = fth.out.handle;
	return r;
}
