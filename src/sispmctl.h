#include <libusb.h>

struct sispm_device {
	libusb_device *dev;
	char id[15];
	int min_outlet;
	int max_outlet;
	int num;
	int bus;
	int addr;
	int product_id;
};

struct environment {
	libusb_context *ctx;
	struct sispm_device *list;
	int count;
	int numeric;
	int verbose;
};

int sis_switch_off(struct sispm_device *dev, int outlet);

int sis_switch_on(struct sispm_device *dev, int outlet);

int sis_get_status(struct sispm_device *dev, int outlet, int *status);

int sis_write_id(struct sispm_device *dev, unsigned char *buffer, size_t size);

int sis_connect(struct environment *e);

void sis_deconnect(struct environment *e);
