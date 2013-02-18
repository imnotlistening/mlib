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
 * Hashtable implementation that is designed to be completely self contained.
 * This is so that it can be memory mapped from a file.
 *
 * This is a private header; do not include it directly.
 */

#ifndef _MLIB_PLIST_BUCKET_H_
#define _MLIB_PLIST_BUCKET_H_

#define MLIB_BUCKET_MAGIC_VAL	0x12344321
#define MLIB_BUCKET_GROWTH_RATE	128

struct mlib_library;

/*
 * Special data structure for storing strings on a disk. This can be easily
 * memory mapped into our address space and accessed directly.
 */
struct mlib_bucket {
	uint32_t	magic;
	uint32_t	length;		/* In bytes (of the entire bucket). */
	uint32_t	index_offs;	/* Offset of indexes. This array grows
					 * backwards and includes this header
					 * in the offset. */
	uint32_t	str_bytes;	/* Number of bytes in the strings
					 * array. */
	char		strings[];	/* The string data. This grows
					 * forwards. */
} __attribute__((packed));

/*
 * The usual macros for dealing with endianness.
 */
#define MLIB_BUCKET_MAGIC(bucket)	__mlib_readl(&(bucket)->magic)
#define MLIB_BUCKET_LENGTH(bucket)	__mlib_readl(&(bucket)->length)
#define MLIB_BUCKET_INDEX_OFFS(bucket)	__mlib_readl(&(bucket)->index_offs)
#define MLIB_BUCKET_STR_BYTES(bucket)	__mlib_readl(&(bucket)->str_bytes)
#define MLIB_BUCKET_SET_MAGIC(bucket, val)		\
	__mlib_writel(&(bucket)->magic, val)
#define MLIB_BUCKET_SET_LENGTH(bucket, val)		\
	__mlib_writel(&(bucket)->length, val)
#define MLIB_BUCKET_SET_INDEX_OFFS(bucket, val)		\
	__mlib_writel(&(bucket)->index_offs, val)
#define MLIB_BUCKET_SET_STR_BYTES(bucket, val)		\
	__mlib_writel(&(bucket)->str_bytes, val)

/*
 * Functions for manipulating the bucket.
 */
int	 mlib_bucket_init();
int	 mlib_init_bucket(struct mlib_bucket *bucket, uint32_t size);
int	 mlib_bucket_add(struct mlib_library *lib, struct mlib_bucket *bucket,
			 const char *str);
int	 mlib_bucket_nr_indexes(const struct mlib_bucket *bucket);
const char	*mlib_bucket_string_at(const struct mlib_bucket *bucket,
				       uint32_t offset);
const char	*mlib_bucket_string(const struct mlib_bucket *bucket, int n);
const char	*mlib_bucket_contains(const struct mlib_bucket *bucket,
				      const char *str);

#endif
