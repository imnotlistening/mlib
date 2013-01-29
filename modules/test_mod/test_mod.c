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
 * Simple test module.
 */

#include <stdio.h>

#include <mlib/mlib.h>

int test_mod_init(void)
{
	mlib_printf("Yo, I'm a test module.\n");
	return 0;
}

int test_mod_fini(void)
{
	mlib_printf("Good bye from a test module.\n");
	return 0;
}

MLIB_MODULE("test-mod", test_mod_init, test_mod_fini,
	    "A simple test module.");
