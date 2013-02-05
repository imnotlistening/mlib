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
 * Library related definitions.
 */

#ifndef _MLIB_LIBRARY_H_
#define _MLIB_LIBRARY_H_

#include <stdint.h>

/*
 * Macros to read and write to the library itself. These handle endianness
 * issues that arise from different architectures directly accessing the target
 * library. We store libraries as big endian data.
 */
#define __mlib_readl(addr)       be32toh(*addr)
#define __mlib_writel(addr, val) ((*addr) = htobe32(val))

#include <mlib/plist_bucket.h>

/*
 * Library magic and types.
 */
#define MLIB_MAGIC			0x4d4c4942	/* MLIB */
#define MLIB_HEADER_SIZE		(1<<10)		/* 1 Kb */
#define MLIB_HEADER_FIELD_COUNT		3	/* # of 32 bit fields */
#define MLIB_LIBRARY_LIB_NAME_LEN	(128 - (4 * MLIB_HEADER_FIELD_COUNT))
#define MLIB_LIBRARY_MEDIA_PREFIX_LEN	(MLIB_HEADER_SIZE - 128)

/*
 * Header for a library. This struct is exactly 1 KByte.
 */
struct mlib_library_header {
	/*
	 * The MLib magic; a quick and dirty way to check if a file is likely
	 * a MLib library or not.
	 */
	uint32_t	mlib_magic;

	/*
	 * Number of items in the library.
	 */
	uint32_t	media_count;

	/*
	 * Size of the library. Limited to 4GB. Since this is not the actual
	 * media itself, this doesn't seem too unreasonable.
	 */
	uint32_t	lib_len;

	/*
	 * The name of the library.
	 */
	char		lib_name[MLIB_LIBRARY_LIB_NAME_LEN];

	/*
	 * For remote libraries, this is available for specifying a URL prefix
	 * for accessing the paths in the library. An example:
	 *
	 *   http://my.mlib.com/mlib
	 *
	 * When media is actually accessed it will have the URL prefixed to the
	 * path defined in the library.
	 */
	char		media_prefix[MLIB_LIBRARY_MEDIA_PREFIX_LEN];
} __attribute__((packed));

/*
 * A list node for keeping track of all open libraries.
 */
struct mlib_library {
	struct list_head	 	 list;
	struct mlib_library_header	*header;
	int				 fd;
};

/* TODO: Byte level endianness handlers? */
#define MLIB_LIB_MAGIC(lib)	__mlib_readl(&(lib)->header->mlib_magic)
#define MLIB_LIB_LEN(lib)	__mlib_readl(&(lib)->header->lib_len)
#define MLIB_LIB_NAME(lib)	((lib)->header->lib_name)
#define MLIB_LIB_PREFIX(lib)	((lib)->header->media_prefix)

#define MLIB_LIB_SET_MAGIC(lib, val)		\
	__mlib_writel(&(lib)->header->lib_len, val)
#define MLIB_LIB_SET_LEN(lib, val)		\
	__mlib_writel(&(lib)->header->lib_len, val)

#define MLIB_PLIST_HDR_MAGIC	0x10202010
#define MLIB_PLIST_FIELDS	3
#define MLIB_PLIST_NAME_LEN	(128 - (MLIB_PLIST_FIELDS * sizeof(uint32_t)))

/*
 * Playlist structure. The header is 128 bytes long: 3 uint32_t fields and
 * 116 bytes of name data. The hashtable is of variable size.
 */
struct mlib_playlist {
	uint32_t	playlist_magic;	/* Mark the start of a playlist. */
	uint32_t	length;		/* Length of playlist in bytes. */
	uint32_t	mcount;		/* Number of elements in playlist. */
	char		name[MLIB_PLIST_NAME_LEN];
	struct mlib_bucket	data;
} __attribute__((packed));

#define MLIB_PLIST_MAGIC(plist)		__mlib_readl(&(plist)->playlist_magic)
#define MLIB_PLIST_LEN(plist)		__mlib_readl(&(plist)->length)
#define MLIB_PLIST_MCOUNT(plist)	__mlib_readl(&(plist)->mcount)
#define MLIB_PLIST_NAME(plist)		((plist)->name)

#define MLIB_PLIST_SET_MAGIC(plist, val)		\
	__mlib_writel(&(plist)->playlist_magic, val)
#define MLIB_PLIST_SET_LEN(plist, val)			\
	__mlib_writel(&(plist)->length, val)
#define MLIB_PLIST_SET_MCOUNT(plist, val)		\
	__mlib_writel(&(plist)->mcount, val)

/**
 * Reads through all of the playlists in a library.
 *
 * @lib		A pointer to the library.
 * @plist	A pointer variable to use to hold the playlist pointer.
 */
#define mlib_for_each_pls(lib, plist)			\
	for (plist = mlib_next_playlist(lib, NULL);	\
	     plist != NULL;				\
	     plist = mlib_next_playlist(lib, plist))	\

/**
 * Reads through all of the playlists in a library.
 *
 * @lib		A pointer to the library.
 * @plist	A pointer variable to use to hold the playlist pointer.
 */
#define mlib_for_each_path(plist, ind, path)			\
	for (ind = 0, path = mlib_get_path_at(plist, ind);	\
	     path != NULL;					\
	     path = mlib_get_path_at(plist, ++ind))

/*
 * MLib library functions for general use.
 */
int	 mlib_library_init();
int	 mlib_create_library(const char *path, const char *name,
			     const char *media_prefix);
struct mlib_library	*mlib_open_library(const char *location, int remote);
struct mlib_library	*mlib_find_library(const char *name);
int	 mlib_sync_library(const struct mlib_library *lib);
int	 mlib_close_library(struct mlib_library *lib);

/*
 * Playlist functions for general use.
 */
int	 mlib_playlist_init();
int	 mlib_start_playlist(struct mlib_library *lib, const char *name);
struct mlib_playlist	 *mlib_next_playlist(const struct mlib_library *lib,
					     struct mlib_playlist *plist);
struct mlib_playlist	 *mlib_find_playlist(const struct mlib_library *lib,
					     const char *name);
int	 mlib_add_path_to_plist(struct mlib_library *lib,
				struct mlib_playlist *plist, const char *path);
int	 mlib_add_path(struct mlib_library *lib, const char *plist,
		       const char *path);
const char	*mlib_get_path_at(const struct mlib_playlist *plist,
				  int index);

/*
 * Highly specialized functions not for external use.
 */
int	 __mlib_library_expand(struct mlib_library *lib, size_t len);
int	 __mlib_library_trunc(struct mlib_library *lib, size_t len);
int 	 __mlib_library_excise(struct mlib_library *lib, void *start,
			       void *end);
int	 __mlib_library_insert_space(struct mlib_library *lib, uint32_t offset,
				     uint32_t length);

#endif
