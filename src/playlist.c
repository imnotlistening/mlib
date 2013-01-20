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
 * Manage playlists in a library.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <mlib/mlib.h>

/*
 * Return a pointer to the end of the passed library. This is where we add new
 * playlists. Note: this function does not allocate space for the platlist. If
 * any access is made to this playlist before space is allocated for it, a
 * SIGBUS will be generated.
 */
struct mlib_playlist *__mlib_new_playlist_addr(struct mlib_library *lib)
{
	void *plist_addr = ((void *)lib->header) + MLIB_LIB_LEN(lib);
	return plist_addr;
}

static inline int __mlib_plist_check_len(const struct mlib_library *lib,
					 struct mlib_playlist *plist)
{
	return (unsigned long)plist >=
		((unsigned long)lib->header) + MLIB_LIB_LEN(lib);
}

/**
 * Returns the play list after the passed play list. If the passed play list is
 * NULL, then the first playlist is returned. This returns NULL if there are
 * no more playlists after the passed playlist or if there is an error.
 *
 * @lib		The library to use.
 * @plist	The current playlist.
 */
struct mlib_playlist *mlib_next_playlist(const struct mlib_library *lib,
					 struct mlib_playlist *plist)
{
	struct mlib_playlist *tmp_plist;

	if (plist == NULL)
		tmp_plist = ((void *)lib->header) + MLIB_HEADER_SIZE;
	else
		tmp_plist = plist;

	/*
	 * Handle the start case: use the first play list in the library.
	 */
	if (!plist) {
		if (__mlib_plist_check_len(lib, tmp_plist))
			return NULL;
		if (MLIB_PLIST_MAGIC(tmp_plist) != MLIB_PLIST_HDR_MAGIC) {
			mlib_printf("Library corruption detected.\n");
			mlib_printf("Invalid playist header magic found.\n");
			return NULL;
		}
		return tmp_plist;
	}

	/*
	 * Otherwise go to the next playlist. Add the length of the current
	 * playlist to the playlist address: that's where the next playlist
	 * should be.
	 */
	tmp_plist = ((void *)tmp_plist) + MLIB_PLIST_LEN(plist);
	if (__mlib_plist_check_len(lib, tmp_plist))
		return NULL;
	if (MLIB_PLIST_MAGIC(tmp_plist) != MLIB_PLIST_HDR_MAGIC) {
		mlib_printf("Library corruption detected.\n");
		mlib_printf("Invalid playist header magic found.\n");
		return NULL;
	}

	return tmp_plist;
}

/**
 * Create an empty playlist in the passed library.
 *
 * @lib		The library to add the playlist to.
 * @name	A name for the playlist.
 */
int mlib_start_playlist(struct mlib_library *lib, const char *name)
{
	uint32_t newlen;
	struct mlib_playlist *plist;

	if (strlen(name) >= (MLIB_PLIST_NAME_LEN - 1))
		mlib_printf("warning: truncating playlist name.\n");

	plist = __mlib_new_playlist_addr(lib);

	/* Allocate room for the playlist. */
	newlen = MLIB_LIB_LEN(lib) + sizeof(struct mlib_playlist);
	if (__mlib_library_expand(lib, newlen) < 0)
		return -1;

	memset(plist->name, 0, MLIB_PLIST_NAME_LEN);
	strncpy(plist->name, name, MLIB_PLIST_NAME_LEN - 1);
	MLIB_PLIST_SET_MAGIC(plist, MLIB_PLIST_HDR_MAGIC);
	MLIB_PLIST_SET_LEN(plist, sizeof(struct mlib_playlist));
	MLIB_PLIST_SET_MCOUNT(plist, 0);

	return mlib_sync_library(lib);
}

/*
 * Create an empty playlist. Very simple low level command here. Usage:
 *
 *   makepls <lib> <name>
 */
int __mlib_make_playlist(int argc, char *argv[])
{
	int ret;
	struct mlib_library *lib;

	if (argc != 3) {
		mlib_printf("Usage: makepls <lib> <name>\n");
		return 1;
	}

	lib = mlib_find_library(argv[1]);
	if (!lib) {
		mlib_printf("Library '%s' not loaded.\n", argv[1]);
		return 1;
	}

	ret = mlib_start_playlist(lib, argv[2]);
	if (ret) {
		mlib_printf("Failed to make empty playlist.\n");
		return 1;
	}

	return 0;
}

static struct mlib_command mlib_command_makepls = {
	.name = "makepls",
	.desc = "Create an empty playlist.",
	.main = __mlib_make_playlist,
};

/*
 * List play lists in a library: usage:
 *
 *   lspls <lib>
 */
int __mlib_ls_playlist(int argc, char *argv[])
{
	struct mlib_library *lib;
	struct mlib_playlist *plist;

	if (argc != 2) {
		mlib_printf("Usage: lspls <lib>\n");
		return 1;
	}

	lib = mlib_find_library(argv[1]);
	if (!lib) {
		mlib_printf("Library '%s' not loaded.\n", argv[1]);
		return 1;
	}

	/*
	mlib_for_each_pls(lib, plist)
		mlib_printf("%5d  - %s\n", plist->mcount, plist->name);
	*/

	plist = NULL;
	while ((plist = mlib_next_playlist(lib, plist)) != NULL) {
		mlib_printf("%5d  - %s\n", plist->mcount, plist->name);
	}

	return 0;
}

static struct mlib_command mlib_command_lspls = {
	.name = "lspls",
	.desc = "List playlists in a library.",
	.main = __mlib_ls_playlist,
};

int mlib_playlist_init()
{
	mlib_command_register(&mlib_command_makepls);
	mlib_command_register(&mlib_command_lspls);
	return 0;
}
