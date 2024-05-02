#include "sispmctl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * error() - print error message for libusb library error
 *
 * Prints the function where the error occurred followed by a string
 * describing the libusb return code.
 *
 * @ret:	libusb return code
 */
#define error(text, ret) error_msg(__func__, text, ret)

/**
 * error_msg() - write error message to stderr
 *
 * @text:	text describing the error
 * @ret:	return code from libusb library function
 */
static void error_msg(const char *func, const char *text, int ret)
{
	fprintf(stderr, "%s: %s - %s\n", func, text, libusb_error_name(ret));
}

static int intcmp(int a, int b)
{
	if (a < b)
		return -1;
	if (a > b)
		return 1;
	return 0;
}

static int sis_compare(const void *left, const void *right)
{
	int ret;
	const struct sispm_device *const a = left;
	const struct sispm_device *const b = right;

	ret = strcmp(a->id, b->id);
	if (ret)
		return ret;

	ret = intcmp(a->bus, b->bus);
	if (ret)
		return ret;

	return intcmp(a->addr, b->addr);
}

/**
 * is_sispm() - check if a USB device is a SiS-PM device
 *
 * @dev:	USB device
 * Return:	1 for SisPM device, 0 otherwise
 */
int is_sispm(struct libusb_device *dev)
{
	struct libusb_device_descriptor desc;
	int ret;

	ret = libusb_get_device_descriptor(dev, &desc);
	if (ret) {
		error("get device descriptor", ret);
		return 0;
	}

	if (desc.idVendor != 0x04b4)
		return 0;

	switch (desc.idProduct) {
	case 0xfd10:
	case 0xfd11:
	case 0xfd12:
	case 0xfd13:
	case 0xfd15:
		return 1;
	default:
		break;
	}

	return 0;
}

static int sis_usb_control_transfer(libusb_device_handle *handle,
				    int requesttype, int request, int value,
				    int index, unsigned char *bytes, size_t size,
				    int timeout)
{
	int ret;
	unsigned char buf[5];

	if (size > sizeof(buf)) {
		return LIBUSB_ERROR_INVALID_PARAM;
	}

	for (int i = 0; i < 5; ++i) {
		usleep(500 * i);
		memcpy(buf, bytes, size);
		ret = libusb_control_transfer(handle, requesttype, request,
					      value, index, buf, size,
					      timeout);
		if (ret == size)
			break;
	}
	memcpy(bytes, buf, size);

	return ret;
}

/**
 * sis_count() - count SiS-PM devices
 *
 * @list:	list of USB devices
 * Return:	number of SiS-PM devices
 */
size_t sis_count(libusb_device **list)
{
	size_t count = 0;

	for (libusb_device **pos = list; *pos; ++pos) {
		if (is_sispm(*pos))
			++count;
	}

	return count;
}

static libusb_device_handle *sis_open(struct sispm_device *dev)
{
	libusb_device_handle *handle;
	int ret;

	ret = libusb_open(dev->dev, &handle);
	if (ret) {
		error("open device", ret);
		return NULL;
	}

	ret = libusb_set_configuration(handle, 1);
	if (ret) {
		error("set configuration", ret);
		goto err;
	}

	ret = libusb_claim_interface(handle, 0);
	if (ret) {
		error("claim interface", ret);
		goto err;
	}

	ret = libusb_set_interface_alt_setting(handle, 0, 0);
	if (ret) {
		error("set iterface alt setting", ret);
		goto err;
	}

	return handle;

err:
	libusb_close(handle);

	return NULL;
}

void sis_close(libusb_device_handle *handle)
{
	int ret;

	ret = libusb_release_interface(handle, 0);
	if (ret)
		error("release interface", ret);

	libusb_close(handle);
}

/**
 * sis_read_info() - get information about SiS-PM device
 *
 * @dev:	USB device
 * Return:	1 for SisPM device, 0 otherwise
 */
static int sis_read_info(struct sispm_device *dev)
{
	struct libusb_device_descriptor desc;
	int ret;

	ret = libusb_get_device_descriptor(dev->dev, &desc);
	if (ret) {
		error("get device descriptor", ret);
		return 0;
	}

	if (desc.idVendor != 0x04b4)
		return 0;

	switch (desc.idProduct) {
	case 0xfd10:
		dev->min_outlet = 0;
		dev->max_outlet = 0;
		break;
	case 0xfd11:
		dev->min_outlet = 1;
		dev->max_outlet = 1;
		break;
	case 0xfd12:
	case 0xfd13:
	case 0xfd15:
		dev->min_outlet = 1;
		dev->max_outlet = 4;
		break;
	default:
		return 0;
	}

	dev->bus = libusb_get_bus_number(dev->dev);
	dev->addr = libusb_get_device_address(dev->dev);
	dev->product_id = desc.idProduct;

	return 1;
}

static int sis_read_id(struct sispm_device *dev)
{
	libusb_device_handle *handle;
	unsigned char buffer[5] = { 0 };
	int ret;

	handle = sis_open(dev);
	if (!handle)
		return 1;

	ret = sis_usb_control_transfer(handle, 0xa1, 0x01, 0x301, 0, buffer,
				       sizeof(buffer), 5000);
	if (ret < 0)
		error("control transfer", ret);
	else if (ret == 5)
		snprintf(dev->id, sizeof(dev->id), "%02x:%02x:%02x:%02x:%02x",
			 buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);

	sis_close(handle);

	return ret;
}

int sis_write_id(struct sispm_device *dev, unsigned char *buffer, size_t size)
{
	libusb_device_handle *handle;
	int ret;

	handle = sis_open(dev);
	if (!handle)
		return 1;

	ret = sis_usb_control_transfer(handle, 0x21, 0x09, 0x301, 0, buffer,
				       size, 5000);
	if (ret < 0)
		error("control transfer", ret);

	snprintf(dev->id, sizeof(dev->id), "%02x:%02x:%02x:%02x:%02x",
		 buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);

	return ret;
}

int sis_switch_off(struct sispm_device *dev, int outlet)
{
	libusb_device_handle *handle;
	unsigned char buffer[5] = { 3 * outlet };
	int ret;

	handle = sis_open(dev);
	if (!handle)
		return 1;

	ret = sis_usb_control_transfer(handle, 0x21, 0x09, 0x300 + 3 * outlet,
				       0, buffer, sizeof(buffer), 5000);
	if (ret < 0)
		error("control transfer", ret);
	else
		ret = 0;

	sis_close(handle);

	return ret;
}

int sis_switch_on(struct sispm_device *dev, int outlet)
{
	libusb_device_handle *handle;
	unsigned char buffer[5] = { 3 * outlet, 3 };
	int ret;

	handle = sis_open(dev);
	if (!handle)
		return 1;

	ret = sis_usb_control_transfer(handle, 0x21, 0x09, 0x300 + 3 * outlet,
				       0, buffer, sizeof(buffer), 5000);
	if (ret < 0)
		error("control transfer", ret);
	else
		ret = 0;

	sis_close(handle);

	return ret;
}

int sis_get_status(struct sispm_device *dev, int outlet, int *status)
{
	libusb_device_handle *handle;
	unsigned char buffer[5] = { 3 * outlet, 3 };
	int ret;

	handle = sis_open(dev);
	if (!handle)
		return 1;

	ret = sis_usb_control_transfer(handle, 0xa1, 0x01, 0x300 + 3 * outlet,
				       0, buffer, sizeof(buffer), 5000);
	if (ret < 0) {
		error("control transfer", ret);
	} else {
		*status = 1 & buffer[1];
		ret = 0;
	}

	sis_close(handle);

	return ret;
}

int sis_connect(struct environment *e)
{
	int ret;
	ssize_t dev_cnt;
	libusb_device **list;
	struct sispm_device *ptr;

	if (e->ctx)
		return 0;

	ret = libusb_init_context(&e->ctx, NULL, 0);
	if (ret) {
		e->ctx = NULL;
		error("initialize context", ret);
		return 1;
	}

	dev_cnt = libusb_get_device_list(e->ctx, &list);
	if (dev_cnt < 0) {
		error("get device list", dev_cnt);
		return 1;
	}
	e->count = sis_count(list);
	if (!e->count) {
		e->list = NULL;
		return 1;
	}
	e->list = calloc(e->count + 1, sizeof(struct sispm_device));
	if (!e->list) {
		perror("Out of memory");
		libusb_free_device_list(list, 1);
		return 1;
	}

	ptr = e->list;
	for (libusb_device **pos = list; *pos; ++pos) {
		if (is_sispm(*pos)) {
			ptr->dev = *pos;
			libusb_ref_device(*pos);
			sis_read_info(ptr);
			sis_read_id(ptr);
			ptr++;
		}
	}
	qsort(e->list, e->count, sizeof(struct sispm_device), sis_compare);

	for (struct sispm_device *ptr = e->list; ptr->dev; ++ptr)
		ptr->num = ptr - e->list;

	libusb_free_device_list(list, 1);

	return 0;
}

void sis_deconnect(struct environment *e)
{
	if (e->list) {
		for (struct sispm_device *ptr = e->list; ptr->dev; ++ptr)
			libusb_unref_device(ptr->dev);
		free(e->list);
	}

	libusb_exit(e->ctx);
	e->ctx = NULL;
}
