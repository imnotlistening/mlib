/* (C) Copyright 2013
 * Alex Waterman <imNotListening@gmail.com>
 *
 * mlib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * mlib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mlib.  If not, see <http://www.gnu.org/licenses/>.
 *
 * This handles the core mechanics of dealing with libraries. Libraries are
 * memory mapped from disk into our virtual address space.
 */

#include <stdio.h>
#include <stdlib.h>

#include <sys/mman.h>

#include <mlib/mlib.h>

/*
 * A list for keeping track of all open libraries.
 */
struct mlib_library_listnode {
	struct list_head	 	 list;
	struct mlib_library_header	*library;
	int				 fd;
};

static LIST_HEAD(library_list);

/*
 * Open a local library.
 */
int __mlib_open_local_lib(const char *lib_name)
{
	struct stat sb;
	struct mlib_library_header *header;
	struct mlib_library_listnode *lib;

	lib = malloc(sizeof(struct mlib_library_listnode));
	if (!lib) {
		perror(lib_name);
		return -1;
	}

	lib->fd = open(lib_name, O_RDWR);
	if (lib->fd < 0) {
		perror(lib_name);
		goto fail;
	}

	if (fstat(lib->fd, &sb) == -1) {
		perror(lib_name);
		goto fail;
	}

	header = mmap(NULL, sb.st_size, PROT_READ|PROT_WRITE, MAP_SHARED,
		      lib->fd, 0); 

fail:
	free(lib);
}

/**
 * Open an MLib library pointed to by @location. If @remote is set then the
 * location is assumed to be a URL of some kind. Otherwise the library is
 * assumed to be a local file. Returns 0 on success, -1 on error.
 *
 * @url:	A URL to access.
 */
int mlib_open_library(const char *location, int remote)
{
	CURL *curl;

	if (remote) {
		/* TODO: support remote URLs. */
		mlib_printf("Remotes not yet supported.\n");
		return -1;
	} else
		return __mlib_open_local_lib(location);
}
