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
 * This is for the MLib shell program.
 */

#ifndef _MLIB_SHELL_H_
#define _MLIB_SHELL_H_

#include <mlib/mlib.h>

#ifndef MLIB_PROMPT
#  define MLIB_PROMPT "mlib> "
#endif

#define MLIB_LEX_DONE		0
#define MLIB_LEX_STRING		1
#define MLIB_LEX_EOL		2
#define MLIB_LEX_ERROR		-1

/*
 * Terminal and I/O functions.
 */
int	 mlib_init_io();
size_t	 mlib_readbuf(char *buffer, size_t max);
int	 mlib_read_line(int *argc, char ***argv);
int	 mlib_push_stream(const char *stream);

extern char *mlib_text;

#endif
