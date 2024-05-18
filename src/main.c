#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sispmctl.h"

#define SETID -1001

/**
 * list_devices() - list devices
 *
 * @e:		environment
 */
static void list_devices(struct environment *e)
{
	for (struct sispm_device *ptr = e->list; ptr->dev; ++ptr) {
		if (e->numeric) {
			printf("%zu %03d %03d\n", ptr - e->list, ptr->bus,
			       ptr->addr);
			printf("%d\n", ptr->max_outlet - ptr->min_outlet + 1);
			printf("%s\n\n", ptr->id);
		} else {
			printf("Gembird #%zu\n", ptr - e->list);
			printf("USB information:  bus %03d, device %03d\n",
			       ptr->bus, ptr->addr);
			printf("device type:	  %d-output %sSiS-PM\n",
			       ptr->max_outlet - ptr->min_outlet + 1,
			       ptr->max_outlet - ptr->min_outlet ? "" : "m");
			printf("serial number:	  %s\n\n", ptr->id);
		}
	}
}

/**
 * version() - print version information
 */
static void version(void)
{
	fprintf(stderr, "SiS PM Control for Linux " PACKAGE_VERSION "\n");
}

/**
 * usage() - print usage information
 */
static void usage(void)
{
	fprintf(stderr, "\n"
			"Usage: sispmctl <arguments>\n"
			"\n"
			"  -d, --device IDX  select device by index\n"
			"  -D, --id ID       select device by serial number\n"
			"  -f, --off NUM     switch output NUM off\n"
			"  -g, --status NUM  show status of output NUM\n"
			"  -h, --help        display this help\n"
			"  -n, --numeric     numeric output\n"
			"  -o, --on NUM      switch output NUM on\n"
			"  -q, --quiet       reduce verbosity\n"
			"  -s, --list        enumerate SiS-PM devices\n"
			"  -t, --toggle NUM  toggle output NUM\n"
			"  -U, --usb BUS:DEV select by USB bus and device\n"
			"  -v, --version     show version information\n"
			"      --setid ID    set serial number 01:##:##:##:##\n");
}

static int execute(struct environment *e,
		   int (*f)(struct environment *e, struct sispm_device *dev,
			    int outlet),
		   struct sispm_device *dev, const char *optarg)
{
	long outlet;

	if (e->verbose)
		printf("Accessing Gembird #%d, USB device %03d:%03d\n",
		       dev->num, dev->bus, dev->addr);

	if (!optarg)
		return EXIT_FAILURE;

	if (!strcmp(optarg, "all")) {
		for (int i = dev->min_outlet; i <= dev->max_outlet; ++i) {
			f(e, dev, i);
		}
		return EXIT_SUCCESS;
	}

	outlet = strtol(optarg, NULL, 0);
	if (outlet < 1) {
		fprintf(stderr, "Minimum outlet number of device %d is 1\n",
			dev->num);
		return EXIT_FAILURE;
	}
	if (outlet > dev->max_outlet - dev->min_outlet + 1) {
		fprintf(stderr, "Maximum outlet number of device %d is %d\n",
			dev->num, dev->max_outlet - dev->min_outlet + 1);
		return EXIT_FAILURE;
	}
	outlet += dev->min_outlet - 1;

	f(e, dev, outlet);

	return EXIT_SUCCESS;
}

static void print_outlet_status(struct environment *e, int status)
{
	if (e->numeric)
		printf("%s\n", status ? "1" : "0");
	else
		printf("%s\n", status ? "on" : "off");
}

static int off(struct environment *e, struct sispm_device *dev, int outlet)
{
	int ret;

	ret = sis_switch_off(dev, outlet);
	if (ret)
		return EXIT_FAILURE;

	if (e->verbose) {
		printf("Switched outlet %d ", outlet - dev->min_outlet + 1);
		print_outlet_status(e, 0);
	}

	return EXIT_SUCCESS;
}

static int on(struct environment *e, struct sispm_device *dev, int outlet)
{
	int ret;

	ret = sis_switch_on(dev, outlet);
	if (ret)
		return EXIT_FAILURE;

	if (e->verbose) {
		printf("Switched outlet %d ", outlet - dev->min_outlet + 1);
		print_outlet_status(e, 1);
	}

	return EXIT_SUCCESS;
}

static int toggle(struct environment *e, struct sispm_device *dev, int outlet)
{
	int ret;
	int status;

	ret = sis_get_status(dev, outlet, &status);
	if (ret)
		return EXIT_FAILURE;

	if (status)
		return off(e, dev, outlet);
	else
		return on(e, dev, outlet);
}

static int get_status(struct environment *e, struct sispm_device *dev,
		      int outlet)
{
	int ret;
	int status;

	ret = sis_get_status(dev, outlet, &status);
	if (ret)
		return EXIT_FAILURE;

	if (e->verbose)
		printf("Status of outlet %d:	",
		       outlet - dev->min_outlet + 1);

	print_outlet_status(e, status);

	return EXIT_SUCCESS;
}

static int set_id(struct sispm_device *dev, const char *optarg)
{
	unsigned char buffer[5] = { 0 };
	char *str, *s;
	size_t index;
	int ret;

	if (dev->product_id < 0xfd13) {
		fprintf(stderr,
			"Setting serial nummber is not supported on this device\n");
		return EXIT_FAILURE;
	}

	str = strdup(optarg);
	if (!str) {
		fprintf(stderr, "Out of memory\n");
		return EXIT_FAILURE;
	}

	for (index = 0, s = str; index < sizeof(buffer); ++index) {
		char *part;	
		long val;
		char *rem;

		part = strtok(s, ":");
		s = NULL;
		if (!part)
			goto err;
		if (strlen(part) != 2)
			goto err;
		val = strtol(part, &rem, 16);
		if (*rem || val < 0 || val > 0xff)
			goto err;
		buffer[index] = val;
	}
	if (index != 5)
		goto err;

	if (buffer[0] != 0x01)
		goto err;

	ret = sis_write_id(dev, buffer, sizeof(buffer));
	if (ret < 0)
		goto fail;

	printf("Serial number of device #%d updated to %s\n",
	       dev->num, dev->id);

	free(str);

	return EXIT_SUCCESS;
	
err:
	fprintf(stderr, "serial number must be in 01:##:##:##:## format\n");
fail:
	free(str);
	return EXIT_FAILURE;
}

/**
 * parse_options() - parse command line arguments
 *
 * @e:		environment
 * @argc:	argument count
 * @argv:	argument values
 * Return:	0 on success
 */
static int parse_options(struct environment *e, int argc, char *const argv[])
{
	int index;
	struct sispm_device *dev = e->list;
	static const struct option long_options[] = {
		{ "device", required_argument, NULL, 'd' },
		{ "help", no_argument, NULL, 'h' },
		{ "id", required_argument, NULL, 'D' },
		{ "numeric", no_argument, NULL, 'n' },
		{ "off", required_argument, NULL, 'f' },
		{ "on", required_argument, NULL, 'o' },
		{ "list", no_argument, NULL, 's' },
		{ "quiet", no_argument, NULL, 'q' },
		{ "setid", required_argument, NULL, SETID },
		{ "status", required_argument, NULL, 'g' },
		{ "toggle", required_argument, NULL, 't' },
		{ "usb", required_argument, NULL, 'U' },
		{ "version", no_argument, NULL, 'v' },
		{ NULL, 0, NULL, 0 }
	};

	if (argc <= 1) {
		usage();
		return EXIT_FAILURE;
	}

	for (;;) {
		int opt;
		int ret = 0;

		opt = getopt_long(argc, argv, "D:d:f:g:hno:qst:U:v", long_options,
				  &index);

		switch (opt) {
		case -1:
			if (argv[optind]) {
				fprintf(stderr, "excess arguments\n");
				usage();
				return EXIT_FAILURE;
			}

			return EXIT_SUCCESS;
		case 'd': {
			long n;

			n = strtol(optarg, NULL, 0);
			if (n < 0 || n >= e->count) {
				fprintf(stderr, "invalid device number\n");
				return EXIT_FAILURE;
			}
			dev = &e->list[n];
			break;
		}
		case 'D':
			dev = NULL;
			if (e->list) {
				for (struct sispm_device *ptr = e->list;
				     ptr->dev; ++ptr) {
					if (!strcmp(ptr->id, optarg))
						dev = ptr;
				}
			}
			if (!dev) {
				fprintf(stderr, "device id not found\n");
				return EXIT_FAILURE;
			}
			break;
		case 'f':
			ret = execute(e, off, dev, optarg);
			break;
		case 'g':
			ret = execute(e, get_status, dev, optarg);
			break;
		case '?':
		case 'h':
			usage();
			return EXIT_FAILURE;
		case 'n':
			e->numeric = 1;
			break;
		case 'o':
			ret = execute(e, on, dev, optarg);
			break;
		case 'q':
			e->verbose = 0;
			break;
		case 's':
			list_devices(e);
			break;
		case 't':
			ret = execute(e, toggle, dev, optarg);
			break;
		case 'U':
			printf("on(%s) not implemented, yet\n", optarg);
			break;
		case 'v':
			version();
			return EXIT_FAILURE;
		case SETID:
			ret = set_id(dev, optarg);
			break;
		}

		if (ret) {
			fprintf(stderr, "Aborted due to error\n");
			return ret;
		}
	}

	return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
	int ret;
	struct environment e = { .verbose = 1 };

	if (sis_connect(&e))
		return EXIT_FAILURE;

	ret = parse_options(&e, argc, argv);

	sis_deconnect(&e);

	return ret;
}
