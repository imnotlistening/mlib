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
 * Core program for MLib. Loads modules and parses user input.
 *
 * TODO: write a description of the MLib architecture.
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <mlib/mlib.h>
#include <mlib/mlib_shell.h>

int mlib_parse_options(int argc, char *argv[]);
int mlib_loop();

extern int yylex();

int main(int argc, char *argv[])
{
	int ret;

	/*
	 * Parse MLib's options. 
	 */
	/* mlib_parse_options(argc, argv); */

	/* Init functions. */
	mlib_init();
	mlib_init_io();

	if (argv[1]) {
		ret = mlib_push_stream(argv[1]);
		if (!ret)
			mlib_printf("Executing: %s\n", argv[1]);
	}

	mlib_loop();

	return 0;
}

int mlib_parse_options(int argc, char *argv[])
{
	return 0;
}

/*
 * Write out history file when we get killed.
 */
int sigint_handler(int sig)
{
	mlib_exit(1);
	return 0;
}
