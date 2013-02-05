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
 * Special data structure to allow us to access a packet array of arbitrary
 * size strings quickly. The data structure looks kinda like this:
 *
 *   +----------------------------+------ ~~~ ---+----------------------------+
 *   | Strings, null terminated   | Excess space |   uint32_t indexes to strs |
 *   +----------------------------+------ ~~~ ---+----------------------------+
 *
 * The indexes are alphabetized for a fast binary search to see if a string is
 * contained in the data structure.
 *
 * This code makes certain assumptions about the data structures - e.g all
 * mlib_buckets are embedded in a playlist structure. Therefor, *do not* use
 * this code directly unless you know what you are doing.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include <mlib/mlib.h>
#include <mlib/list.h>
#include <mlib/plist_bucket.h>

/* #define __DEBUG_BUCKETS */

/*
 * Sets up a new hashtable in the passed hash data structure. @size is the
 * maximum number of bytes that the bucket may live in. This memory must have
 * already been set up correctly via the correct library functions (i.e
 * mlib_lbrary_expand()).
 */
int mlib_init_bucket(struct mlib_bucket *bucket, uint32_t size)
{
	if (size < sizeof(struct mlib_bucket)) {
		mlib_error("Bucket size too small.\n");
		return -1;
	}

	MLIB_BUCKET_SET_MAGIC(bucket, MLIB_BUCKET_MAGIC_VAL);
	MLIB_BUCKET_SET_LENGTH(bucket, size);
	MLIB_BUCKET_SET_INDEX_OFFS(bucket, size);
	MLIB_BUCKET_SET_STR_BYTES(bucket, 0);
	return 0;
}

/*
 * Return uint32_t pointer to the index array.
 */
uint32_t *mlib_bucket_indexes(const struct mlib_bucket *bucket)
{
	return ((void *)bucket) + MLIB_BUCKET_INDEX_OFFS(bucket);
}

/*
 * Compute the number of indexes in a bucket.
 */
int mlib_bucket_nr_indexes(const struct mlib_bucket *bucket)
{
	return (MLIB_BUCKET_LENGTH(bucket) - MLIB_BUCKET_INDEX_OFFS(bucket)) /
		sizeof(uint32_t);
}

/*
 * Return index value at particular offset.
 */
uint32_t mlib_bucket_index(const struct mlib_bucket *bucket, int i)
{
	uint32_t *indexes = mlib_bucket_indexes(bucket);
	return __mlib_readl(&indexes[i]);
}

/*
 * Return the N'th string in the bucket. N'th refers to the index offset; that
 * is we first compute index N then use that to find where the string is and
 * return that string.
 */
const char *mlib_bucket_string(const struct mlib_bucket *bucket, int n)
{
	uint32_t str_offset = mlib_bucket_index(bucket, n);
	return bucket->strings + str_offset;
}

/*
 * Return the string at the request string offset.
 */
const char *mlib_bucket_string_at(const struct mlib_bucket *bucket,
					 uint32_t offset)
{
	return bucket->strings + offset;
}

/*
 * Expand the passed bucket. This expands the bucket by @length bytes.
 */
int mlib_bucket_expand(struct mlib_library *lib, struct mlib_bucket *bucket,
		       uint32_t length)
{
	uint32_t offset;
	struct mlib_playlist *plist;

	offset = (((void *)bucket) + MLIB_BUCKET_INDEX_OFFS(bucket)) -
		((void *)lib->header);
	if (__mlib_library_insert_space(lib, offset, length))
		return -1;

	MLIB_BUCKET_SET_LENGTH(bucket, MLIB_BUCKET_LENGTH(bucket) + length);
	MLIB_BUCKET_SET_INDEX_OFFS(bucket,
				   MLIB_BUCKET_INDEX_OFFS(bucket) + length);

	/* Update the playlist the bucket is embedded in. */
	plist = container_of(bucket, struct mlib_playlist, data);
	MLIB_PLIST_SET_LEN(plist, MLIB_PLIST_LEN(plist) + length);

	return 0;
}

static const struct mlib_bucket *__cmp_bucket;
static pthread_mutex_t __bucket_cmp_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Compare function for the index list. Compares the strings that two indexes
 * point to.
 */
static int __mlib_bucket_cmp_indexes(const void *a, const void *b)
{
	uint32_t ind_a, ind_b;
	const char *str_a, *str_b;

	ind_a = __mlib_readl((uint32_t *)a);
	ind_b = __mlib_readl((uint32_t *)b);

	str_a = mlib_bucket_string_at(__cmp_bucket, ind_a);
	str_b = mlib_bucket_string_at(__cmp_bucket, ind_b);

	return strcmp(str_a, str_b);
}

/*
 * Compares a string to the string that an index points to.
 */
static int __mlib_bucket_cmp_str_to_ind(const void *str, const void *a)
{
	uint32_t index;
	const char *bucket_str;

	index = __mlib_readl((uint32_t *)a);
	bucket_str = mlib_bucket_string_at(__cmp_bucket, index);

	return strcmp(str, bucket_str);
}

/*
 * Sort the bucket. Typically used after adding an element to the bucket. This
 * must be protected by a lock so that only one thread can sort at a time due
 * to the nature of qsort.
 */
void mlib_bucket_sort(struct mlib_bucket *bucket)
{
	pthread_mutex_lock(&__bucket_cmp_mutex);

	__cmp_bucket = bucket;
	qsort(mlib_bucket_indexes(bucket), mlib_bucket_nr_indexes(bucket),
	      sizeof(uint32_t), __mlib_bucket_cmp_indexes);

	pthread_mutex_unlock(&__bucket_cmp_mutex);
}

/*
 * Check if a path is in @bucket.
 *
 * @bucket	The bucket to search.
 * @path	Path to search for.
 */
const char *mlib_bucket_contains(const struct mlib_bucket *bucket,
				 const char *str)
{
	const char *elem = NULL;

	pthread_mutex_lock(&__bucket_cmp_mutex);
	__cmp_bucket = bucket;
	elem = bsearch(str, mlib_bucket_indexes(bucket),
		       mlib_bucket_nr_indexes(bucket), sizeof(uint32_t),
		       __mlib_bucket_cmp_str_to_ind);
	pthread_mutex_unlock(&__bucket_cmp_mutex);

	return elem;
}

/*
 * Insert an element into the bucket.
 */
int mlib_bucket_add(struct mlib_library *lib, struct mlib_bucket *bucket,
		    const char *str)
{
	uint32_t len = strlen(str) + 1;
	void *end_of_strs, *start_of_indexes;
	char *str_dest;
	uint32_t *indexes;

	/* Don't add duplicates. */
	if (mlib_bucket_contains(bucket, str))
		return -1;

	/* Ensure that we have enough space. */
restart:
	end_of_strs = bucket->strings + MLIB_BUCKET_STR_BYTES(bucket);
	start_of_indexes = ((void *)bucket) + MLIB_BUCKET_INDEX_OFFS(bucket);

	if ((end_of_strs + len) >= (start_of_indexes - 4)) {
		if (mlib_bucket_expand(lib, bucket, MLIB_BUCKET_GROWTH_RATE))
			return -1;
		goto restart;
	}

	/* We have enough space. */
	str_dest = end_of_strs;
	strcpy(str_dest, str);

	indexes = (uint32_t *)(start_of_indexes - 4);
	__mlib_writel(indexes, (uint32_t)(end_of_strs -
					  (void *)bucket->strings));

	MLIB_BUCKET_SET_INDEX_OFFS(bucket,
				   MLIB_BUCKET_INDEX_OFFS(bucket) - 4);
	MLIB_BUCKET_SET_STR_BYTES(bucket,
				  MLIB_BUCKET_STR_BYTES(bucket) + len);

	mlib_bucket_sort(bucket);
	return 0;
}

/*
 * Code to debug the bucket implementation. Only necessary for debugging
 * purposes.
 */
#ifdef __DEBUG_BUCKETS
void __mlib_print_bucket_info(struct mlib_bucket *bucket)
{
	int nr_indexes, index, i;
	const char *str;

	mlib_printf("Bucket info:\n");
	mlib_printf("  Length:     %u\n", MLIB_BUCKET_LENGTH(bucket));
	mlib_printf("  Index offs: %u\n", MLIB_BUCKET_INDEX_OFFS(bucket));
	mlib_printf("  Str bytes:  %u\n", MLIB_BUCKET_STR_BYTES(bucket));
	mlib_printf("  Free space: %u\n",
		    (uint32_t)MLIB_BUCKET_INDEX_OFFS(bucket) -
		    (uint32_t)MLIB_BUCKET_STR_BYTES(bucket) -
		    (uint32_t)sizeof(struct mlib_bucket));

	/* Print the indexes and their strings. */
	nr_indexes = mlib_bucket_nr_indexes(bucket);
	for (i = 0; i < nr_indexes; i++) {
		index = mlib_bucket_index(bucket, i);
		str = mlib_bucket_string(bucket, i);
		mlib_printf("indexes[%d] = %-4u | '%s'\n", i, index, str);
	}
}

int __mlib_bucket_dump(int argc, char *argv[])
{
	struct mlib_library *lib;
	struct mlib_playlist *plist;
	struct mlib_bucket *bucket;

	if (argc != 3) {
		mlib_printf("Usage: bucket_dump <lib> <plist>\n");
		return 1;
	}

	lib = mlib_find_library(argv[1]);
	if (!lib) {
		mlib_printf("Library '%s' not loaded.\n", argv[1]);
		return 1;
	}

	plist = mlib_find_playlist(lib, argv[2]);
	if (!plist) {
		mlib_printf("Playlist '%s' does not exist.\n", argv[2]);
		return 1;
	}

	bucket = &plist->data;
	__mlib_print_bucket_info(bucket);

	return 0;
}

static struct mlib_command mlib_bucket_dump = {
	.name = "bucket_dump",
	.desc = "Debug - dump a bucket.",
	.main = __mlib_bucket_dump,
};
#endif

int mlib_bucket_init()
{
#ifdef __DEBUG_BUCKETS
	mlib_command_register(&mlib_bucket_dump);
#endif
	return 0;
}
