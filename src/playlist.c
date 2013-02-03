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
			mlib_error("Library corruption detected.\n");
			mlib_error("Invalid playist header magic found.\n");
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
		mlib_error("Library corruption detected.\n");
		mlib_error("Invalid playist header magic found.\n");
		return NULL;
	}

	return tmp_plist;
}

/**
 * Read through the paths in a play list. if @path is NULL, then start with
 * the first path. Returns the path after @path or NULL if there are no more
 * paths in the playlist.
 *
 * @plist	The playlist to iterate through.
 * @path	The index path.
 */
const char *mlib_get_path_at(const struct mlib_playlist *plist, int index)
{
	if (index >= mlib_bucket_nr_indexes(&plist->data))
		return NULL;

	return mlib_bucket_string(&plist->data, index);
}

/**
 * Find a playlist in the specified library. If a playlist is found, a pointer
 * to that playlist is returned. This playlist must not be directly accessed.
 * If no playlist is found, then returns NULL.
 *
 * @lib		Library to search through.
 * @name	Name of the playlist to look for,
 */
struct mlib_playlist *mlib_find_playlist(const struct mlib_library *lib,
					 const char *name)
{
	struct mlib_playlist *plist;

	mlib_for_each_pls(lib, plist) {
		if (strncmp(MLIB_PLIST_NAME(plist), name,
			    MLIB_PLIST_NAME_LEN) == 0)
			return plist;
	}
	return NULL;
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

	if (mlib_find_playlist(lib, name)) {
		mlib_user_error("playlist '%s' already exists.\n", name);
		return -1;
	}

	if (strlen(name) >= (MLIB_PLIST_NAME_LEN - 1))
		mlib_printf("warning: truncating playlist name.\n");

	plist = __mlib_new_playlist_addr(lib);

	/* Allocate room for the playlist. */
	newlen = MLIB_LIB_LEN(lib) + sizeof(struct mlib_playlist) +
		MLIB_BUCKET_GROWTH_RATE;
	if (__mlib_library_expand(lib, newlen) < 0)
		return -1;

	memset(plist->name, 0, MLIB_PLIST_NAME_LEN);
	strncpy(plist->name, name, MLIB_PLIST_NAME_LEN - 1);
	MLIB_PLIST_SET_MAGIC(plist, MLIB_PLIST_HDR_MAGIC);
	MLIB_PLIST_SET_LEN(plist, sizeof(struct mlib_playlist) +
			   MLIB_BUCKET_GROWTH_RATE);
	MLIB_PLIST_SET_MCOUNT(plist, 0);

	mlib_init_bucket(&plist->data, MLIB_BUCKET_GROWTH_RATE);

	return mlib_sync_library(lib);
}

/**
 * Delete a playlist from the passed library. Returns 0 on succes, -1 on error.
 *
 * @lib		The library to use.
 * @name	Name of the playlist.
 */
int mlib_delete_playlist(struct mlib_library *lib, const char *name)
{
	void *start, *end;
	uint32_t plist_len;
	struct mlib_playlist *plist;

	plist = mlib_find_playlist(lib, name);
	if (!plist) {
		mlib_user_error("Playlist '%s' does not exist.\n", name);
		return -1;
	}

	plist_len = MLIB_PLIST_LEN(plist);
	start = plist;
	end = start + plist_len;

	if (__mlib_library_excise(lib, start, end)) {
		mlib_error("Failed to truncate '%s'\n", MLIB_LIB_NAME(lib));
		return -1;
	}
	return 0;
}

/**
 * Add the passed path to a playlist. If an error occurs, < 0 is returned,
 * otherwise 0 is returned.
 *
 * @lib		Library to find the playlist in.
 * @plist	A pointer to the playlist itself.
 * @path	A path to add.
 */
int mlib_add_path_to_plist(struct mlib_library *lib,
			   struct mlib_playlist *plist, const char *path)
{
	if  (mlib_bucket_add(lib, &plist->data, path))
		return -1;

	MLIB_PLIST_SET_MCOUNT(plist, MLIB_PLIST_MCOUNT(plist) + 1);
	return 0;
}

/**
 * Add the passed path to a playlist. If the playlist does not exist or an
 * error occurs, < 0 is returned, otherwise 0 is returned. This will
 * automatically add the path to '.global' as well if it has not yet been
 * added.
 *
 * @lib		Library to find the playlist in.
 * @plist	Name of the playlist.
 * @path	A path to add.
 */
int mlib_add_path(struct mlib_library *lib, const char *plist,
		  const char *path)
{
	struct mlib_playlist *real_plist;
	struct mlib_playlist *global_plist;

	if (strcmp(plist, ".global")) {
		global_plist = mlib_find_playlist(lib, ".global");
		if (!global_plist) {
			mlib_error("Library corruption: "
				   ".global plist not found.\n");
			return -1;
		}
		mlib_add_path_to_plist(lib, global_plist, path);
	}

	real_plist = mlib_find_playlist(lib, plist);
	if (!real_plist) {
		mlib_user_error("Playlist '%s' not found.\n", plist);
		return -1;
	}
	return mlib_add_path_to_plist(lib, real_plist, path);
}

/*
 * Internel version of mlib_find_path() that doesn't return a const.
 */
char *__mlib_find_path(const struct mlib_playlist *plist, const char *path)
{
	return (char *)mlib_bucket_contains(&plist->data, path);
}

/**
 * Search for the element in @path in the passed library and playlist. Returns
 * a pointer to the location of that string in the library or NULL if not
 * found.
 *
 * @lib		The library.
 * @plist	The playlist in @lib.
 * @path	The path to look for.
 */
const char *mlib_find_path(const struct mlib_playlist *plist, const char *path)
{
	return __mlib_find_path(plist, path);
}

/*
 * Create an empty playlist. Very simple low level command here. Usage:
 *
 *   mkpls <lib> <name>
 */
int __mlib_make_playlist(int argc, char *argv[])
{
	int ret;
	struct mlib_library *lib;

	if (argc != 3) {
		mlib_printf("Usage: mkpls <lib> <name>\n");
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

static struct mlib_command mlib_command_mkpls = {
	.name = "mkpls",
	.desc = "Create an empty playlist.",
	.main = __mlib_make_playlist,
};

/*
 * List play lists in a library or paths in a playlist. Usage:
 *
 *   lspls <lib> [playlist]
 */
int __mlib_ls_playlist(int argc, char *argv[])
{
	int ind;
	const char *path;
	struct mlib_library *lib;
	struct mlib_playlist *plist;

	if (argc < 2 || argc > 4) {
		mlib_printf("Usage: lspls <lib> [playlist]\n");
		return 1;
	}

	lib = mlib_find_library(argv[1]);
	if (!lib) {
		mlib_printf("Library '%s' not loaded.\n", argv[1]);
		return 1;
	}

	if (argc == 2) {
		mlib_for_each_pls(lib, plist)
			mlib_printf("%5d  - %s\n", MLIB_PLIST_MCOUNT(plist),
				    MLIB_PLIST_NAME(plist));
	} else {
		plist = mlib_find_playlist(lib, argv[2]);
		if (!plist) {
			mlib_printf("Playlist '%s' does not exist.\n",
				    argv[2]);
			return 1;
		}
		mlib_for_each_path(plist, ind, path)
			mlib_printf("%s\n", path);
	}

	return 0;
}

static struct mlib_command mlib_command_lspls = {
	.name = "lspls",
	.desc = "List playlists in a library.",
	.main = __mlib_ls_playlist,
};

/*
 * Add a path to the specified playlist. Usage:
 *
 *   plsadd <lib> <playlist> <path>
 */
int __mlib_playlist_add(int argc, char *argv[])
{
	int ret;
	struct mlib_library *lib;

	if (argc != 4) {
		mlib_printf("Usage: plsadd <lib> <plist> <path>\n");
		return 1;
	}

	lib = mlib_find_library(argv[1]);
	if (!lib) {
		mlib_printf("Library '%s' not loaded.\n", argv[1]);
		return 1;
	}

	ret = mlib_add_path(lib, argv[2], argv[3]);
	if (ret)
		return 1;
	return 0;
}

static struct mlib_command mlib_command_plsadd = {
	.name = "plsadd",
	.desc = "Add a path to a playlist.",
	.main = __mlib_playlist_add,
};

/*
 * Remove a playlist. Usage:
 *
 *   rmpls <lib> <pls>
 */
int __mlib_rm_playlist(int argc, char *argv[])
{
	struct mlib_library *lib;

	if (argc != 3) {
		mlib_printf("Usage: rmpls <lib> <plist>\n");
		return 1;
	}

	lib = mlib_find_library(argv[1]);
	if (!lib) {
		mlib_printf("Library '%s' not loaded.\n", argv[1]);
		return 1;
	}

	if (!mlib_delete_playlist(lib, argv[2]))
	    return 1;
	return 0;
}

static struct mlib_command mlib_command_rmpls = {
	.name = "rmpls",
	.desc = "Remove a playlist.",
	.main = __mlib_rm_playlist,
};

int mlib_playlist_init()
{
	mlib_command_register(&mlib_command_mkpls);
	mlib_command_register(&mlib_command_rmpls);
	mlib_command_register(&mlib_command_lspls);
	mlib_command_register(&mlib_command_plsadd);
	return 0;
}
