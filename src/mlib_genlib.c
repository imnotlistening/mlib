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
 * A simple program to add all media files in the passed directories to an MLib
 * library.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <dirent.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <mlib/mlib.h>

#define genlib_print(FMT, ...)						\
	do {								\
		if (verbose)						\
			mlib_printf(FMT, ##__VA_ARGS__);		\
	} while (0)

static void	die(char *msg);
static void	die_help(void);
static int	do_search(void);
static int	parse_args(int argc, char *argv[]);

static char **custom_list;
static char *output;
static char *prefix;
static char *name;
static char *dir;
static int   verbose;
static int   overwrite;

static struct mlib_library *lib;

static const struct option opts[] = {
	{ "filter",	1, NULL, 'f' },
	{ "prefix",	1, NULL, 'p' },
	{ "overwrite",	0, NULL, 'o' },
	{ "name",	1, NULL, 'n' },
	{ "verbose",	0, NULL, 'v' },
	{ "help",	0, NULL, 'h' },
	{ NULL,		0, NULL,  0  }
};
static const char *short_opts = "f:p:oO:n:vh";

int main(int argc, char *argv[])
{
	struct stat buf;

	mlib_init();

	if (parse_args(argc, argv))
		die("failed to parse arguments");

	if (!prefix) {
		prefix = realpath(dir, NULL);
		if (!prefix) {
			mlib_perror("Failed to make prefix");
			die("Failed.");
		}
	}
	if (!name)
		name = "genlib library";
	if (!output)
		output = "genlib.mlib";

	if (stat(output, &buf) == 0) {
		if (overwrite) {
			if (unlink(output)) {
				mlib_perror("Failed to delete %s", output);
				die("Failed.");
			}
			if (mlib_create_library(output, name, prefix))
				die("Failed.");
		}
	} else {
		mlib_printf("Creating library: %s\n", output);
		mlib_create_library(output, name, prefix);
	}

	lib = mlib_open_library(output, 0);
	if (!lib)
		die("Failed.");

	do_search();

	return 0;
}

/*
 * Allocates the mem using malloc(). Make sure to free after use.
 */
static char *path_combine(const char *path, const char *file)
{
	char *new_name;

	new_name = malloc(strlen(path) + strlen(file) + 2);
	new_name[0] = 0;
	strcat(new_name, path);
	strcat(new_name, "/");
	strcat(new_name, file);
	return new_name;
}

/*
 * Get a string that points to only the unique part of the path relative to the
 * prefix of the library. This depends on the library being opened already.
 */
static char *drop_prefix(char *path)
{
	char *str;
	const char *prefix = MLIB_LIB_PREFIX(lib);

	str = path;
	while (*str++ == *prefix++)
		;

	if (*str == '/')
		str++;

	return str;
}

/*
 * Recursive search.
 */
static int __do_search(const char *cwd)
{
	int ret;
	DIR *dir;
	char *new_name, *elem;
	struct stat buf;
	struct dirent *ent;
	struct mlib_playlist *plist;

	dir = opendir(cwd);
	if (!dir) {
		mlib_perror("opendir: %s", cwd);
		return -1;
	}

	/*
	 * Do this in two passes. First add any media files in the current dir
	 * and then recurse into sub-dirs.
	 */
	while ((ent = readdir(dir)) != NULL) {
		new_name = path_combine(cwd, ent->d_name);
		plist = mlib_find_playlist(lib, ".global");
		if (!plist)
			die("Failed.");
		if (stat(new_name, &buf)) {
			mlib_perror("stat: %s", new_name);
			return -1;
		}
		if (S_ISDIR(buf.st_mode)) {
			goto skip_dir;
		}

		elem = drop_prefix(new_name);

		if (mlib_find_path(plist, elem))
			continue;

		if (custom_list) {
			if (!mlib_filter(elem, custom_list)) {
				genlib_print("- Rejecting %s\n", elem);
				continue;
			}
		} else {
			if (!mlib_filter(elem, mlib_audio_ext) &&
			    !mlib_filter(elem, mlib_video_ext)) {
				genlib_print("- Rejecting %s\n", elem);
				continue;
			}
		}

		genlib_print("+ %s\n", elem);
		if (mlib_add_path(lib, ".global", elem))
			die("Failed.");
	skip_dir:
		free(new_name);
	}
	rewinddir(dir);
	while ((ent = readdir(dir)) != NULL) {
		new_name = path_combine(cwd, ent->d_name);
		if (stat(new_name, &buf)) {
			mlib_perror("stat: %s", ent->d_name);
			return -1;
		}
		if (!S_ISDIR(buf.st_mode) ||
		    strcmp(ent->d_name, ".") == 0 ||
		    strcmp(ent->d_name, "..") == 0)
			goto skip_files;

		genlib_print("> %s\n", new_name);
		ret = __do_search(new_name);
	skip_files:
		free(new_name);
		if (ret)
			return -1;
	}
	return 0;
}

/*
 * Do the actual search. This expects everything to have been set up.
 */
static int do_search()
{
	mlib_printf("Using library:  %s\n", output);
	mlib_printf("Library name:   %s\n", MLIB_LIB_NAME(lib));
	mlib_printf("Library prefix: %s\n", MLIB_LIB_PREFIX(lib));
	mlib_printf("Searching dir:  %s\n", dir);

	__do_search(dir);

	return 0;
}

static int parse_args(int argc, char *argv[])
{
	int opt;

	while ((opt = getopt_long(argc, argv, short_opts, opts, NULL)) != -1) {
		switch (opt) {
		case 'f':
			custom_list = mlib_parse_list(optarg, ',', NULL);
			if (!custom_list)
				mlib_printf("Failed to parse filter list.\n");
			break;
		case 'p':
			prefix = optarg;
			break;
		case 'o':
			overwrite = 1;
			break;
		case 'n':
			name = optarg;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'h':
			die_help();
			break; /* Should never hit this. */
		case '?':
			mlib_printf("Missing option to %s\n", argv[optind]);
			return -1;
		default:
			mlib_printf("Error parsing options.\n");
			return -1;
		}
	}

	if ((argc - optind) < 2) {
		mlib_printf("Missing manditory arguments.\n");
		die_help();
	}

	dir = argv[optind];
	output = argv[optind + 1];

	return 0;
}

static void die(char *msg)
{
	mlib_printf("mlib-genlib exiting: %s\n", msg);
	exit(1);
}

static void die_help(void)
{
	printf(
"A simple program to add all media files in the passed directory to an MLib\n\
library.\n\
Usage:\n\
\n\
  mlib-genlib [OPTIONS] <dir> <lib>\n\
\n\
Where OPTIONS are:\n\
\n\
  -f|--filter <list>	A comma separated list of media suffixes to match\n\
			against - i.e 'mkv'. If not specified the builtin\n\
			list will be used.\n\
  -p|--prefix <prefix>	The prefix to use for the library. This is useful if\n\
			the ibrary is goin to be a remote served library and\n\
			the prefix is a URL of some sort - for instance\n\
			http://my.serv.net/music. If not specified the path\n\
			to the directory will be used.\n\
  -o|--overwrite	Overwrite an existing library if necessary. Otherwise\n\
			the any new entries will be added to the existing\n\
			library. If the target library does not exist yet\n\
			one will be created.\n\
  -n|--name <name>	Specify a name for the library if it gets created.\n\
  -v|--verbose		Be verbose.\n\
  -h|--help		Print this help message.\n\
\n\
The argument <dir> must be a directory name. Although it can be specified as\n\
a relative path, if no prefix is specified, the absolute path of the\n\
directory will be used. <name> specifies the name of the library.\n");
	die("done");
}
