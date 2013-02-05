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
 */

#ifndef _REGRESS_H_
#define _REGRESS_H_

#define CREATE_LIBRARY	(1 << 0)

struct mlib_library;

struct regression_test {
	char	*name;
	int	 flags;
	int	(*func)(struct mlib_library *lib, void *priv);
	void	*priv;
};

/**
 * To define a regression test, first implement your test. The test must have
 * an entrance function that matches
 *
 *   int (*func)(struct mlib_library *lib, void *priv)
 *
 * Then make sure to put the name in this header file down below in the section
 * for defining prototypes of the regression functions. This function must
 * return 0 on success, and non-zero on failure.
 *
 * The fields of the regression are as follows. @NAME is what you would expect,
 * the name of the regression test. @FLAGS is the bitwise OR of any available
 * regression flags you wish to use. At the moment that is just CREATE_LIBRARY
 * which means a library will be automatically created and passed to you. Once
 * you are done, it will be closed and removed. Finally @FUNC is the function
 * pointer itself. If CREATE_LIBRARY is not specified in @FLAGS, then the passed
 * library argument will be NULL. The @PRIV field may be specified or left as
 * NULL. If specified, however, the pointer will be passed verbatim to your
 * test.
 */
#define REGRESSION(NAME, FLAGS, FUNC, PRIV)	\
	{					\
		.name = NAME,			\
		.flags = FLAGS,			\
		.func = FUNC,			\
		.priv = PRIV,			\
	}

int	 parse_args(int argc, char *argv[]);
void	 die_print_help(void);
int	 do_regressions(void);

/*
 * Function definitions for all regression tests.
 */
int	 regress_verify_create(struct mlib_library *lib, void *priv);
int	 regress_verify_open(struct mlib_library *lib, void *priv);
int	 regress_verify_mk_rm_pls(struct mlib_library *lib, void *priv);

#endif
