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
 * Definitions and what not for MLib. 
 */

#ifndef _MLIB_H_
#define _MLIB_H_

#include <stdlib.h>

#include <mlib/list.h>
#include <mlib/library.h>

/*
 * Describe a command so that we can later execute it.
 */
struct mlib_command {
	const char		*name;
	const char		*desc;

	/*
	 * Main entrance function for the command.
	 */
	int			(*main)(int argc, char *argv[]);

	struct list_head	 list;
};

/*
 * This is for queueing output to the console.
 */
struct mlib_msg {
	struct list_head	 queue;
	char			*text;
};

/*
 * Core functionality.
 */
int	 mlib_init();

/*
 * Command related functions.
 */
int	 mlib_register_command(struct mlib_command *command);
struct mlib_command	*mlib_find_command(const char *name);
int	 mlib_register_builtins();

/*
 * Print macros and functions. This aims to keep I/O to the console nice and
 * coordinated.
 */
int	 mlib_printf(const char *fmt, ...)
	__attribute__((format(printf, 1, 2)));
int	 mlib_lib_printf(const char *fmt, ...)
	__attribute__((format(printf, 1, 2)));

#define mlib_perror(FMT, ...)					\
	do {							\
		mlib_printf(FMT ": %s\n", strerror(errno));	\
	} while (0)

#endif
