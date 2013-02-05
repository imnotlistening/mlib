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

#include <mlib/mlib.h>

#include <regress.h>

int regress_verify_create(struct mlib_library *lib, void *priv)
{
	return 0;
}

int regress_verify_open(struct mlib_library *lib, void *priv)
{
	return 0;
}

int regress_verify_mk_rm_pls(struct mlib_library *lib, void *priv)
{
	return 0;
}
