/* (C) Copyright 2013, Alex Waterman <imNotListening@gmail.com>
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
 * Basic regression tests.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <mlib/mlib.h>

#include <regress.h>

int regress_verify_create(struct mlib_library *lib, void *priv)
{
	char *name = priv;

	return mlib_create_library(name, "test-lib", "./");
}

int regress_verify_open(struct mlib_library *lib, void *priv)
{
	int ret = 0;
	char *name = priv;
	struct mlib_library *test_lib;

	test_lib = mlib_open_library(name, 0);
	if (!test_lib) {
		mlib_printf("Failed to open: %s\n", name);
		ret = -1;
		goto done;
	}
	if (mlib_close_library(test_lib)) {
		mlib_printf("Failed to close library: %s\n", name);
		ret = -1;
	}
	
done:
	unlink(name);
	return ret;
}

int regress_verify_mk_rm_pls(struct mlib_library *lib, void *priv)
{
	if (mlib_start_playlist(lib, "test-pls"))
		return -1;
	if (!mlib_find_playlist(lib, "test-pls"))
		return -1;
	if (mlib_delete_playlist(lib, "test-pls"))
		return -1;
	if (mlib_find_playlist(lib, "test-pls"))
		return -1;

	return 0;
}

int regress_verify_add_to_plist(struct mlib_library *lib, void *priv)
{
	struct mlib_playlist *pls;

	if (mlib_add_path(lib, ".global", "a/test/path"))
		return -1;

	pls = mlib_find_playlist(lib, ".global");
	if (!pls) {
		mlib_error("Failed to find .global playlist\n");
		return -1;
	}

	if (!mlib_find_path(pls, "a/test/path"))
		return -1;
	return 0;
}
