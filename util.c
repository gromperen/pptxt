/* See LICENSE file for copyright and license details. */
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ftw.h>

#include "util.h"

void *
xmalloc(size_t len)
{
	void *p;

	if (!(p = malloc(len)))
		die("malloc: %s\n", strerror(errno));

	return p;
}

void *
ecalloc(size_t nmemb, size_t size)
{
	void *p;
	if(!(p = calloc(nmemb, size)))
		die("calloc:");
	return p;
}

void
die(const char *fmt, ...) 
{
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (fmt[0] && fmt[strlen(fmt)-1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else {
		fputc('\n', stderr);
	}

	exit(1);
}

void
safe_create_dir(const char *dir)
{
	struct stat st = {0};
	if (stat(dir, &st) == -1) {
		mkdir(dir, 0755);
	}
	return;
}

void
rec_mkdir(char *path)
{
	char *sep = strrchr(path, '/');
	if (sep != NULL) {
		*sep = 0;
		rec_mkdir(path);
		*sep = '/';
	}
	mkdir(path, 0755);
}

FILE *fopen_mkdir(char *path, char *mode) 
{
	char *sep = strrchr(path, '/');
	if (sep) {
		char *path0 = strdup(path);
		path0[sep - path] = 0;
		rec_mkdir(path0);
		free(path0);
	}
	return fopen(path, mode);
}

int unlink_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
	    int rv = remove(fpath);

		    if (rv)
				        perror(fpath);

						    return rv;
}

int rmrf(char *path)
{
	    return nftw(path, unlink_cb, 64, FTW_DEPTH | FTW_PHYS);
}

void *
xrealloc(void *p, size_t len)
{
	if ((p = realloc(p, len)) == NULL) {
		die("realloc: %s\n", strerror(errno));
	}
	return p;
}
