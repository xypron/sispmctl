// SPDX-License-Identifier: GPL-2.0+
/*
 * USB connectivity
 *
 * Copyright (c) 2020 Heinrich Schuchardt
 */

#include <errno.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include "sispm_ctl.h"

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

#define DEV_INFO_TXT 	"Gembird #%d\n" \
			"USB information:  bus %03d, device %03d\n" \
			"device type:      %d-socket SiS-PM\n" \
			"serial number:    %s\n\n"
#define DEV_INFO_NUM	"%d %03d %03d\n%d\n%s\n\n"

struct pms_device {
	libusb_device *dev;
	uint16_t vendor_id;
	uint16_t product_id;
	uint8_t bus_number;
	uint8_t device_address;
	uint8_t first_port;
	uint8_t port_count;
	char serial_id[15];
};

struct pms_device_list {
	size_t count;
	struct pms_device device[0];
};

static uint16_t supported_product_ids[] = {
	PRODUCT_ID_SISPM,
	PRODUCT_ID_MSISPM_OLD,
	PRODUCT_ID_MSISPM_FLASH,
	PRODUCT_ID_SISPM_FLASH_NEW,
	PRODUCT_ID_SISPM_EG_PMS2,
};

/**
 * usb_device_supported() - check if the device is supported
 *
 * @desc:	USB device descriptor
 * Return:	0 if supported
 */
static int usb_device_supported(struct libusb_device_descriptor *desc)
{
	size_t i;

	if (desc->idVendor != VENDOR_ID)
		return -1;
	for (i = 0; i < ARRAY_SIZE(supported_product_ids); ++i) {
		if (desc->idProduct == supported_product_ids[i])
			return 0;
	}
	return -1;
}

/**
 * usb_control_transfer() - exchange data with USB device
 *
 * Up to five attempts to transfer the data are made.
 *
 * @handle:	device handle
 * @type:	request type for setup packet
 * @request:	request field for setup packet
 * @value:	value field for setup packet
 * @index:	index field for setup packet
 * @data:	data buffer
 * @length:	size of data buffer
 * Return:	number of bytes transferred or error code
 */
static int usb_control_transfer(libusb_device_handle *handle, uint8_t type,
                                uint8_t request, uint16_t value, uint16_t index,
                                unsigned char *data, uint16_t length)
{
	int ret, i;

	for (i = 0; i < 5; ++i) {
		usleep(500 * i);
		ret = libusb_control_transfer(handle, type, request, value,
		                              index, data, length, 5000);
		if (ret == length)
			break;
	}
	return ret;
}

/**
 * usb_get_serial() - retrieve the serial number of a Gembird device
 *
 * @dev:	device
 * @serial:	buffer to receive serial number (must be 15 bytes long)
 * @len:	size of the buffer
 * Return:	0 for success
 */
static int usb_get_serial(libusb_device *dev, char *serial, size_t len)
{
	int ret;
	unsigned char buffer[5];
	libusb_device_handle *handle;

	if (len < 15)
		return LIBUSB_ERROR_OTHER;

	ret = libusb_open(dev, &handle);
	if (ret)
		goto err;

	ret = usb_control_transfer(handle, 0xa1, 0x01, 0x301, 0, buffer,
	                           sizeof(buffer));

	if (ret < 0)
		goto err;

	if (ret == 5) {
		snprintf(serial, len, "%02x:%02x:%02x:%02x:%02x\n",
		         buffer[0], buffer[1], buffer[2], buffer[3], buffer[4]);
		ret = 0;
	} else {
		ret = LIBUSB_ERROR_OTHER;
	}
err:
	libusb_close(handle);

	if (ret < 0)
		printf("Error %s\n", libusb_error_name(ret));

	return ret;
}

/**
 * pms_serial_id_cmp() - compare two PMS devices by serial ID
 *
 * This function is used for sorting the PMS devices by serial ID.
 */
static int pms_serial_id_cmp(const void *a, const void *b) {
	return strcmp(((struct pms_device *)a)->serial_id,
		      ((struct pms_device *)b)->serial_id);
}

/**
 * pms_get_list() - get PMS device list
 *
 * A list of PMS devices sorted by serial ID is returned.
 *
 * After usage call pms_destroy_list() to adjust reference counts and to free
 * memory.
 *
 * @context:	USB context
 * @pms_list:	pointer to receive list of PMS devices
 *		After usage call 
 * Return:	0 for success
 */
int pms_get_list(libusb_context *context, struct pms_device_list **pms_list)
{
	libusb_device **list;
	int ret;
	ssize_t count, list_size, i;

	list_size = libusb_get_device_list(context, &list);
	*pms_list= malloc(sizeof(struct pms_device_list) +
			  list_size * sizeof(struct pms_device));
	if (!*pms_list) {
		return -ENOMEM;
	}
	for (i = 0, count = 0; count < MAXGEMBIRD && i < list_size; ++i) {
		struct libusb_device_descriptor desc;
		libusb_device *dev = list[i];
		struct pms_device *pms = &(*pms_list)->device[count];

		ret = libusb_get_device_descriptor(dev, &desc);
		if (ret)
			continue;
		if (usb_device_supported(&desc))
			continue;
		if (!usb_get_serial(list[i], pms->serial_id,
				    sizeof(pms->serial_id))) {
			/* Increment the reference count of the PMS device */
			pms->dev = libusb_ref_device(dev);
			pms->vendor_id = desc.idVendor;
			pms->product_id = desc.idProduct;
			pms->bus_number = libusb_get_bus_number(dev);
			pms->device_address = libusb_get_device_address(dev);
			switch (desc.idProduct) {
			case PRODUCT_ID_MSISPM_OLD:
				pms->port_count = 1;
				pms->first_port = 0;
				break;
			case PRODUCT_ID_MSISPM_FLASH:
				pms->port_count = 1;
				pms->first_port = 1;
				break;
			default:
				pms->port_count = 4;
				pms->first_port = 1;
			}
			++count;
		}
	}
	qsort((*pms_list)->device, count, sizeof(struct pms_device),
	      pms_serial_id_cmp);

	/* Free the list and decrement the reference count of all devices */
	libusb_free_device_list(list, 1);

	(*pms_list)->count = count;
	return 0;
}

/**
 * pms_destroy_list() - destroy a list of PMS devices
 *
 * The reference count of the USB devices is decremented and the allocated
 * memory is freed.
 *
 * @list:	list to destroy
 */
void pms_destroy_list(struct pms_device_list *list) {
	size_t i;

	if (!list)
		return;
	for (i = 0; i < list->count; ++i)
		/* Decrement reference count of the PMS devices */
		libusb_unref_device(list->device[i].dev);
	free(list);
}

/**
 * pms_list_devices() - print a list of PMS devices
 *
 * @context:	USB context
 * @numeric:	numeric output
 */
int pms_list_devices(libusb_context *context, bool numeric) {
	struct pms_device_list *pms_devices;
	size_t i;
	int ret;

	ret = pms_get_list(context, &pms_devices);
	if (ret)
		return ret;

	for (i = 0; i < pms_devices->count; ++i) {
		struct pms_device *pms = &pms_devices->device[i];
		char *format = numeric ? DEV_INFO_NUM : DEV_INFO_TXT;

		printf(format, i, pms->bus_number, pms->device_address,
		       pms->port_count, pms->serial_id);
	}
	pms_destroy_list(pms_devices);
	return 0;
}

int main()
{
	int ret;
	libusb_context *context;

	ret = libusb_init(&context);
	if (ret)
		return 1;

	pms_list_devices(context, false);
	pms_list_devices(context, true);

	libusb_exit(context);
	return 0;
}
