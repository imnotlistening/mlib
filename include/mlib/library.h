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
 * A path to a media file.
 */
struct mlib_path {
	/*
	 * Length of the path.
	 */
	uint32_t	length;

	/*
	 * The path itself.
	 */
	char		path[];
};

/*
 * Library magic and types.
 */
#define MLIB_MAGIC			0x42494c4d	/* MLIB */
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
 * MLib library functions.
 */
int	 mlib_library_init();
int	 mlib_load_library(const char *url);

#endif
