#ifndef _MSM_KGSL_H
#define _MSM_KGSL_H

#define KGSL_VERSION_MAJOR        3
#define KGSL_VERSION_MINOR        11

/*context flags */
#define KGSL_CONTEXT_SAVE_GMEM		0x00000001
#define KGSL_CONTEXT_NO_GMEM_ALLOC	0x00000002
#define KGSL_CONTEXT_SUBMIT_IB_LIST	0x00000004
#define KGSL_CONTEXT_CTX_SWITCH		0x00000008
#define KGSL_CONTEXT_PREAMBLE		0x00000010
#define KGSL_CONTEXT_TRASH_STATE	0x00000020
#define KGSL_CONTEXT_PER_CONTEXT_TS	0x00000040

#define KGSL_CONTEXT_INVALID 0xffffffff

/* Memory allocayion flags */
#define KGSL_MEMFLAGS_GPUREADONLY	0x01000000

/* generic flag values */
#define KGSL_FLAGS_NORMALMODE  0x00000000
#define KGSL_FLAGS_SAFEMODE    0x00000001
#define KGSL_FLAGS_INITIALIZED0 0x00000002
#define KGSL_FLAGS_INITIALIZED 0x00000004
#define KGSL_FLAGS_STARTED     0x00000008
#define KGSL_FLAGS_ACTIVE      0x00000010
#define KGSL_FLAGS_RESERVED0   0x00000020
#define KGSL_FLAGS_RESERVED1   0x00000040
#define KGSL_FLAGS_RESERVED2   0x00000080
#define KGSL_FLAGS_SOFT_RESET  0x00000100
#define KGSL_FLAGS_PER_CONTEXT_TIMESTAMPS 0x00000200

/* Clock flags to show which clocks should be controlled by a given platform */
#define KGSL_CLK_SRC	0x00000001
#define KGSL_CLK_CORE	0x00000002
#define KGSL_CLK_IFACE	0x00000004
#define KGSL_CLK_MEM	0x00000008
#define KGSL_CLK_MEM_IFACE 0x00000010
#define KGSL_CLK_AXI	0x00000020

/*
 * Reset status values for context
 */
enum kgsl_ctx_reset_stat {
	KGSL_CTX_STAT_NO_ERROR				= 0x00000000,
	KGSL_CTX_STAT_GUILTY_CONTEXT_RESET_EXT		= 0x00000001,
	KGSL_CTX_STAT_INNOCENT_CONTEXT_RESET_EXT	= 0x00000002,
	KGSL_CTX_STAT_UNKNOWN_CONTEXT_RESET_EXT		= 0x00000003
};

#define KGSL_MAX_PWRLEVELS 5

#define KGSL_CONVERT_TO_MBPS(val) \
	(val*1000*1000U)

/* device id */
enum kgsl_deviceid {
	KGSL_DEVICE_3D0		= 0x00000000,
	KGSL_DEVICE_2D0		= 0x00000001,
	KGSL_DEVICE_2D1		= 0x00000002,
	KGSL_DEVICE_MAX		= 0x00000003
};

enum kgsl_user_mem_type {
	KGSL_USER_MEM_TYPE_PMEM		= 0x00000000,
	KGSL_USER_MEM_TYPE_ASHMEM	= 0x00000001,
	KGSL_USER_MEM_TYPE_ADDR		= 0x00000002,
	KGSL_USER_MEM_TYPE_ION		= 0x00000003,
	KGSL_USER_MEM_TYPE_MAX		= 0x00000004,
};

struct kgsl_devinfo {

	unsigned int device_id;
	/* chip revision id
	* coreid:8 majorrev:8 minorrev:8 patch:8
	*/
	unsigned int chip_id;
	unsigned int mmu_enabled;
	unsigned int gmem_gpubaseaddr;
	/*
	* This field contains the adreno revision
	* number 200, 205, 220, etc...
	*/
	unsigned int gpu_id;
	unsigned int gmem_sizebytes;
};

/* this structure defines the region of memory that can be mmap()ed from this
   driver. The timestamp fields are volatile because they are written by the
   GPU
*/
struct kgsl_devmemstore {
	volatile unsigned int soptimestamp;
	unsigned int sbz;
	volatile unsigned int eoptimestamp;
	unsigned int sbz2;
	volatile unsigned int ts_cmp_enable;
	unsigned int sbz3;
	volatile unsigned int ref_wait_ts;
	unsigned int sbz4;
	unsigned int current_context;
	unsigned int sbz5;
};

#define KGSL_MEMSTORE_OFFSET(ctxt_id, field) \
	((ctxt_id)*sizeof(struct kgsl_devmemstore) + \
	 offsetof(struct kgsl_devmemstore, field))

/* timestamp id*/
enum kgsl_timestamp_type {
	KGSL_TIMESTAMP_CONSUMED = 0x00000001, /* start-of-pipeline timestamp */
	KGSL_TIMESTAMP_RETIRED  = 0x00000002, /* end-of-pipeline timestamp*/
	KGSL_TIMESTAMP_QUEUED   = 0x00000003,
};

/* property types - used with kgsl_device_getproperty */
enum kgsl_property_type {
	KGSL_PROP_DEVICE_INFO     = 0x00000001,
	KGSL_PROP_DEVICE_SHADOW   = 0x00000002,
	KGSL_PROP_DEVICE_POWER    = 0x00000003,
	KGSL_PROP_SHMEM           = 0x00000004,
	KGSL_PROP_SHMEM_APERTURES = 0x00000005,
	KGSL_PROP_MMU_ENABLE 	  = 0x00000006,
	KGSL_PROP_INTERRUPT_WAITS = 0x00000007,
	KGSL_PROP_VERSION         = 0x00000008,
	KGSL_PROP_GPU_RESET_STAT  = 0x00000009,
	KGSL_PROP_PWRCTRL         = 0x0000000E,
};

struct kgsl_shadowprop {
	unsigned int gpuaddr;
	unsigned int size;
	unsigned int flags; /* contains KGSL_FLAGS_ values */
};

struct kgsl_pwrlevel {
	unsigned int gpu_freq;
	unsigned int bus_freq;
	unsigned int io_fraction;
};

struct kgsl_version {
	unsigned int drv_major;
	unsigned int drv_minor;
	unsigned int dev_major;
	unsigned int dev_minor;
};

#ifdef __KERNEL__

#define KGSL_3D0_REG_MEMORY	"kgsl_3d0_reg_memory"
#define KGSL_3D0_IRQ		"kgsl_3d0_irq"
#define KGSL_2D0_REG_MEMORY	"kgsl_2d0_reg_memory"
#define KGSL_2D0_IRQ		"kgsl_2d0_irq"
#define KGSL_2D1_REG_MEMORY	"kgsl_2d1_reg_memory"
#define KGSL_2D1_IRQ		"kgsl_2d1_irq"

enum kgsl_iommu_context_id {
	KGSL_IOMMU_CONTEXT_USER = 0,
	KGSL_IOMMU_CONTEXT_PRIV = 1,
};

struct kgsl_iommu_ctx {
	const char *iommu_ctx_name;
	enum kgsl_iommu_context_id ctx_id;
};

struct kgsl_device_iommu_data {
	const struct kgsl_iommu_ctx *iommu_ctxs;
	int iommu_ctx_count;
	unsigned int physstart;
	unsigned int physend;
};

struct kgsl_device_platform_data {
	struct kgsl_pwrlevel pwrlevel[KGSL_MAX_PWRLEVELS];
	int init_level;
	int num_levels;
	int (*set_grp_async)(void);
	unsigned int idle_timeout;
	bool strtstp_sleepwake;
	unsigned int nap_allowed;
	unsigned int clk_map;
	unsigned int idle_needed;
	struct msm_bus_scale_pdata *bus_scale_table;
	struct kgsl_device_iommu_data *iommu_data;
	int iommu_count;
	struct msm_dcvs_core_info *core_info;
};

#endif

/* structure holds list of ibs */
struct kgsl_ibdesc {
	unsigned int gpuaddr;
	void *hostptr;
	unsigned int sizedwords;
	unsigned int ctrl;
};

/* ioctls */
#define KGSL_IOC_TYPE 0x09

/* get misc info about the GPU
   type should be a value from enum kgsl_property_type
   value points to a structure that varies based on type
   sizebytes is sizeof() that structure
   for KGSL_PROP_DEVICE_INFO, use struct kgsl_devinfo
   this structure contaings hardware versioning info.
   for KGSL_PROP_DEVICE_SHADOW, use struct kgsl_shadowprop
   this is used to find mmap() offset and sizes for mapping
   struct kgsl_memstore into userspace.
*/
struct kgsl_device_getproperty {
	unsigned int type;
	void  *value;
	unsigned int sizebytes;
};

#define IOCTL_KGSL_DEVICE_GETPROPERTY \
	_IOWR(KGSL_IOC_TYPE, 0x2, struct kgsl_device_getproperty)

/* IOCTL_KGSL_DEVICE_READ (0x3) - removed 03/2012
 */

/* block until the GPU has executed past a given timestamp
 * timeout is in milliseconds.
 */
struct kgsl_device_waittimestamp {
	unsigned int timestamp;
	unsigned int timeout;
};

#define IOCTL_KGSL_DEVICE_WAITTIMESTAMP \
	_IOW(KGSL_IOC_TYPE, 0x6, struct kgsl_device_waittimestamp)

struct kgsl_device_waittimestamp_ctxtid {
	unsigned int context_id;
	unsigned int timestamp;
	unsigned int timeout;
};

#define IOCTL_KGSL_DEVICE_WAITTIMESTAMP_CTXTID \
	_IOW(KGSL_IOC_TYPE, 0x7, struct kgsl_device_waittimestamp_ctxtid)

/* issue indirect commands to the GPU.
 * drawctxt_id must have been created with IOCTL_KGSL_DRAWCTXT_CREATE
 * ibaddr and sizedwords must specify a subset of a buffer created
 * with IOCTL_KGSL_SHAREDMEM_FROM_PMEM
 * flags may be a mask of KGSL_CONTEXT_ values
 * timestamp is a returned counter value which can be passed to
 * other ioctls to determine when the commands have been executed by
 * the GPU.
 */
struct kgsl_ringbuffer_issueibcmds {
	unsigned int drawctxt_id;
	unsigned int ibdesc_addr;
	unsigned int numibs;
	unsigned int timestamp; /*output param */
	unsigned int flags;
};

#define IOCTL_KGSL_RINGBUFFER_ISSUEIBCMDS \
	_IOWR(KGSL_IOC_TYPE, 0x10, struct kgsl_ringbuffer_issueibcmds)

/* read the most recently executed timestamp value
 * type should be a value from enum kgsl_timestamp_type
 */
struct kgsl_cmdstream_readtimestamp {
	unsigned int type;
	unsigned int timestamp; /*output param */
};

#define IOCTL_KGSL_CMDSTREAM_READTIMESTAMP_OLD \
	_IOR(KGSL_IOC_TYPE, 0x11, struct kgsl_cmdstream_readtimestamp)

#define IOCTL_KGSL_CMDSTREAM_READTIMESTAMP \
	_IOWR(KGSL_IOC_TYPE, 0x11, struct kgsl_cmdstream_readtimestamp)

/* free memory when the GPU reaches a given timestamp.
 * gpuaddr specify a memory region created by a
 * IOCTL_KGSL_SHAREDMEM_FROM_PMEM call
 * type should be a value from enum kgsl_timestamp_type
 */
struct kgsl_cmdstream_freememontimestamp {
	unsigned int gpuaddr;
	unsigned int type;
	unsigned int timestamp;
};

#define IOCTL_KGSL_CMDSTREAM_FREEMEMONTIMESTAMP \
	_IOW(KGSL_IOC_TYPE, 0x12, struct kgsl_cmdstream_freememontimestamp)

/* Previous versions of this header had incorrectly defined
   IOCTL_KGSL_CMDSTREAM_FREEMEMONTIMESTAMP as a read-only ioctl instead
   of a write only ioctl.  To ensure binary compatibility, the following
   #define will be used to intercept the incorrect ioctl
*/

#define IOCTL_KGSL_CMDSTREAM_FREEMEMONTIMESTAMP_OLD \
	_IOR(KGSL_IOC_TYPE, 0x12, struct kgsl_cmdstream_freememontimestamp)

/* create a draw context, which is used to preserve GPU state.
 * The flags field may contain a mask KGSL_CONTEXT_*  values
 */
struct kgsl_drawctxt_create {
	unsigned int flags;
	unsigned int drawctxt_id; /*output param */
};

#define IOCTL_KGSL_DRAWCTXT_CREATE \
	_IOWR(KGSL_IOC_TYPE, 0x13, struct kgsl_drawctxt_create)

/* destroy a draw context */
struct kgsl_drawctxt_destroy {
	unsigned int drawctxt_id;
};

#define IOCTL_KGSL_DRAWCTXT_DESTROY \
	_IOW(KGSL_IOC_TYPE, 0x14, struct kgsl_drawctxt_destroy)

/* add a block of pmem, fb, ashmem or user allocated address
 * into the GPU address space */
struct kgsl_map_user_mem {
	int fd;
	unsigned int gpuaddr;   /*output param */
	unsigned int len;
	unsigned int offset;
	unsigned int hostptr;   /*input param */
	enum kgsl_user_mem_type memtype;
	unsigned int reserved;	/* May be required to add
				params for another mem type */
};

#define IOCTL_KGSL_MAP_USER_MEM \
	_IOWR(KGSL_IOC_TYPE, 0x15, struct kgsl_map_user_mem)

struct kgsl_cmdstream_readtimestamp_ctxtid {
	unsigned int context_id;
	unsigned int type;
	unsigned int timestamp; /*output param */
};

#define IOCTL_KGSL_CMDSTREAM_READTIMESTAMP_CTXTID \
	_IOWR(KGSL_IOC_TYPE, 0x16, struct kgsl_cmdstream_readtimestamp_ctxtid)

struct kgsl_cmdstream_freememontimestamp_ctxtid {
	unsigned int context_id;
	unsigned int gpuaddr;
	unsigned int type;
	unsigned int timestamp;
};

#define IOCTL_KGSL_CMDSTREAM_FREEMEMONTIMESTAMP_CTXTID \
	_IOW(KGSL_IOC_TYPE, 0x17, \
	struct kgsl_cmdstream_freememontimestamp_ctxtid)

/* add a block of pmem or fb into the GPU address space */
struct kgsl_sharedmem_from_pmem {
	int pmem_fd;
	unsigned int gpuaddr;	/*output param */
	unsigned int len;
	unsigned int offset;
};

#define IOCTL_KGSL_SHAREDMEM_FROM_PMEM \
	_IOWR(KGSL_IOC_TYPE, 0x20, struct kgsl_sharedmem_from_pmem)

/* remove memory from the GPU's address space */
struct kgsl_sharedmem_free {
	unsigned int gpuaddr;
};

#define IOCTL_KGSL_SHAREDMEM_FREE \
	_IOW(KGSL_IOC_TYPE, 0x21, struct kgsl_sharedmem_free)

struct kgsl_cff_user_event {
	unsigned char cff_opcode;
	unsigned int op1;
	unsigned int op2;
	unsigned int op3;
	unsigned int op4;
	unsigned int op5;
	unsigned int __pad[2];
};

#define IOCTL_KGSL_CFF_USER_EVENT \
	_IOW(KGSL_IOC_TYPE, 0x31, struct kgsl_cff_user_event)

struct kgsl_gmem_desc {
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
	unsigned int pitch;
};

struct kgsl_buffer_desc {
	void 			*hostptr;
	unsigned int	gpuaddr;
	int				size;
	unsigned int	format;
	unsigned int  	pitch;
	unsigned int  	enabled;
};

struct kgsl_bind_gmem_shadow {
	unsigned int drawctxt_id;
	struct kgsl_gmem_desc gmem_desc;
	unsigned int shadow_x;
	unsigned int shadow_y;
	struct kgsl_buffer_desc shadow_buffer;
	unsigned int buffer_id;
};

#define IOCTL_KGSL_DRAWCTXT_BIND_GMEM_SHADOW \
    _IOW(KGSL_IOC_TYPE, 0x22, struct kgsl_bind_gmem_shadow)

/* add a block of memory into the GPU address space */
struct kgsl_sharedmem_from_vmalloc {
	unsigned int gpuaddr;	/*output param */
	unsigned int hostptr;
	unsigned int flags;
};

#define IOCTL_KGSL_SHAREDMEM_FROM_VMALLOC \
	_IOWR(KGSL_IOC_TYPE, 0x23, struct kgsl_sharedmem_from_vmalloc)

#define IOCTL_KGSL_SHAREDMEM_FLUSH_CACHE \
	_IOW(KGSL_IOC_TYPE, 0x24, struct kgsl_sharedmem_free)

struct kgsl_drawctxt_set_bin_base_offset {
	unsigned int drawctxt_id;
	unsigned int offset;
};

#define IOCTL_KGSL_DRAWCTXT_SET_BIN_BASE_OFFSET \
	_IOW(KGSL_IOC_TYPE, 0x25, struct kgsl_drawctxt_set_bin_base_offset)

enum kgsl_cmdwindow_type {
	KGSL_CMDWINDOW_MIN     = 0x00000000,
	KGSL_CMDWINDOW_2D      = 0x00000000,
	KGSL_CMDWINDOW_3D      = 0x00000001, /* legacy */
	KGSL_CMDWINDOW_MMU     = 0x00000002,
	KGSL_CMDWINDOW_ARBITER = 0x000000FF,
	KGSL_CMDWINDOW_MAX     = 0x000000FF,
};

/* write to the command window */
struct kgsl_cmdwindow_write {
	enum kgsl_cmdwindow_type target;
	unsigned int addr;
	unsigned int data;
};

#define IOCTL_KGSL_CMDWINDOW_WRITE \
	_IOW(KGSL_IOC_TYPE, 0x2e, struct kgsl_cmdwindow_write)

struct kgsl_gpumem_alloc {
	unsigned long gpuaddr;
	size_t size;
	unsigned int flags;
};

#define IOCTL_KGSL_GPUMEM_ALLOC \
	_IOWR(KGSL_IOC_TYPE, 0x2f, struct kgsl_gpumem_alloc)

struct kgsl_cff_syncmem {
	unsigned int gpuaddr;
	unsigned int len;
	unsigned int __pad[2]; /* For future binary compatibility */
};

#define IOCTL_KGSL_CFF_SYNCMEM \
	_IOW(KGSL_IOC_TYPE, 0x30, struct kgsl_cff_syncmem)

/*
 * A timestamp event allows the user space to register an action following an
 * expired timestamp.
 */

struct kgsl_timestamp_event {
	int type;                /* Type of event (see list below) */
	unsigned int timestamp;  /* Timestamp to trigger event on */
	unsigned int context_id; /* Context for the timestamp */
	void *priv;              /* Pointer to the event specific blob */
	size_t len;              /* Size of the event specific blob */
};

#define IOCTL_KGSL_TIMESTAMP_EVENT \
	_IOW(KGSL_IOC_TYPE, 0x31, struct kgsl_timestamp_event)

/* A genlock timestamp event releases an existing lock on timestamp expire */

#define KGSL_TIMESTAMP_EVENT_GENLOCK 1

struct kgsl_timestamp_event_genlock {
	int handle; /* Handle of the genlock lock to release */
};

/*
 * Set a property within the kernel.  Uses the same structure as
 * IOCTL_KGSL_GETPROPERTY
 */

#define IOCTL_KGSL_SETPROPERTY \
	_IOW(KGSL_IOC_TYPE, 0x32, struct kgsl_device_getproperty)

#ifdef __KERNEL__
#ifdef CONFIG_MSM_KGSL_DRM
int kgsl_gem_obj_addr(int drm_fd, int handle, unsigned long *start,
			unsigned long *len);
#else
#define kgsl_gem_obj_addr(...) 0
#endif
#endif
#endif /* _MSM_KGSL_H */
