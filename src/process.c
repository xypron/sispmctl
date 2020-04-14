/*
  process.c - process web requests

  (C) 2015-2020, Heinrich Schuchardt <xypron.glpk@gmx.de>
  (C) 2011-2016, Pete Hildebrandt <send2ph@gmail.com>
  (C) 2010, Olivier Matheret, France, for the scheduling part
  (C) 2004-2011, Mondrian Nuessle, Computer Architecture Group,
      University of Mannheim, Germany
  (C) 2005, Andreas Neuper, Germany

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <syslog.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <usb.h>
#include "config.h"
#include "sispm_ctl.h"

#define BSIZE	65536
int debug = 1;
int verbose = 1;
#ifdef DATADIR
char *homedir = DATADIR;
#else
char *homedir = 0;
#endif

#ifndef WEBLESS
char *secret;

static char msg_unauthorized[] =
	"HTTP/1.1 401 Unauthorized\nServer: SisPM\n"
	"WWW-Authenticate: Basic realm=\"SisPM\n\""
	"Content-Type: text/html\n\n"
	"<!DOCTYPE HTML>\n"
	"<html><head>\n<title>401 Unauthorized</title>\n"
	"<meta http-equiv=\"refresh\" content=\"10;url=/\">\n"
	"</head><body>\n"
	"<h1>401 Unauthorized</h1></body></html>\n\n";

static char msg_not_found[] = 
	"HTTP/1.1 404 Not found\n"
	"Content-Type: text/html\n\n"
	"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" "
	"\"http://www.w3.org/TR/html4/loose.dtd\">\n"
	"<html><head>\n<title>404 Not found</title>\n"
	"<meta http-equiv=\"refresh\" content=\"2;url=/\">\n"
	"</head><body>\n"
	"<h1>404 Not found</h1></body></html>\n\n";

static char msg_not_available[] =
	"HTTP/1.1 503 Service not available\n"
	"Content-Type: text/html\n\n"
	"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" "
	"\"http://www.w3.org/TR/html4/loose.dtd\">\n"
	"<html><head>\n<title>503 Service not available</title>\n"
	"<meta http-equiv=\"refresh\" content=\"2;url=/\">\n"
	"</head><body>\n"
	"<h1>503 Service not available</h1></body></html>\n\n";

static void service_not_available(int out)
{
	send(out, msg_not_available, strlen(msg_not_available), 0);
}

static void unauthorized(int out)
{
	send(out, msg_unauthorized, strlen(msg_unauthorized), 0);
}

static void bad_request(int out)
{
	send(out, msg_not_found, strlen(msg_not_found), 0);
}

char *next_word(char *ptr)
{
	bool flag = false;

	if (!ptr) {
		return ptr;
	}
	for (;; ++ptr) {
		char c = *ptr;

		if (c < ' ') {
			return NULL;
		}
		if (c == ' ') {
			flag = true;
		} else if (flag) {
			return ptr;
		}
	}
}

void process(int out, char *request)
{
	char xbuffer[BSIZE+2];
	char filename[1024];
	char *eol, *ptr;
	FILE *in = NULL;
	long length = 0;
	long lastpos = 0;
	long remlen = 0;
	char *retvalue = NULL;

	/* Make sure the string is terminated */
	request[BUFFERSIZE - 1] = 0;
	if (debug)
		syslog(LOG_DEBUG,"Requested is\n(%s)\n",request);

	/* Extract the file name */
	memset(filename, 0, sizeof(filename));
	eol = strchr(request, '\n');
	if (eol) {
		*eol = 0;
		ptr = strchr(request, ' ');
		if (ptr)
			strncpy(filename, strchr(request, ' ') + 1,
				sizeof(filename) - 1);
		ptr = strchr(filename, ' ');
		if (ptr)
			*ptr = 0;
	}
	/* Look for authentication */
	if (secret) {
		char *password = NULL;

		for(; eol;) {
			ptr = eol + 1;
			if (strncmp(ptr, "Authorization: ", 15)) {
				eol = strchr(ptr, '\n');
				continue;
			}
			ptr = next_word(ptr);
			password = next_word(ptr);
			if (!password) {
				break;
			}
			for (ptr = password; *ptr > ' '; ++ptr)
				;
			*ptr = '\0';
			break;
		}
		if (!password || strcmp(secret, password)) {
			unauthorized(out);
			return;
		}
	}

	/*
	 * avoid reading other directories
	 * TODO: URL encoding is not resolved
	 */
	ptr = strrchr(filename,'/');
	if (ptr != NULL)
		++ptr;
	else
		ptr = filename;

	if (strlen(ptr) == 0)
		ptr="index.html";

	printf("File: '%s'\n", ptr);

	if (!strcmp(ptr, "index.html")) {

	}

	if (!strcmp(ptr, "on")) {
		return;
	}

	if (!strcmp(ptr, "off")) {
		return;
	}

	if (!strcmp(ptr, "toggle")) {
		return;
	}

	if (debug) {
		syslog(LOG_DEBUG, "requested file name(%s)\n", filename);
		syslog(LOG_DEBUG, "resulting file name(%s)\n", ptr);
		syslog(LOG_DEBUG, "change directory to (%s)\n", homedir);
	}

	if (chdir(homedir) != 0) {
		syslog(LOG_ERR, "Cannot access directory %s\n", homedir);
		bad_request(out);
		return;
	}

	if (debug)
		syslog(LOG_DEBUG,"open file(%s)\n",ptr);

	in = fopen(ptr,"r");

	if (in == NULL) {
		syslog(LOG_ERR, "Cannot open %s, %s\n", ptr, strerror(errno));
		bad_request(out);
		return;
	}

	lastpos = ftell(in);

	while (!feof(in)) {
		memset(xbuffer, 0, BSIZE);
		retvalue = fgets(xbuffer, BSIZE - 1, in);
		remlen = length = ftell(in) - lastpos;
		lastpos = ftell(in);
		if (retvalue == NULL) {
			break;
		}
		send(out, xbuffer, remlen, 0);
	}

	fclose(in);
	return;
}
#endif
