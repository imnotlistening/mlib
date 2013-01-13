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
 * Defines and maintains the list of known commands. Also includes a few simple
 * command implementations.
 */

#include <string.h>

#include <mlib/mlib.h>

static LIST_HEAD(mlib_command_list);

/**
 * Register a command with MLib.
 *
 *@command:	The command to register.
 */
int mlib_command_register(struct mlib_command *command)
{
	/* TODO: verify list does not already have this command in it. */
	list_add_tail(&command->list, &mlib_command_list);
	return 0;
}

/**
 * Find a command structure from the passed name. This will return a pointer
 * to the command struct if found, otherwise, NULL.
 *
 * @command:	Name of the command to look for.
 */
struct mlib_command *mlib_find_command(const char *name)
{
	struct list_head *elem;
	struct mlib_command *cmd;

	list_for_each(elem, &mlib_command_list) {
		cmd = list_entry(elem, struct mlib_command, list);
		if (strcmp(cmd->name, name) == 0)
			return cmd;
	}
	return NULL;
}

/*
 * Load an MLib module.
 *
 * @argc:	The arg count
 * @argv:	The argument vector
 */
int __mlib_loadm(int argc, char *argv[])
{
	return -2;
}

static struct mlib_command mlib_command_loadm = {
	.name = "loadm",
	.desc = "Load an MLib module.",
	.main = __mlib_loadm,
};

/*
 * Run an MLib script.
 *
 * @argc:	The arg count
 * @argv:	The argument vector
 */
int __mlib_run(int argc, char *argv[])
{
	return -1;
}

static struct mlib_command mlib_command_run = {
	.name = "run",
	.desc = "Run an MLib script.",
	.main = __mlib_run,
};

/*
 * Echo the arguments.
 *
 * @argc:	The arg count
 * @argv:	The argument vector
 */
int __mlib_echo(int argc, char *argv[])
{
	int i;

	for (i = 1; i < (argc - 1); i++) {
		mlib_printf("%s ", argv[i]);
	}

	if (i == (argc - 1))
		mlib_printf("%s\n", argv[i]);

	return 0;
}

static struct mlib_command mlib_command_echo = {
	.name = "echo",
	.desc = "Echo passed arguments.",
	.main = __mlib_echo,
};

/*
 * Print available commands and their descriptions.
 *
 * @argc:	The arg count
 * @argv:	The argument vector
 */
int __mlib_help(int argc, char *argv[])
{
	struct list_head *elem;
	struct mlib_command *cmd;

	list_for_each(elem, &mlib_command_list) {
		cmd = list_entry(elem, struct mlib_command, list);
		mlib_printf("%-14s %s\n", cmd->name, cmd->desc);
	}
	return 0;
}

static struct mlib_command mlib_command_help = {
	.name = "help",
	.desc = "Print available commands and their descriptions.",
	.main = __mlib_help,
};

/*
 * Exit a program using MLib. Mostly for the shell.
 *
 * @argc:	The arg count
 * @argv:	The argument vector
 */
int __mlib_exit(int argc, char *argv[])
{
	exit(0);
	return 0;
}

static struct mlib_command mlib_command_exit = {
	.name = "exit",
	.desc = "Exit MLib.",
	.main = __mlib_exit,
};

/*
 * Register the built in commands. These are commands that are implemented in
 * the core library.
 */
int mlib_register_builtins()
{
	mlib_command_register(&mlib_command_loadm);
	mlib_command_register(&mlib_command_echo);
	mlib_command_register(&mlib_command_run);
	mlib_command_register(&mlib_command_help);
	mlib_command_register(&mlib_command_exit);
	return 0;
}