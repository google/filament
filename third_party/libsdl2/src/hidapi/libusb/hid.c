/*******************************************************
 HIDAPI - Multi-Platform library for
 communication with HID devices.

 Alan Ott
 Signal 11 Software

 8/22/2009
 Linux Version - 6/2/2010
 Libusb Version - 8/13/2010
 FreeBSD Version - 11/1/2011

 Copyright 2009, All Rights Reserved.

 At the discretion of the user of this library,
 this software may be licensed under the terms of the
 GNU General Public License v3, a BSD-Style license, or the
 original HIDAPI license as outlined in the LICENSE.txt,
 LICENSE-gpl3.txt, LICENSE-bsd.txt, and LICENSE-orig.txt
 files located at the root of the source distribution.
 These files may also be found in the public source
 code repository located at:
        https://github.com/libusb/hidapi .
********************************************************/

/* This file is heavily modified from the original libusb.c, for portability.
 * Last upstream update was from July 25, 2019, Git commit 93dca807.
 */

#include "../../SDL_internal.h"
#include "SDL_thread.h"
#include "SDL_mutex.h"

#ifdef SDL_JOYSTICK_HIDAPI

#if defined(HAVE__WCSDUP) && !defined(HAVE_WCSDUP)
#define wcsdup _wcsdup
#endif

#include <libusb.h>
#include <locale.h> /* setlocale */

#include "hidapi.h"

#ifdef NAMESPACE
namespace NAMESPACE
{
#endif

/* Barrier implementation because Android/Bionic don't have pthread_barrier.
   This implementation came from Brent Priddy and was posted on
   StackOverflow. It is used with his permission. */

typedef struct _SDL_ThreadBarrier
{
	SDL_mutex *mutex;
	SDL_cond *cond;
	Uint32 count;
	Uint32 trip_count;
} SDL_ThreadBarrier;

static int SDL_CreateThreadBarrier(SDL_ThreadBarrier *barrier, Uint32 count)
{
	if (barrier == NULL) {
		return SDL_SetError("barrier must be non-NULL");
	}
	if (count == 0) {
		return SDL_SetError("count must be > 0");
	}

	barrier->mutex = SDL_CreateMutex();
	if (barrier->mutex == NULL) {
		return -1; /* Error set by CreateMutex */
	}
	barrier->cond = SDL_CreateCond();
	if (barrier->cond == NULL) {
		return -1; /* Error set by CreateCond */
	}

	barrier->trip_count = count;
	barrier->count = 0;

	return 0;
}

static void SDL_DestroyThreadBarrier(SDL_ThreadBarrier *barrier)
{
	SDL_DestroyCond(barrier->cond);
	SDL_DestroyMutex(barrier->mutex);
}

static int SDL_WaitThreadBarrier(SDL_ThreadBarrier *barrier)
{
	SDL_LockMutex(barrier->mutex);
	barrier->count += 1;
	if (barrier->count >= barrier->trip_count) {
		barrier->count = 0;
		SDL_CondBroadcast(barrier->cond);
		SDL_UnlockMutex(barrier->mutex);
		return 1;
	}
	SDL_CondWait(barrier->cond, barrier->mutex);
	SDL_UnlockMutex(barrier->mutex);
	return 0;
}

#if defined(__cplusplus) && !defined(NAMESPACE)
extern "C" {
#endif

#ifdef DEBUG_PRINTF
#define LOG(...) fprintf(stderr, __VA_ARGS__)
#else
#define LOG(...) do {} while (0)
#endif

#ifndef __FreeBSD__
#define DETACH_KERNEL_DRIVER
#endif

/* Uncomment to enable the retrieval of Usage and Usage Page in
hid_enumerate(). Warning, on platforms different from FreeBSD
this is very invasive as it requires the detach
and re-attach of the kernel driver. See comments inside hid_enumerate().
libusb HIDAPI programs are encouraged to use the interface number
instead to differentiate between interfaces on a composite HID device. */
/*#define INVASIVE_GET_USAGE*/

/* Linked List of input reports received from the device. */
struct input_report {
	uint8_t *data;
	size_t len;
	struct input_report *next;
};


struct hid_device_ {
	/* Handle to the actual device. */
	libusb_device_handle *device_handle;

	/* Endpoint information */
	int input_endpoint;
	int output_endpoint;
	int input_ep_max_packet_size;

	/* The interface number of the HID */
	int interface;
	int detached_driver;

	/* Indexes of Strings */
	int manufacturer_index;
	int product_index;
	int serial_index;

	/* Whether blocking reads are used */
	int blocking; /* boolean */

	/* Read thread objects */
	SDL_Thread *thread;
	SDL_mutex *mutex; /* Protects input_reports */
	SDL_cond *condition;
	SDL_ThreadBarrier barrier; /* Ensures correct startup sequence */
	int shutdown_thread;
	int transfer_loop_finished;
	struct libusb_transfer *transfer;

	/* List of received input reports. */
	struct input_report *input_reports;
};

static libusb_context *usb_context = NULL;

uint16_t get_usb_code_for_current_locale(void);
static int return_data(hid_device *dev, unsigned char *data, size_t length);

static hid_device *new_hid_device(void)
{
	hid_device *dev = (hid_device*) calloc(1, sizeof(hid_device));
	dev->blocking = 1;

	dev->mutex = SDL_CreateMutex();
	dev->condition = SDL_CreateCond();
	SDL_CreateThreadBarrier(&dev->barrier, 2);

	return dev;
}

static void free_hid_device(hid_device *dev)
{
	/* Clean up the thread objects */
	SDL_DestroyThreadBarrier(&dev->barrier);
	SDL_DestroyCond(dev->condition);
	SDL_DestroyMutex(dev->mutex);

	/* Free the device itself */
	free(dev);
}

#if 0
/*TODO: Implement this function on hidapi/libusb.. */
static void register_error(hid_device *dev, const char *op)
{
}
#endif

#ifdef INVASIVE_GET_USAGE
/* Get bytes from a HID Report Descriptor.
   Only call with a num_bytes of 0, 1, 2, or 4. */
static uint32_t get_bytes(uint8_t *rpt, size_t len, size_t num_bytes, size_t cur)
{
	/* Return if there aren't enough bytes. */
	if (cur + num_bytes >= len)
		return 0;

	if (num_bytes == 0)
		return 0;
	else if (num_bytes == 1) {
		return rpt[cur+1];
	}
	else if (num_bytes == 2) {
		return (rpt[cur+2] * 256 + rpt[cur+1]);
	}
	else if (num_bytes == 4) {
		return (rpt[cur+4] * 0x01000000 +
		        rpt[cur+3] * 0x00010000 +
		        rpt[cur+2] * 0x00000100 +
		        rpt[cur+1] * 0x00000001);
	}
	else
		return 0;
}

/* Retrieves the device's Usage Page and Usage from the report
   descriptor. The algorithm is simple, as it just returns the first
   Usage and Usage Page that it finds in the descriptor.
   The return value is 0 on success and -1 on failure. */
static int get_usage(uint8_t *report_descriptor, size_t size,
                     unsigned short *usage_page, unsigned short *usage)
{
	unsigned int i = 0;
	int size_code;
	int data_len, key_size;
	int usage_found = 0, usage_page_found = 0;

	while (i < size) {
		int key = report_descriptor[i];
		int key_cmd = key & 0xfc;

		//printf("key: %02hhx\n", key);

		if ((key & 0xf0) == 0xf0) {
			/* This is a Long Item. The next byte contains the
			   length of the data section (value) for this key.
			   See the HID specification, version 1.11, section
			   6.2.2.3, titled "Long Items." */
			if (i+1 < size)
				data_len = report_descriptor[i+1];
			else
				data_len = 0; /* malformed report */
			key_size = 3;
		}
		else {
			/* This is a Short Item. The bottom two bits of the
			   key contain the size code for the data section
			   (value) for this key.  Refer to the HID
			   specification, version 1.11, section 6.2.2.2,
			   titled "Short Items." */
			size_code = key & 0x3;
			switch (size_code) {
			case 0:
			case 1:
			case 2:
				data_len = size_code;
				break;
			case 3:
				data_len = 4;
				break;
			default:
				/* Can't ever happen since size_code is & 0x3 */
				data_len = 0;
				break;
			};
			key_size = 1;
		}

		if (key_cmd == 0x4) {
			*usage_page  = get_bytes(report_descriptor, size, data_len, i);
			usage_page_found = 1;
			//printf("Usage Page: %x\n", (uint32_t)*usage_page);
		}
		if (key_cmd == 0x8) {
			*usage = get_bytes(report_descriptor, size, data_len, i);
			usage_found = 1;
			//printf("Usage: %x\n", (uint32_t)*usage);
		}

		if (usage_page_found && usage_found)
			return 0; /* success */

		/* Skip over this key and it's associated data */
		i += data_len + key_size;
	}

	return -1; /* failure */
}
#endif /* INVASIVE_GET_USAGE */

#if defined(__FreeBSD__) && __FreeBSD__ < 10
/* The libusb version included in FreeBSD < 10 doesn't have this function. In
   mainline libusb, it's inlined in libusb.h. This function will bear a striking
   resemblance to that one, because there's about one way to code it.

   Note that the data parameter is Unicode in UTF-16LE encoding.
   Return value is the number of bytes in data, or LIBUSB_ERROR_*.
 */
static inline int libusb_get_string_descriptor(libusb_device_handle *dev,
	uint8_t descriptor_index, uint16_t lang_id,
	unsigned char *data, int length)
{
	return libusb_control_transfer(dev,
		LIBUSB_ENDPOINT_IN | 0x0, /* Endpoint 0 IN */
		LIBUSB_REQUEST_GET_DESCRIPTOR,
		(LIBUSB_DT_STRING << 8) | descriptor_index,
		lang_id, data, (uint16_t) length, 1000);
}
#endif


/* Get the first language the device says it reports. This comes from
   USB string #0. */
static uint16_t get_first_language(libusb_device_handle *dev)
{
	uint16_t buf[32];
	int len;

	/* Get the string from libusb. */
	len = libusb_get_string_descriptor(dev,
			0x0, /* String ID */
			0x0, /* Language */
			(unsigned char*)buf,
			sizeof(buf));
	if (len < 4)
		return 0x0;

	return buf[1]; /* First two bytes are len and descriptor type. */
}

static int is_language_supported(libusb_device_handle *dev, uint16_t lang)
{
	uint16_t buf[32];
	int len;
	int i;

	/* Get the string from libusb. */
	len = libusb_get_string_descriptor(dev,
			0x0, /* String ID */
			0x0, /* Language */
			(unsigned char*)buf,
			sizeof(buf));
	if (len < 4)
		return 0x0;


	len /= 2; /* language IDs are two-bytes each. */
	/* Start at index 1 because there are two bytes of protocol data. */
	for (i = 1; i < len; i++) {
		if (buf[i] == lang)
			return 1;
	}

	return 0;
}


/* This function returns a newly allocated wide string containing the USB
   device string numbered by the index. The returned string must be freed
   by using free(). */
static wchar_t *get_usb_string(libusb_device_handle *dev, uint8_t idx)
{
	char buf[512];
	int len;
	wchar_t *str = NULL;

	wchar_t wbuf[256];
	SDL_iconv_t ic;
	size_t inbytes;
	size_t outbytes;
	size_t res;
	const char *inptr;
	char *outptr;

	/* Determine which language to use. */
	uint16_t lang;
	lang = get_usb_code_for_current_locale();
	if (!is_language_supported(dev, lang))
		lang = get_first_language(dev);

	/* Get the string from libusb. */
	len = libusb_get_string_descriptor(dev,
			idx,
			lang,
			(unsigned char*)buf,
			sizeof(buf));
	if (len < 0)
		return NULL;

	/* buf does not need to be explicitly NULL-terminated because
	   it is only passed into iconv() which does not need it. */

	/* Initialize iconv. */
	ic = SDL_iconv_open("WCHAR_T", "UTF-16LE");
	if (ic == (SDL_iconv_t)-1) {
		LOG("SDL_iconv_open() failed\n");
		return NULL;
	}

	/* Convert to native wchar_t (UTF-32 on glibc/BSD systems).
	   Skip the first character (2-bytes). */
	inptr = buf+2;
	inbytes = len-2;
	outptr = (char*) wbuf;
	outbytes = sizeof(wbuf);
	res = SDL_iconv(ic, &inptr, &inbytes, &outptr, &outbytes);
	if (res == (size_t)-1) {
		LOG("SDL_iconv() failed\n");
		goto err;
	}

	/* Write the terminating NULL. */
	wbuf[sizeof(wbuf)/sizeof(wbuf[0])-1] = 0x00000000;
	if (outbytes >= sizeof(wbuf[0]))
		*((wchar_t*)outptr) = 0x00000000;

	/* Allocate and copy the string. */
	str = wcsdup(wbuf);

err:
	SDL_iconv_close(ic);

	return str;
}

struct usb_string_cache_entry {
	uint16_t vid;
	uint16_t pid;
	wchar_t *vendor;
	wchar_t *product;
};

static struct usb_string_cache_entry *usb_string_cache = NULL;
static size_t usb_string_cache_size = 0;
static size_t usb_string_cache_insert_pos = 0;

static int usb_string_cache_grow()
{
	struct usb_string_cache_entry *new_cache;
	size_t allocSize;
	size_t new_cache_size;

	new_cache_size = usb_string_cache_size + 8;
	allocSize = sizeof(struct usb_string_cache_entry) * new_cache_size;
	new_cache = (struct usb_string_cache_entry *)realloc(usb_string_cache, allocSize);
	if (!new_cache)
		return -1;

	usb_string_cache = new_cache;
	usb_string_cache_size = new_cache_size;

	return 0;
}

static void usb_string_cache_destroy()
{
	size_t i;
	for (i = 0; i < usb_string_cache_insert_pos; i++) {
		free(usb_string_cache[i].vendor);
		free(usb_string_cache[i].product);
	}
	free(usb_string_cache);

	usb_string_cache = NULL;
	usb_string_cache_size = 0;
	usb_string_cache_insert_pos = 0;
}

static struct usb_string_cache_entry *usb_string_cache_insert()
{
	struct usb_string_cache_entry *new_entry = NULL;
	if (usb_string_cache_insert_pos >= usb_string_cache_size) {
		if (usb_string_cache_grow() < 0)
			return NULL;
	}
	new_entry = &usb_string_cache[usb_string_cache_insert_pos];
	usb_string_cache_insert_pos++;
	return new_entry;
}

static const struct usb_string_cache_entry *usb_string_cache_find(struct libusb_device_descriptor *desc, struct libusb_device_handle *handle)
{
	struct usb_string_cache_entry *entry = NULL;
	size_t i;

	/* Search for existing string cache entry */
	for (i = 0; i < usb_string_cache_insert_pos; i++) {
		entry = &usb_string_cache[i];
		if (entry->vid != desc->idVendor)
			continue;
		if (entry->pid != desc->idProduct)
			continue;
		return entry;
	}

	/* Not found, create one. */
	entry = usb_string_cache_insert();
	if (!entry)
		return NULL;

	entry->vid = desc->idVendor;
	entry->pid = desc->idProduct;
	if (desc->iManufacturer > 0)
		entry->vendor = get_usb_string(handle, desc->iManufacturer);
	else
		entry->vendor = NULL;
	if (desc->iProduct > 0)
		entry->product = get_usb_string(handle, desc->iProduct);
	else
		entry->product = NULL;

	return entry;
}

static char *make_path(libusb_device *dev, int interface_number)
{
	char str[64];
	snprintf(str, sizeof(str), "%04x:%04x:%02x",
		libusb_get_bus_number(dev),
		libusb_get_device_address(dev),
		interface_number);
	str[sizeof(str)-1] = '\0';

	return strdup(str);
}


int HID_API_EXPORT hid_init(void)
{
	if (!usb_context) {
		const char *locale;

		/* Init Libusb */
		if (libusb_init(&usb_context))
			return -1;

		/* Set the locale if it's not set. */
		locale = setlocale(LC_CTYPE, NULL);
		if (!locale)
			setlocale(LC_CTYPE, "");
	}

	return 0;
}

int HID_API_EXPORT hid_exit(void)
{
	usb_string_cache_destroy();

	if (usb_context) {
		libusb_exit(usb_context);
		usb_context = NULL;
	}

	return 0;
}

static int is_xbox360(unsigned short vendor_id, const struct libusb_interface_descriptor *intf_desc)
{
	static const int XB360_IFACE_SUBCLASS = 93;
	static const int XB360_IFACE_PROTOCOL = 1; /* Wired */
	static const int XB360W_IFACE_PROTOCOL = 129; /* Wireless */
	static const int SUPPORTED_VENDORS[] = {
		0x0079, /* GPD Win 2 */
		0x044f, /* Thrustmaster */
		0x045e, /* Microsoft */
		0x046d, /* Logitech */
		0x056e, /* Elecom */
		0x06a3, /* Saitek */
		0x0738, /* Mad Catz */
		0x07ff, /* Mad Catz */
		0x0e6f, /* PDP */
		0x0f0d, /* Hori */
		0x1038, /* SteelSeries */
		0x11c9, /* Nacon */
		0x12ab, /* Unknown */
		0x1430, /* RedOctane */
		0x146b, /* BigBen */
		0x1532, /* Razer Sabertooth */
		0x15e4, /* Numark */
		0x162e, /* Joytech */
		0x1689, /* Razer Onza */
		0x1949, /* Lab126, Inc. */
		0x1bad, /* Harmonix */
		0x20d6, /* PowerA */
		0x24c6, /* PowerA */
	};

	if (intf_desc->bInterfaceClass == LIBUSB_CLASS_VENDOR_SPEC &&
	    intf_desc->bInterfaceSubClass == XB360_IFACE_SUBCLASS &&
	    (intf_desc->bInterfaceProtocol == XB360_IFACE_PROTOCOL ||
	     intf_desc->bInterfaceProtocol == XB360W_IFACE_PROTOCOL)) {
		int i;
		for (i = 0; i < sizeof(SUPPORTED_VENDORS)/sizeof(SUPPORTED_VENDORS[0]); ++i) {
			if (vendor_id == SUPPORTED_VENDORS[i]) {
				return 1;
			}
		}
	}
	return 0;
}

static int is_xboxone(unsigned short vendor_id, const struct libusb_interface_descriptor *intf_desc)
{
	static const int XB1_IFACE_SUBCLASS = 71;
	static const int XB1_IFACE_PROTOCOL = 208;
	static const int SUPPORTED_VENDORS[] = {
		0x045e, /* Microsoft */
		0x0738, /* Mad Catz */
		0x0e6f, /* PDP */
		0x0f0d, /* Hori */
		0x1532, /* Razer Wildcat */
		0x20d6, /* PowerA */
		0x24c6, /* PowerA */
		0x2e24, /* Hyperkin */
	};

	if (intf_desc->bInterfaceNumber == 0 &&
	    intf_desc->bInterfaceClass == LIBUSB_CLASS_VENDOR_SPEC &&
	    intf_desc->bInterfaceSubClass == XB1_IFACE_SUBCLASS &&
	    intf_desc->bInterfaceProtocol == XB1_IFACE_PROTOCOL) {
		int i;
		for (i = 0; i < sizeof(SUPPORTED_VENDORS)/sizeof(SUPPORTED_VENDORS[0]); ++i) {
			if (vendor_id == SUPPORTED_VENDORS[i]) {
				return 1;
			}
		}
	}
	return 0;
}

static int should_enumerate_interface(unsigned short vendor_id, const struct libusb_interface_descriptor *intf_desc)
{
	if (intf_desc->bInterfaceClass == LIBUSB_CLASS_HID)
		return 1;

	/* Also enumerate Xbox 360 controllers */
	if (is_xbox360(vendor_id, intf_desc))
		return 1;

	/* Also enumerate Xbox One controllers */
	if (is_xboxone(vendor_id, intf_desc))
		return 1;

	return 0;
}

struct hid_device_info  HID_API_EXPORT *hid_enumerate(unsigned short vendor_id, unsigned short product_id)
{
	libusb_device **devs;
	libusb_device *dev;
	libusb_device_handle *handle;
	ssize_t num_devs;
	int i = 0;

	struct hid_device_info *root = NULL; /* return object */
	struct hid_device_info *cur_dev = NULL;

	if(hid_init() < 0)
		return NULL;

	num_devs = libusb_get_device_list(usb_context, &devs);
	if (num_devs < 0)
		return NULL;
	while ((dev = devs[i++]) != NULL) {
		struct libusb_device_descriptor desc;
		struct libusb_config_descriptor *conf_desc = NULL;
		int j, k;
		int interface_num = 0;

		int res = libusb_get_device_descriptor(dev, &desc);
		unsigned short dev_vid = desc.idVendor;
		unsigned short dev_pid = desc.idProduct;

		res = libusb_get_active_config_descriptor(dev, &conf_desc);
		if (res < 0)
			libusb_get_config_descriptor(dev, 0, &conf_desc);
		if (conf_desc) {
			for (j = 0; j < conf_desc->bNumInterfaces; j++) {
				const struct libusb_interface *intf = &conf_desc->interface[j];
				for (k = 0; k < intf->num_altsetting; k++) {
					const struct libusb_interface_descriptor *intf_desc;
					intf_desc = &intf->altsetting[k];
					if (should_enumerate_interface(dev_vid, intf_desc)) {
						interface_num = intf_desc->bInterfaceNumber;

						/* Check the VID/PID against the arguments */
						if ((vendor_id == 0x0 || vendor_id == dev_vid) &&
						    (product_id == 0x0 || product_id == dev_pid)) {
							res = libusb_open(dev, &handle);

							if (res >= 0) {
								struct hid_device_info *tmp;
								const struct usb_string_cache_entry *string_cache;

								/* VID/PID match. Create the record. */
								tmp = (struct hid_device_info*) calloc(1, sizeof(struct hid_device_info));
								if (cur_dev) {
									cur_dev->next = tmp;
								}
								else {
									root = tmp;
								}
								cur_dev = tmp;

								/* Fill out the record */
								cur_dev->next = NULL;
								cur_dev->path = make_path(dev, interface_num);

								/* Serial Number */
								if (desc.iSerialNumber > 0)
									cur_dev->serial_number =
										get_usb_string(handle, desc.iSerialNumber);

								/* Manufacturer and Product strings */
								if (dev_vid && dev_pid) {
									string_cache = usb_string_cache_find(&desc, handle);
									if (string_cache) {
										if (string_cache->vendor) {
											cur_dev->manufacturer_string = wcsdup(string_cache->vendor);
										}
										if (string_cache->product) {
											cur_dev->product_string = wcsdup(string_cache->product);
										}
									}
								} else {
									if (desc.iManufacturer > 0)
										cur_dev->manufacturer_string =
											get_usb_string(handle, desc.iManufacturer);
									if (desc.iProduct > 0)
										cur_dev->product_string =
											get_usb_string(handle, desc.iProduct);
								}

#ifdef INVASIVE_GET_USAGE
{
							/*
							This section is removed because it is too
							invasive on the system. Getting a Usage Page
							and Usage requires parsing the HID Report
							descriptor. Getting a HID Report descriptor
							involves claiming the interface. Claiming the
							interface involves detaching the kernel driver.
							Detaching the kernel driver is hard on the system
							because it will unclaim interfaces (if another
							app has them claimed) and the re-attachment of
							the driver will sometimes change /dev entry names.
							It is for these reasons that this section is
							#if 0. For composite devices, use the interface
							field in the hid_device_info struct to distinguish
							between interfaces. */
								unsigned char data[256];
#ifdef DETACH_KERNEL_DRIVER
								int detached = 0;
								/* Usage Page and Usage */
								res = libusb_kernel_driver_active(handle, interface_num);
								if (res == 1) {
									res = libusb_detach_kernel_driver(handle, interface_num);
									if (res < 0)
										LOG("Couldn't detach kernel driver, even though a kernel driver was attached.");
									else
										detached = 1;
								}
#endif
								res = libusb_claim_interface(handle, interface_num);
								if (res >= 0) {
									/* Get the HID Report Descriptor. */
									res = libusb_control_transfer(handle, LIBUSB_ENDPOINT_IN|LIBUSB_RECIPIENT_INTERFACE, LIBUSB_REQUEST_GET_DESCRIPTOR, (LIBUSB_DT_REPORT << 8)|interface_num, 0, data, sizeof(data), 5000);
									if (res >= 0) {
										unsigned short page=0, usage=0;
										/* Parse the usage and usage page
										   out of the report descriptor. */
										get_usage(data, res,  &page, &usage);
										cur_dev->usage_page = page;
										cur_dev->usage = usage;
									}
									else
										LOG("libusb_control_transfer() for getting the HID report failed with %d\n", res);

									/* Release the interface */
									res = libusb_release_interface(handle, interface_num);
									if (res < 0)
										LOG("Can't release the interface.\n");
								}
								else
									LOG("Can't claim interface %d\n", res);
#ifdef DETACH_KERNEL_DRIVER
								/* Re-attach kernel driver if necessary. */
								if (detached) {
									res = libusb_attach_kernel_driver(handle, interface_num);
									if (res < 0)
										LOG("Couldn't re-attach kernel driver.\n");
								}
#endif
}
#endif /* INVASIVE_GET_USAGE */

								libusb_close(handle);

								/* VID/PID */
								cur_dev->vendor_id = dev_vid;
								cur_dev->product_id = dev_pid;

								/* Release Number */
								cur_dev->release_number = desc.bcdDevice;

								/* Interface Number */
								cur_dev->interface_number = interface_num;
								cur_dev->interface_class = intf_desc->bInterfaceClass;
								cur_dev->interface_subclass = intf_desc->bInterfaceSubClass;
								cur_dev->interface_protocol = intf_desc->bInterfaceProtocol;

							} else
								LOG("Can't open device 0x%.4x/0x%.4x during enumeration: %d\n", dev_vid, dev_pid, res);
						}
					}
				} /* altsettings */
			} /* interfaces */
			libusb_free_config_descriptor(conf_desc);
		}
	}

	libusb_free_device_list(devs, 1);

	return root;
}

void  HID_API_EXPORT hid_free_enumeration(struct hid_device_info *devs)
{
	struct hid_device_info *d = devs;
	while (d) {
		struct hid_device_info *next = d->next;
		free(d->path);
		free(d->serial_number);
		free(d->manufacturer_string);
		free(d->product_string);
		free(d);
		d = next;
	}
}

hid_device * hid_open(unsigned short vendor_id, unsigned short product_id, const wchar_t *serial_number)
{
	struct hid_device_info *devs, *cur_dev;
	const char *path_to_open = NULL;
	hid_device *handle = NULL;

	devs = hid_enumerate(vendor_id, product_id);
	cur_dev = devs;
	while (cur_dev) {
		if (cur_dev->vendor_id == vendor_id &&
		    cur_dev->product_id == product_id) {
			if (serial_number) {
				if (cur_dev->serial_number &&
				    wcscmp(serial_number, cur_dev->serial_number) == 0) {
					path_to_open = cur_dev->path;
					break;
				}
			}
			else {
				path_to_open = cur_dev->path;
				break;
			}
		}
		cur_dev = cur_dev->next;
	}

	if (path_to_open) {
		/* Open the device */
		handle = hid_open_path(path_to_open, 0);
	}

	hid_free_enumeration(devs);

	return handle;
}

static void LIBUSB_CALL read_callback(struct libusb_transfer *transfer)
{
	hid_device *dev = (hid_device *)transfer->user_data;
	int res;

	if (transfer->status == LIBUSB_TRANSFER_COMPLETED) {

		struct input_report *rpt = (struct input_report*) malloc(sizeof(*rpt));
		rpt->data = (uint8_t*) malloc(transfer->actual_length);
		memcpy(rpt->data, transfer->buffer, transfer->actual_length);
		rpt->len = transfer->actual_length;
		rpt->next = NULL;

		SDL_LockMutex(dev->mutex);

		/* Attach the new report object to the end of the list. */
		if (dev->input_reports == NULL) {
			/* The list is empty. Put it at the root. */
			dev->input_reports = rpt;
			SDL_CondSignal(dev->condition);
		}
		else {
			/* Find the end of the list and attach. */
			struct input_report *cur = dev->input_reports;
			int num_queued = 0;
			while (cur->next != NULL) {
				cur = cur->next;
				num_queued++;
			}
			cur->next = rpt;

			/* Pop one off if we've reached 30 in the queue. This
			   way we don't grow forever if the user never reads
			   anything from the device. */
			if (num_queued > 30) {
				return_data(dev, NULL, 0);
			}
		}
		SDL_UnlockMutex(dev->mutex);
	}
	else if (transfer->status == LIBUSB_TRANSFER_CANCELLED) {
		dev->shutdown_thread = 1;
	}
	else if (transfer->status == LIBUSB_TRANSFER_NO_DEVICE) {
		dev->shutdown_thread = 1;
	}
	else if (transfer->status == LIBUSB_TRANSFER_TIMED_OUT) {
		//LOG("Timeout (normal)\n");
	}
	else {
		LOG("Unknown transfer code: %d\n", transfer->status);
	}

	if (dev->shutdown_thread) {
		dev->transfer_loop_finished = 1;
		return;
	}

	/* Re-submit the transfer object. */
	res = libusb_submit_transfer(transfer);
	if (res != 0) {
		LOG("Unable to submit URB. libusb error code: %d\n", res);
		dev->shutdown_thread = 1;
		dev->transfer_loop_finished = 1;
	}
}


static int read_thread(void *param)
{
	hid_device *dev = (hid_device *)param;
	uint8_t *buf;
	const size_t length = dev->input_ep_max_packet_size;

	/* Set up the transfer object. */
	buf = (uint8_t*) malloc(length);
	dev->transfer = libusb_alloc_transfer(0);
	libusb_fill_interrupt_transfer(dev->transfer,
		dev->device_handle,
		dev->input_endpoint,
		buf,
		length,
		read_callback,
		dev,
		5000/*timeout*/);

	/* Make the first submission. Further submissions are made
	   from inside read_callback() */
	libusb_submit_transfer(dev->transfer);

	/* Notify the main thread that the read thread is up and running. */
	SDL_WaitThreadBarrier(&dev->barrier);

	/* Handle all the events. */
	while (!dev->shutdown_thread) {
		int res;
		res = libusb_handle_events(usb_context);
		if (res < 0) {
			/* There was an error. */
			LOG("read_thread(): libusb reports error # %d\n", res);

			/* Break out of this loop only on fatal error.*/
			if (res != LIBUSB_ERROR_BUSY &&
			    res != LIBUSB_ERROR_TIMEOUT &&
			    res != LIBUSB_ERROR_OVERFLOW &&
			    res != LIBUSB_ERROR_INTERRUPTED) {
				dev->shutdown_thread = 1;
				break;
			}
		}
	}

	/* Cancel any transfer that may be pending. This call will fail
	   if no transfers are pending, but that's OK. */
	libusb_cancel_transfer(dev->transfer);

	while (!dev->transfer_loop_finished)
		libusb_handle_events_completed(usb_context, &dev->transfer_loop_finished);

	/* Now that the read thread is stopping, Wake any threads which are
	   waiting on data (in hid_read_timeout()). Do this under a mutex to
	   make sure that a thread which is about to go to sleep waiting on
	   the condition actually will go to sleep before the condition is
	   signaled. */
	SDL_LockMutex(dev->mutex);
	SDL_CondBroadcast(dev->condition);
	SDL_UnlockMutex(dev->mutex);

	/* The dev->transfer->buffer and dev->transfer objects are cleaned up
	   in hid_close(). They are not cleaned up here because this thread
	   could end either due to a disconnect or due to a user
	   call to hid_close(). In both cases the objects can be safely
	   cleaned up after the call to pthread_join() (in hid_close()), but
	   since hid_close() calls libusb_cancel_transfer(), on these objects,
	   they can not be cleaned up here. */

	return 0;
}

static void init_xboxone(libusb_device_handle *device_handle, struct libusb_config_descriptor *conf_desc)
{
	static const int XB1_IFACE_SUBCLASS = 71;
	static const int XB1_IFACE_PROTOCOL = 208;
	int j, k, res;

	for (j = 0; j < conf_desc->bNumInterfaces; j++) {
		const struct libusb_interface *intf = &conf_desc->interface[j];
		for (k = 0; k < intf->num_altsetting; k++) {
			const struct libusb_interface_descriptor *intf_desc;
			intf_desc = &intf->altsetting[k];

			if (intf_desc->bInterfaceNumber != 0 &&
			    intf_desc->bAlternateSetting == 0 &&
			    intf_desc->bInterfaceClass == LIBUSB_CLASS_VENDOR_SPEC &&
			    intf_desc->bInterfaceSubClass == XB1_IFACE_SUBCLASS &&
			    intf_desc->bInterfaceProtocol == XB1_IFACE_PROTOCOL) {
				res = libusb_claim_interface(device_handle, intf_desc->bInterfaceNumber);
				if (res < 0) {
					LOG("can't claim interface %d: %d\n", intf_desc->bInterfaceNumber, res);
					continue;
				}

				res = libusb_set_interface_alt_setting(device_handle, intf_desc->bInterfaceNumber, intf_desc->bAlternateSetting);
				if (res < 0) {
					LOG("xbox init: can't set alt setting %d: %d\n", intf_desc->bInterfaceNumber, res);
				}

				libusb_release_interface(device_handle, intf_desc->bInterfaceNumber);
			}
		}
	}
}

hid_device * HID_API_EXPORT hid_open_path(const char *path, int bExclusive)
{
	hid_device *dev = NULL;

	libusb_device **devs;
	libusb_device *usb_dev;
	int res;
	int d = 0;
	int good_open = 0;

	if(hid_init() < 0)
		return NULL;

	dev = new_hid_device();

	libusb_get_device_list(usb_context, &devs);
	while ((usb_dev = devs[d++]) != NULL) {
		struct libusb_device_descriptor desc;
		struct libusb_config_descriptor *conf_desc = NULL;
		int i,j,k;

		libusb_get_device_descriptor(usb_dev, &desc);

		res = libusb_get_active_config_descriptor(usb_dev, &conf_desc);
		if (res < 0)
			libusb_get_config_descriptor(usb_dev, 0, &conf_desc);
		if (!conf_desc)
			continue;
		for (j = 0; j < conf_desc->bNumInterfaces && !good_open; j++) {
			const struct libusb_interface *intf = &conf_desc->interface[j];
			for (k = 0; k < intf->num_altsetting && !good_open; k++) {
				const struct libusb_interface_descriptor *intf_desc;
				intf_desc = &intf->altsetting[k];
				if (should_enumerate_interface(desc.idVendor, intf_desc)) {
					char *dev_path = make_path(usb_dev, intf_desc->bInterfaceNumber);
					if (!strcmp(dev_path, path)) {
						int detached_driver = 0;

						/* Matched Paths. Open this device */

						/* OPEN HERE */
						res = libusb_open(usb_dev, &dev->device_handle);
						if (res < 0) {
							LOG("can't open device\n");
							free(dev_path);
							break;
						}
						good_open = 1;

#ifdef DETACH_KERNEL_DRIVER
						/* Detach the kernel driver, but only if the
						   device is managed by the kernel */
						if (libusb_kernel_driver_active(dev->device_handle, intf_desc->bInterfaceNumber) == 1) {
							res = libusb_detach_kernel_driver(dev->device_handle, intf_desc->bInterfaceNumber);
							if (res < 0) {
								libusb_close(dev->device_handle);
								LOG("Unable to detach Kernel Driver\n");
								free(dev_path);
								good_open = 0;
								break;
							}
							detached_driver = 1;
						}
#endif

						res = libusb_claim_interface(dev->device_handle, intf_desc->bInterfaceNumber);
						if (res < 0) {
							LOG("can't claim interface %d: %d\n", intf_desc->bInterfaceNumber, res);
							free(dev_path);
							libusb_close(dev->device_handle);
							good_open = 0;
							break;
						}

						/* Initialize XBox One controllers */
						if (is_xboxone(desc.idVendor, intf_desc)) {
							init_xboxone(dev->device_handle, conf_desc);
						}

						/* Store off the string descriptor indexes */
						dev->manufacturer_index = desc.iManufacturer;
						dev->product_index      = desc.iProduct;
						dev->serial_index       = desc.iSerialNumber;

						/* Store off the interface number */
						dev->interface = intf_desc->bInterfaceNumber;
						dev->detached_driver = detached_driver;

						/* Find the INPUT and OUTPUT endpoints. An
						   OUTPUT endpoint is not required. */
						for (i = 0; i < intf_desc->bNumEndpoints; i++) {
							const struct libusb_endpoint_descriptor *ep
								= &intf_desc->endpoint[i];

							/* Determine the type and direction of this
							   endpoint. */
							int is_interrupt =
								(ep->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK)
							      == LIBUSB_TRANSFER_TYPE_INTERRUPT;
							int is_output =
								(ep->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK)
							      == LIBUSB_ENDPOINT_OUT;
							int is_input =
								(ep->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK)
							      == LIBUSB_ENDPOINT_IN;

							/* Decide whether to use it for input or output. */
							if (dev->input_endpoint == 0 &&
							    is_interrupt && is_input) {
								/* Use this endpoint for INPUT */
								dev->input_endpoint = ep->bEndpointAddress;
								dev->input_ep_max_packet_size = ep->wMaxPacketSize;
							}
							if (dev->output_endpoint == 0 &&
							    is_interrupt && is_output) {
								/* Use this endpoint for OUTPUT */
								dev->output_endpoint = ep->bEndpointAddress;
							}
						}

						dev->thread = SDL_CreateThread(read_thread, NULL, dev);

						/* Wait here for the read thread to be initialized. */
						SDL_WaitThreadBarrier(&dev->barrier);

					}
					free(dev_path);
				}
			}
		}
		libusb_free_config_descriptor(conf_desc);

	}

	libusb_free_device_list(devs, 1);

	/* If we have a good handle, return it. */
	if (good_open) {
		return dev;
	}
	else {
		/* Unable to open any devices. */
		free_hid_device(dev);
		return NULL;
	}
}


int HID_API_EXPORT hid_write(hid_device *dev, const unsigned char *data, size_t length)
{
	int res;

	if (dev->output_endpoint <= 0) {
		int report_number = data[0];
		int skipped_report_id = 0;

		if (report_number == 0x0) {
			data++;
			length--;
			skipped_report_id = 1;
		}

		/* No interrupt out endpoint. Use the Control Endpoint */
		res = libusb_control_transfer(dev->device_handle,
			LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE|LIBUSB_ENDPOINT_OUT,
			0x09/*HID Set_Report*/,
			(2/*HID output*/ << 8) | report_number,
			dev->interface,
			(unsigned char *)data, length,
			1000/*timeout millis*/);

		if (res < 0)
			return -1;

		if (skipped_report_id)
			length++;

		return length;
	}
	else {
		/* Use the interrupt out endpoint */
		int actual_length;
		res = libusb_interrupt_transfer(dev->device_handle,
			dev->output_endpoint,
			(unsigned char*)data,
			length,
			&actual_length, 1000);

		if (res < 0)
			return -1;

		return actual_length;
	}
}

/* Helper function, to simplify hid_read().
   This should be called with dev->mutex locked. */
static int return_data(hid_device *dev, unsigned char *data, size_t length)
{
	/* Copy the data out of the linked list item (rpt) into the
	   return buffer (data), and delete the liked list item. */
	struct input_report *rpt = dev->input_reports;
	size_t len = (length < rpt->len)? length: rpt->len;
	if (data && len > 0)
		memcpy(data, rpt->data, len);
	dev->input_reports = rpt->next;
	free(rpt->data);
	free(rpt);
	return len;
}

#if 0 /* TODO: pthread_cleanup SDL? */
static void cleanup_mutex(void *param)
{
	hid_device *dev = (hid_device *)param;
	SDL_UnlockMutex(dev->mutex);
}
#endif

int HID_API_EXPORT hid_read_timeout(hid_device *dev, unsigned char *data, size_t length, int milliseconds)
{
#if 0
	int transferred;
	int res = libusb_interrupt_transfer(dev->device_handle, dev->input_endpoint, data, length, &transferred, 5000);
	LOG("transferred: %d\n", transferred);
	return transferred;
#endif
	int bytes_read;

	SDL_LockMutex(dev->mutex);
	/* TODO: pthread_cleanup SDL? */

	bytes_read = -1;

	/* There's an input report queued up. Return it. */
	if (dev->input_reports) {
		/* Return the first one */
		bytes_read = return_data(dev, data, length);
		goto ret;
	}

	if (dev->shutdown_thread) {
		/* This means the device has been disconnected.
		   An error code of -1 should be returned. */
		bytes_read = -1;
		goto ret;
	}

	if (milliseconds == -1) {
		/* Blocking */
		while (!dev->input_reports && !dev->shutdown_thread) {
			SDL_CondWait(dev->condition, dev->mutex);
		}
		if (dev->input_reports) {
			bytes_read = return_data(dev, data, length);
		}
	}
	else if (milliseconds > 0) {
		/* Non-blocking, but called with timeout. */
		int res;

		while (!dev->input_reports && !dev->shutdown_thread) {
			res = SDL_CondWaitTimeout(dev->condition, dev->mutex, milliseconds);
			if (res == 0) {
				if (dev->input_reports) {
					bytes_read = return_data(dev, data, length);
					break;
				}

				/* If we're here, there was a spurious wake up
				   or the read thread was shutdown. Run the
				   loop again (ie: don't break). */
			}
			else if (res == SDL_MUTEX_TIMEDOUT) {
				/* Timed out. */
				bytes_read = 0;
				break;
			}
			else {
				/* Error. */
				bytes_read = -1;
				break;
			}
		}
	}
	else {
		/* Purely non-blocking */
		bytes_read = 0;
	}

ret:
	SDL_UnlockMutex(dev->mutex);
	/* TODO: pthread_cleanup SDL? */

	return bytes_read;
}

int HID_API_EXPORT hid_read(hid_device *dev, unsigned char *data, size_t length)
{
	return hid_read_timeout(dev, data, length, dev->blocking ? -1 : 0);
}

int HID_API_EXPORT hid_set_nonblocking(hid_device *dev, int nonblock)
{
	dev->blocking = !nonblock;

	return 0;
}


int HID_API_EXPORT hid_send_feature_report(hid_device *dev, const unsigned char *data, size_t length)
{
	int res = -1;
	int skipped_report_id = 0;
	int report_number = data[0];

	if (report_number == 0x0) {
		data++;
		length--;
		skipped_report_id = 1;
	}

	res = libusb_control_transfer(dev->device_handle,
		LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE|LIBUSB_ENDPOINT_OUT,
		0x09/*HID set_report*/,
		(3/*HID feature*/ << 8) | report_number,
		dev->interface,
		(unsigned char *)data, length,
		1000/*timeout millis*/);

	if (res < 0)
		return -1;

	/* Account for the report ID */
	if (skipped_report_id)
		length++;

	return length;
}

int HID_API_EXPORT hid_get_feature_report(hid_device *dev, unsigned char *data, size_t length)
{
	int res = -1;
	int skipped_report_id = 0;
	int report_number = data[0];

	if (report_number == 0x0) {
		/* Offset the return buffer by 1, so that the report ID
		   will remain in byte 0. */
		data++;
		length--;
		skipped_report_id = 1;
	}
	res = libusb_control_transfer(dev->device_handle,
		LIBUSB_REQUEST_TYPE_CLASS|LIBUSB_RECIPIENT_INTERFACE|LIBUSB_ENDPOINT_IN,
		0x01/*HID get_report*/,
		(3/*HID feature*/ << 8) | report_number,
		dev->interface,
		(unsigned char *)data, length,
		1000/*timeout millis*/);

	if (res < 0)
		return -1;

	if (skipped_report_id)
		res++;

	return res;
}


void HID_API_EXPORT hid_close(hid_device *dev)
{
	int status;

	if (!dev)
		return;

	/* Cause read_thread() to stop. */
	dev->shutdown_thread = 1;
	libusb_cancel_transfer(dev->transfer);

	/* Wait for read_thread() to end. */
	SDL_WaitThread(dev->thread, &status);

	/* Clean up the Transfer objects allocated in read_thread(). */
	free(dev->transfer->buffer);
	libusb_free_transfer(dev->transfer);

	/* release the interface */
	libusb_release_interface(dev->device_handle, dev->interface);

#ifdef DETACH_KERNEL_DRIVER
	/* Re-attach kernel driver if necessary. */
	if (dev->detached_driver) {
		int res = libusb_attach_kernel_driver(dev->device_handle, dev->interface);
		if (res < 0)
			LOG("Couldn't re-attach kernel driver.\n");
	}
#endif

	/* Close the handle */
	libusb_close(dev->device_handle);

	/* Clear out the queue of received reports. */
	SDL_LockMutex(dev->mutex);
	while (dev->input_reports) {
		return_data(dev, NULL, 0);
	}
	SDL_UnlockMutex(dev->mutex);

	free_hid_device(dev);
}


int HID_API_EXPORT_CALL hid_get_manufacturer_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	return hid_get_indexed_string(dev, dev->manufacturer_index, string, maxlen);
}

int HID_API_EXPORT_CALL hid_get_product_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	return hid_get_indexed_string(dev, dev->product_index, string, maxlen);
}

int HID_API_EXPORT_CALL hid_get_serial_number_string(hid_device *dev, wchar_t *string, size_t maxlen)
{
	return hid_get_indexed_string(dev, dev->serial_index, string, maxlen);
}

int HID_API_EXPORT_CALL hid_get_indexed_string(hid_device *dev, int string_index, wchar_t *string, size_t maxlen)
{
	wchar_t *str;

	str = get_usb_string(dev->device_handle, string_index);
	if (str) {
		wcsncpy(string, str, maxlen);
		string[maxlen-1] = L'\0';
		free(str);
		return 0;
	}
	else
		return -1;
}


HID_API_EXPORT const wchar_t * HID_API_CALL  hid_error(hid_device *dev)
{
	return NULL;
}


struct lang_map_entry {
	const char *name;
	const char *string_code;
	uint16_t usb_code;
};

#define LANG(name,code,usb_code) { name, code, usb_code }
static struct lang_map_entry lang_map[] = {
	LANG("Afrikaans", "af", 0x0436),
	LANG("Albanian", "sq", 0x041C),
	LANG("Arabic - United Arab Emirates", "ar_ae", 0x3801),
	LANG("Arabic - Bahrain", "ar_bh", 0x3C01),
	LANG("Arabic - Algeria", "ar_dz", 0x1401),
	LANG("Arabic - Egypt", "ar_eg", 0x0C01),
	LANG("Arabic - Iraq", "ar_iq", 0x0801),
	LANG("Arabic - Jordan", "ar_jo", 0x2C01),
	LANG("Arabic - Kuwait", "ar_kw", 0x3401),
	LANG("Arabic - Lebanon", "ar_lb", 0x3001),
	LANG("Arabic - Libya", "ar_ly", 0x1001),
	LANG("Arabic - Morocco", "ar_ma", 0x1801),
	LANG("Arabic - Oman", "ar_om", 0x2001),
	LANG("Arabic - Qatar", "ar_qa", 0x4001),
	LANG("Arabic - Saudi Arabia", "ar_sa", 0x0401),
	LANG("Arabic - Syria", "ar_sy", 0x2801),
	LANG("Arabic - Tunisia", "ar_tn", 0x1C01),
	LANG("Arabic - Yemen", "ar_ye", 0x2401),
	LANG("Armenian", "hy", 0x042B),
	LANG("Azeri - Latin", "az_az", 0x042C),
	LANG("Azeri - Cyrillic", "az_az", 0x082C),
	LANG("Basque", "eu", 0x042D),
	LANG("Belarusian", "be", 0x0423),
	LANG("Bulgarian", "bg", 0x0402),
	LANG("Catalan", "ca", 0x0403),
	LANG("Chinese - China", "zh_cn", 0x0804),
	LANG("Chinese - Hong Kong SAR", "zh_hk", 0x0C04),
	LANG("Chinese - Macau SAR", "zh_mo", 0x1404),
	LANG("Chinese - Singapore", "zh_sg", 0x1004),
	LANG("Chinese - Taiwan", "zh_tw", 0x0404),
	LANG("Croatian", "hr", 0x041A),
	LANG("Czech", "cs", 0x0405),
	LANG("Danish", "da", 0x0406),
	LANG("Dutch - Netherlands", "nl_nl", 0x0413),
	LANG("Dutch - Belgium", "nl_be", 0x0813),
	LANG("English - Australia", "en_au", 0x0C09),
	LANG("English - Belize", "en_bz", 0x2809),
	LANG("English - Canada", "en_ca", 0x1009),
	LANG("English - Caribbean", "en_cb", 0x2409),
	LANG("English - Ireland", "en_ie", 0x1809),
	LANG("English - Jamaica", "en_jm", 0x2009),
	LANG("English - New Zealand", "en_nz", 0x1409),
	LANG("English - Philippines", "en_ph", 0x3409),
	LANG("English - Southern Africa", "en_za", 0x1C09),
	LANG("English - Trinidad", "en_tt", 0x2C09),
	LANG("English - Great Britain", "en_gb", 0x0809),
	LANG("English - United States", "en_us", 0x0409),
	LANG("Estonian", "et", 0x0425),
	LANG("Farsi", "fa", 0x0429),
	LANG("Finnish", "fi", 0x040B),
	LANG("Faroese", "fo", 0x0438),
	LANG("French - France", "fr_fr", 0x040C),
	LANG("French - Belgium", "fr_be", 0x080C),
	LANG("French - Canada", "fr_ca", 0x0C0C),
	LANG("French - Luxembourg", "fr_lu", 0x140C),
	LANG("French - Switzerland", "fr_ch", 0x100C),
	LANG("Gaelic - Ireland", "gd_ie", 0x083C),
	LANG("Gaelic - Scotland", "gd", 0x043C),
	LANG("German - Germany", "de_de", 0x0407),
	LANG("German - Austria", "de_at", 0x0C07),
	LANG("German - Liechtenstein", "de_li", 0x1407),
	LANG("German - Luxembourg", "de_lu", 0x1007),
	LANG("German - Switzerland", "de_ch", 0x0807),
	LANG("Greek", "el", 0x0408),
	LANG("Hebrew", "he", 0x040D),
	LANG("Hindi", "hi", 0x0439),
	LANG("Hungarian", "hu", 0x040E),
	LANG("Icelandic", "is", 0x040F),
	LANG("Indonesian", "id", 0x0421),
	LANG("Italian - Italy", "it_it", 0x0410),
	LANG("Italian - Switzerland", "it_ch", 0x0810),
	LANG("Japanese", "ja", 0x0411),
	LANG("Korean", "ko", 0x0412),
	LANG("Latvian", "lv", 0x0426),
	LANG("Lithuanian", "lt", 0x0427),
	LANG("F.Y.R.O. Macedonia", "mk", 0x042F),
	LANG("Malay - Malaysia", "ms_my", 0x043E),
	LANG("Malay ??? Brunei", "ms_bn", 0x083E),
	LANG("Maltese", "mt", 0x043A),
	LANG("Marathi", "mr", 0x044E),
	LANG("Norwegian - Bokml", "no_no", 0x0414),
	LANG("Norwegian - Nynorsk", "no_no", 0x0814),
	LANG("Polish", "pl", 0x0415),
	LANG("Portuguese - Portugal", "pt_pt", 0x0816),
	LANG("Portuguese - Brazil", "pt_br", 0x0416),
	LANG("Raeto-Romance", "rm", 0x0417),
	LANG("Romanian - Romania", "ro", 0x0418),
	LANG("Romanian - Republic of Moldova", "ro_mo", 0x0818),
	LANG("Russian", "ru", 0x0419),
	LANG("Russian - Republic of Moldova", "ru_mo", 0x0819),
	LANG("Sanskrit", "sa", 0x044F),
	LANG("Serbian - Cyrillic", "sr_sp", 0x0C1A),
	LANG("Serbian - Latin", "sr_sp", 0x081A),
	LANG("Setsuana", "tn", 0x0432),
	LANG("Slovenian", "sl", 0x0424),
	LANG("Slovak", "sk", 0x041B),
	LANG("Sorbian", "sb", 0x042E),
	LANG("Spanish - Spain (Traditional)", "es_es", 0x040A),
	LANG("Spanish - Argentina", "es_ar", 0x2C0A),
	LANG("Spanish - Bolivia", "es_bo", 0x400A),
	LANG("Spanish - Chile", "es_cl", 0x340A),
	LANG("Spanish - Colombia", "es_co", 0x240A),
	LANG("Spanish - Costa Rica", "es_cr", 0x140A),
	LANG("Spanish - Dominican Republic", "es_do", 0x1C0A),
	LANG("Spanish - Ecuador", "es_ec", 0x300A),
	LANG("Spanish - Guatemala", "es_gt", 0x100A),
	LANG("Spanish - Honduras", "es_hn", 0x480A),
	LANG("Spanish - Mexico", "es_mx", 0x080A),
	LANG("Spanish - Nicaragua", "es_ni", 0x4C0A),
	LANG("Spanish - Panama", "es_pa", 0x180A),
	LANG("Spanish - Peru", "es_pe", 0x280A),
	LANG("Spanish - Puerto Rico", "es_pr", 0x500A),
	LANG("Spanish - Paraguay", "es_py", 0x3C0A),
	LANG("Spanish - El Salvador", "es_sv", 0x440A),
	LANG("Spanish - Uruguay", "es_uy", 0x380A),
	LANG("Spanish - Venezuela", "es_ve", 0x200A),
	LANG("Southern Sotho", "st", 0x0430),
	LANG("Swahili", "sw", 0x0441),
	LANG("Swedish - Sweden", "sv_se", 0x041D),
	LANG("Swedish - Finland", "sv_fi", 0x081D),
	LANG("Tamil", "ta", 0x0449),
	LANG("Tatar", "tt", 0X0444),
	LANG("Thai", "th", 0x041E),
	LANG("Turkish", "tr", 0x041F),
	LANG("Tsonga", "ts", 0x0431),
	LANG("Ukrainian", "uk", 0x0422),
	LANG("Urdu", "ur", 0x0420),
	LANG("Uzbek - Cyrillic", "uz_uz", 0x0843),
	LANG("Uzbek ??? Latin", "uz_uz", 0x0443),
	LANG("Vietnamese", "vi", 0x042A),
	LANG("Xhosa", "xh", 0x0434),
	LANG("Yiddish", "yi", 0x043D),
	LANG("Zulu", "zu", 0x0435),
	LANG(NULL, NULL, 0x0),
};

uint16_t get_usb_code_for_current_locale(void)
{
	char *locale;
	char search_string[64];
	char *ptr;
	struct lang_map_entry *lang;

	/* Get the current locale. */
	locale = setlocale(0, NULL);
	if (!locale)
		return 0x0;

	/* Make a copy of the current locale string. */
	strncpy(search_string, locale, sizeof(search_string));
	search_string[sizeof(search_string)-1] = '\0';

	/* Chop off the encoding part, and make it lower case. */
	ptr = search_string;
	while (*ptr) {
		*ptr = tolower(*ptr);
		if (*ptr == '.') {
			*ptr = '\0';
			break;
		}
		ptr++;
	}

	/* Find the entry which matches the string code of our locale. */
	lang = lang_map;
	while (lang->string_code) {
		if (!strcmp(lang->string_code, search_string)) {
			return lang->usb_code;
		}
		lang++;
	}

	/* There was no match. Find with just the language only. */
	/* Chop off the variant. Chop it off at the '_'. */
	ptr = search_string;
	while (*ptr) {
		*ptr = tolower(*ptr);
		if (*ptr == '_') {
			*ptr = '\0';
			break;
		}
		ptr++;
	}

#if 0 /* TODO: Do we need this? */
	/* Find the entry which matches the string code of our language. */
	lang = lang_map;
	while (lang->string_code) {
		if (!strcmp(lang->string_code, search_string)) {
			return lang->usb_code;
		}
		lang++;
	}
#endif

	/* Found nothing. */
	return 0x0;
}

#if defined(__cplusplus) && !defined(NAMESPACE)
}
#endif

#ifdef NAMESPACE
}
#endif

#endif /* SDL_JOYSTICK_HIDAPI */
