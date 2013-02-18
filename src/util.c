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
 * Utility functions.
 */

#include <errno.h>
#include <stdio.h>

#include <mlib/mlib.h>

/**
 * Parses a list with a delimiter of @delimiter.  This will make a new chunk of
 * memory for the list and return an array of strings. Make sure to free the
 * returned list with mlib_free_parsed_list(). The list will be NULL terminated;
 * if @len is not NULL, then the length of the list (minus the NULL terminator)
 * will be placed in the location pointed to by @len.
 *
 * @list	List to parse.
 * @delimiter	The delimiting character to use.
 * @len		The computed length is placed in the referenced mem if non
 *		NULL.
 * @return	Returns a parsed list of strings.
 */
char **mlib_parse_list(const char *list, char delimiter, int *len)
{
	int count = 0, i;
	char *list_mem;
	char **list_ptrs;

	/* We wont edit list mem yet :). */
	list_mem = (char *)list;
	while (*list_mem) {
		/*
		 * Weed out delimimiters that are adjecent. Report it as only
		 * one delimiter. The final check is to avoid over stepping the
		 * passed string.
		 */
		if (*list_mem == delimiter &&
		    *(list_mem + 1) != delimiter &&
		    *(list_mem + 1) != 0) {
			count++;
		}
		list_mem++;
	}

	/*
	 * Empty string or string with only delimiters: don't parse it.
	 */
	if (!count)
		return NULL;

	/* Now we get the real list mem. */
	list_mem = strdup(list);
	list_ptrs = malloc(sizeof(char *) * (count + 1));
	if (!list_mem || !list_ptrs) {
		mlib_perror("No mem");
 		return NULL;
	}

	/* Set delimters to 0 to act as NULL terminators for the strings. */
	for (i = 0; list_mem[i]; i++) {
		if (list_mem[i] == delimiter)
			list_mem[i] = 0;
	}

	for (i = 0; i < count; i++) {
		list_ptrs[i] = list_mem;
		/* Move list mem to the next string. */
		while (*list_mem++)
			;
	}

	if (*len)
		*len = count - 1;
	return list_ptrs;
}

/**
 * Free a list created by mlib_parse_list().
 *
 * @list	The list to free.
 */
void mlib_free_parsed_list(char **list)
{
	free(list[0]);
	free(list);
}

/**
 * Compute the offset of @ptr into the library specified by @lib. The offset
 * is relative the the base of the memory mapped file area, not the actual @lib
 * pointer itself. There is no error checking for @ptr. If it is an invalid
 * pointer, and invalid offset will be returned.
 *
 * @lib		The library to use.
 * @ptr		A pointer into the library.
 * @return	Returns the unsigned 32 bit difference.
 */
uint32_t mlib_lib_offset(struct mlib_library *lib, void *ptr)
{
	return (uint32_t)(ptr - (void *)lib->header);
}

/**
 * Return non-zero if @file's file extension matchs one of the passed filter
 * types. @mtypes must be NULL terminated.
 *
 * @file	The file name to match with.
 * @mtypes	A list of extensions to match against - e.g 'mkv'.
 */
int mlib_filter(const char *file, char *mtypes[])
{
	char *filt;
	int file_len, filt_len;

	file_len = strlen(file);
	filt = *mtypes;
	while (filt) {
		filt_len = strlen(filt);
		if (!strcmp(filt, file + file_len - filt_len))
			return 1;

		filt = *mtypes++;
	}

	return 0;
}
