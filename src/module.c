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
 * Load modules.
 */

#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>

#include <mlib/mlib.h>
#include <mlib/module.h>

static LIST_HEAD(mlib_modules);

/**
 * Load the module in the passed string. A 0 is returned if successful, other
 * wise < 0 will be returned.
 *
 * @path	Path to the module. Will be resolved based on shared library
 * 		loadign rules.
 */
int mlib_load_module(const char *path)
{
	return 0;
}
