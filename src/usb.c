// SPDX-License-Identifier: GPL-2.0+
/*
 * USB connectivity
 *
 * Copyright (c) 2020 Heinrich Schuchardt
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <libusb-1.0/libusb.h>
#include "sispm_ctl.h"

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

#define DEV_INFO_TXT 	"Gembird #%d\n" \
			"USB information:  bus %03d, device %03d\n" \
			"device type:      %d-socket SiS-PM\n" \
			"serial number:    %s\n\n"
#define DEV_INFO_NUM "%d %03d %03d\n%d\n%s\n\n"

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
	char buffer[5];
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
 * usb_list_devices() - list usb devices
 *
 * @context:	USB context
 * @numeric:	numeric output
 */
void usb_list_devices(libusb_context *context, bool numeric)
{
	libusb_device **list;
	int ret;
	ssize_t list_size, i;
	char serial[15];
	size_t count;

	list_size = libusb_get_device_list(context, &list);
	for (i = 0, count = 0; count < MAXGEMBIRD && i < list_size; ++i) {
		struct libusb_device_descriptor desc;
		libusb_device *dev = list[i];

		ret = libusb_get_device_descriptor(dev, &desc);
		if (ret)
			continue;
		if (usb_device_supported(&desc))
			continue;
		if (!usb_get_serial(list[i], serial, sizeof(serial))) {
			char *format = numeric ? DEV_INFO_NUM : DEV_INFO_TXT;
			int ports;

			switch (desc.idProduct) {
			case PRODUCT_ID_MSISPM_OLD:
			case PRODUCT_ID_MSISPM_FLASH:
				ports = 1;
				break;
			default:
				ports = 4;
			}
			printf(format, count, libusb_get_bus_number(dev),
			       libusb_get_device_address(dev), ports, serial);
			++count;
		}
	}
	libusb_free_device_list(list, 0);
}

int main()
{
	int ret;
	libusb_context *context;

	ret = libusb_init(&context);
	if (ret)
		return 1;

	usb_list_devices(context, false);
	usb_list_devices(context, true);

	libusb_exit(context);
	return 0;
}
