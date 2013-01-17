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

#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <mlib/mlib.h>

#include <curl/curl.h>

static LIST_HEAD(library_list);

/*
 * Returns non-zero if a library with the same name as the passed library is
 * already open.
 */
int __mlib_library_already_open(struct mlib_library_header *lib)
{
	struct list_head *elem;
	struct mlib_library *cur_lib;
	struct mlib_library_header *header;

	list_for_each(elem, &library_list) {
		cur_lib = list_entry(elem, struct mlib_library, list);
		header = cur_lib->header;
		if (strncmp(lib->lib_name, header->lib_name,
			    MLIB_LIBRARY_LIB_NAME_LEN) == 0)
			return 1;
	}
	return 0;
}

/*
 * Expand the passed FD to the requested length. If the library is longer this
 * will overwrite existing data; this is for internal use only. Use
 * __mlib_library_expand() instead.
 */
int __mlib_library_expand_fd(int fd, size_t len)
{
	if (lseek(fd, len - 1, SEEK_SET) == -1)
		return -1;
	return write(fd, "", 1);
}

/*
 * Grow the size of a library to the requested length. If the library is
 * already bigger than the passed size an error is returned.
 */
int __mlib_library_expand(struct mlib_library *lib, size_t len)
{
	int ret;

	if (lib->header->lib_len >= len)
		return -1;
	ret = __mlib_library_expand_fd(lib->fd, len);
	if (ret)
		return ret;

	lib->header->lib_len = len;
	return 0;
}

/**
 * Create a library from scratch with @name located in @path.
 *
 * @path:		The path for the library.
 * @name:		The name to give the library.
 * @media_prefix:	The prefix to append to media paths if not absolute.
 */
int mlib_create_library(const char *path, const char *name,
			const char *media_prefix)
{
	int fd;
	struct mlib_library_header *header;

	if ((strlen(name) + 1 ) > MLIB_LIBRARY_LIB_NAME_LEN) {
		mlib_printf("Library name too long.\n");
		return -1;
	}
	if ((strlen(media_prefix) + 1 ) > MLIB_LIBRARY_MEDIA_PREFIX_LEN) {
		mlib_printf("Library media prefix too long.\n");
		return -1;
	}

	fd = open(path, O_CREAT|O_EXCL|O_RDWR,
		  S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (fd < 0) {
		mlib_perror("open: %s", path);
		return -1;
	}

	__mlib_library_expand_fd(fd, 1024);

	header = mmap(NULL, 1024, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0); 
	if (!header) {
		mlib_perror("mmap: %s", path);
		goto fail;
	}

	memset(header, MLIB_HEADER_SIZE, 0);
	header->mlib_magic = MLIB_MAGIC;
	header->media_count = 0;
	header->lib_len = 1024;
	memcpy(header->lib_name, name, strlen(name) + 1);
	memcpy(header->media_prefix, media_prefix, strlen(media_prefix) + 1);

	/* Sync and close. */
	msync(header, MLIB_HEADER_SIZE, MS_SYNC);
	close(fd);
	return 0;

fail:
	close(fd);
	return -1;
}

/*
 * Open a local library.
 */
struct mlib_library *__mlib_open_local_lib(const char *lib_name)
{
	struct stat sb;
	struct mlib_library *lib;
	struct mlib_library_header *header;

	lib = malloc(sizeof(struct mlib_library));
	if (!lib) {
		mlib_perror("malloc: %s", lib_name);
		return NULL;
	}

	lib->fd = open(lib_name, O_RDWR);
	if (lib->fd < 0) {
		mlib_perror("open: %s", lib_name);
		goto fail;
	}

	if (fstat(lib->fd, &sb) == -1) {
		mlib_perror("fstat: %s", lib_name);
		goto fail_2;
	}
	if (sb.st_size < 1024) {
		mlib_printf("%s: not an mlib library.\n", lib_name);
		goto fail_2;
	}

	header = mmap(NULL, sb.st_size, PROT_READ|PROT_WRITE, MAP_SHARED,
		      lib->fd, 0); 
	if (header == MAP_FAILED) {
		mlib_perror("mmap: %s", lib_name);
		goto fail_2;
	}
	if (header->mlib_magic != MLIB_MAGIC) {
		mlib_printf("%s: not an mlib library.\n", lib_name);
		goto fail_2;
	}

	/* Make sure the library is not already open. */
	if (__mlib_library_already_open(header)) {
		mlib_printf("Library %s is already open.\n", header->lib_name);
		goto fail_2;
	}

	lib->header = header;
	list_add_tail(&lib->list, &library_list);
	return lib;

fail_2:
	close(lib->fd);
fail:
	free(lib);
	return NULL;
}

/**
 * Open an MLib library pointed to by @location. If @remote is set then the
 * location is assumed to be a URL of some kind. Otherwise the library is
 * assumed to be a local file. Returns a pointer to the library on success or
 * NULL on failure.
 *
 * @url:	A URL to access.
 */
struct mlib_library *mlib_open_library(const char *location, int remote)
{
	CURL *curl __attribute__((unused));

	if (remote) {
		/* TODO: support remote URLs. */
		mlib_printf("Remotes not yet supported.\n");
		return NULL;
	} else {
		return __mlib_open_local_lib(location);
	}
}

/**
 * Close a library. Returns 0 on succes, -1 otherwise.
 *
 * @lib:	The library to close.
 */
int mlib_close_library(struct mlib_library *lib)
{
	int ret;

	list_del(&lib->list);

	ret = msync(lib->header, lib->header->lib_len, MS_SYNC);
	if (ret)
		mlib_perror("msync - warning");
	munmap(lib->header, lib->header->lib_len);

	close(lib->fd);
	free(lib);
	return ret;
}

/**
 * Find the pointer to the library with the passed @name. Returns a pointer to
 * the named libray if it exists or NULL if not.
 *
 * @name	The name of the library.
 */
struct mlib_library *mlib_find_library(const char *name)
{
	int name_len;
	struct list_head *elem;
	struct mlib_library *lib;
	struct mlib_library_header *header;

	list_for_each(elem, &library_list) {
		lib = list_entry(elem, struct mlib_library, list);
		header = lib->header;

		/* Print some useful info. */
		name_len = strlen(header->lib_name);
		if (strncmp(name, header->lib_name, name_len) == 0)
			return lib;
	}

	return NULL;
}

/*
 * Command to open a library. Right now only accepts the following usage:
 *
 *   open <lib-path>
 *
 * TODO: expand this to enable opening of remote libraries.
 */
int __mlib_open_library(int argc, char *argv[])
{
	struct mlib_library *lib;

	if (argc != 2) {
		mlib_printf("Usage: open <lib-path>\n");
		return 1;
	}

	lib = mlib_open_library(argv[1], MLIB_LOCAL);
	if (!lib)
		return 1;

	mlib_printf("Opened library: %s\n", lib->header->lib_name);
	return 0;
}

static struct mlib_command mlib_command_open = {
	.name = "open",
	.desc = "Open a library.",
	.main = __mlib_open_library,
};

/*
 * Command to close a library. Usage:
 *
 *   close <lib-name>
 *
 */
int __mlib_close_library(int argc, char *argv[])
{
	int ret;
	struct mlib_library *lib;

	if (argc != 2) {
		mlib_printf("Usage: close <lib-name>\n");
		return 1;
	}

	lib = mlib_find_library(argv[1]);
	if (!lib) {
		mlib_printf("Library does not exist: %s\n", argv[1]);
		return 1;
	}

	ret = mlib_close_library(lib);
	if (ret) {
		mlib_printf("Failed to close %s\n", argv[1]);
		return 1;
	}

	return 0;
}

static struct mlib_command mlib_command_close = {
	.name = "close",
	.desc = "Close a library.",
	.main = __mlib_close_library,
};

/*
 * Command to display currently loaded libraries. Usage:
 *
 *   lslib
 */
int __mlib_list_libraries(int argc, char *argv[])
{
	struct list_head *elem;
	struct mlib_library *lib;
	struct mlib_library_header *header;

	mlib_printf("Loaded libraries:\n");

	list_for_each(elem, &library_list) {
		lib = list_entry(elem, struct mlib_library, list);
		header = lib->header;

		/* Print some useful info. */
		mlib_printf("  %-12s    ", header->lib_name);
		mlib_printf("%s\n", header->media_prefix);
	}
	return 0;
}

static struct mlib_command mlib_command_lslib = {
	.name = "lslib",
	.desc = "List loaded libraries.",
	.main = __mlib_list_libraries,
};

/*
 * Command to create a library. Usage:
 *
 *   create <path> <name> <media_prefix>
 */
int __mlib_create_library(int argc, char *argv[])
{
	int ret;

	if (argc != 4) {
		mlib_printf("argc = %d\n", argc);
		mlib_printf("Usage: create <path> <name> <media_prefix>\n");
		return -1;
	}

	ret = mlib_create_library(argv[1], argv[2], argv[3]);
	if (ret)
		return -1;

	mlib_printf("Made library: %s\n", argv[1]);
	mlib_printf("        Name: %s\n", argv[2]);
	mlib_printf("Media prefix: %s\n", argv[3]);
	return 0;
}

static struct mlib_command mlib_command_create = {
	.name = "create",
	.desc = "Create a library.",
	.main = __mlib_create_library,
};

int mlib_library_init()
{
	mlib_command_register(&mlib_command_open);
	mlib_command_register(&mlib_command_close);
	mlib_command_register(&mlib_command_create);
	mlib_command_register(&mlib_command_lslib);
	return 0;
}
