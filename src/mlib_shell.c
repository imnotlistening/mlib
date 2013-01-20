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
 * The MLib shell.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mlib/mlib.h>
#include <mlib/mlib_shell.h>

/*
 * The main user input handling loop.
 */
int mlib_loop()
{
	int i;
	int argc;
	char **argv;
	struct mlib_command *cmd;

	while (mlib_read_line(&argc, &argv) != -1) {
		cmd = mlib_find_command(argv[0]);
		if (!cmd) {
			mlib_printf("%s: command not found.\n", argv[0]);
			goto done;
		}

		cmd->main(argc, argv);

	done:
		/* Free argv. */
		for (i = 0; i < argc; i++)
			free(argv[i]);
		free(argv);
	}

	return 0;
}
