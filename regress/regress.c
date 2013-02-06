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
 * Regression test for MLib core fucntionality - primarily library and playlist
 * features.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include <mlib/mlib.h>

#include <regress.h>

static struct option regress_opts[] = {
	{ "help",		0, NULL, 'h' },
	{ "out-file",		1, NULL, 'o' },
	{ "verbose",		0, NULL, 'v' },
	{ "abort",		0, NULL, 'a' },

	/* NULL terminate. */
	{ NULL,			0, NULL,  0  },
};
static const char *regress_short_opts = ":hvao:";

/*
 * All regression tests to be executed. They are executed one after another.
 */
static struct regression_test regressions[]  __attribute__ ((unused)) = {
	REGRESSION("Verify create", 0, regress_verify_create, ".tlib"),
	REGRESSION("Verify open", 0, regress_verify_open, ".tlib"),
	REGRESSION("Verify mk/rm of plists", CREATE_LIBRARY,
		   regress_verify_mk_rm_pls, NULL),
	REGRESSION("Add element to playlist", CREATE_LIBRARY,
		   regress_verify_add_to_plist, NULL),

	/* NULL terminator. */
	REGRESSION(NULL, 0, NULL, NULL),
};

/*
 * Behavior modifying fields.
 */
static int	 abort_on_fail = 0;
static int	 verbose = 0;
static char	*output_file = NULL;

int main(int argc, char *argv[])
{
	if (parse_args(argc, argv))
		die_print_help();

	mlib_library_init();
	do_regressions();

	return 0;
}

int do_single_regression(struct regression_test *test)
{
	int pass;
	struct mlib_library *lib = NULL;

	if (test->flags & CREATE_LIBRARY) {
		if (mlib_create_library(".test-mlib.lib", "tl", "./")) {
			printf("-- Regression: failed to make internal lib\n");
			printf("-- Exit: 1.\n");
			exit(1);
		}
		lib = mlib_open_library(".test-mlib.lib", 0);
		if (!lib) {
			printf("-- Regression: failed to open internal lib\n");
			printf("-- Exit: 1.\n");
			exit(1);
		}
	}

	pass = test->func(lib, test->priv);

	if (lib)
		mlib_close_library(lib);
	unlink(".test-mlib.lib");

	printf("%-48s %s\n", test->name, pass ? "FAIL" : "PASS");
	return pass;
}

/*
 * Returns the number of failed tests.
 */
int do_regressions()
{
	int fail;
	int passed = 0, total = 0;
	struct regression_test *test;

	printf("-- Running MLib regressions...\n");
	for (test = regressions; test->func != NULL; test++) {
		total++;

		fail = do_single_regression(test);
		if (fail && abort_on_fail) {
			printf("-- Failure: aborting.\n");
			break;
		}
		if (!fail)
			passed++;
	}

	printf("-- Stats: passed %d/%d  (failures = %d)\n", passed, total,
	       total - passed);
	return total - passed;
}

/*
 * Returns 0 on success. Automatically fills in the behavior fields.
 */
int parse_args(int argc, char *argv[])
{
	int opt;

	opterr = 0;

	do {
		opt = getopt_long(argc, argv,
				  regress_short_opts, regress_opts, NULL);
		switch (opt) {
		case 'h':
			die_print_help();
			break;
		case 'a':
			abort_on_fail = 1;
			break;
		case 'o':
			output_file = optarg;
			break;
		case 'v':
			verbose = 1;
			break;
		case '?':
			mlib_user_error("Option not recognized: %s\n",
					argv[optind-1]);
			return -1;
		case ':':
			mlib_user_error("Missing argument for option: %s\n",
					argv[optind-1]);
			return -1;
		case -1:
			break;
		}
	} while (opt != -1);
	return 0;
}

/*
 * Exits after printing help.
 */
void die_print_help(void)
{
	printf("regress - Regression tests for MLib.\n"
	       "\n"
	       "Usage: regress [-hav] [-o output-file]\n"
	       "  Mandatory arguments to long options are manditory for short "
	       "options, too.\n"
	       "\n"
	       "	-h|--help	Print this help message.\n"
	       "	-o|--out-file	Print MLibs normal output to "
	       "the passed file.\n"
	       "	-a|--abort	Abort on test failure.\n"
	       "	-v|--verbose	Print MLib's output to the "
	       "terminal.\n\n");
	exit(1);
}
