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
 * Definitions for making a module and the core module code itself.
 */

#ifndef _MLIB_MODULE_H_
#define _MLIB_MODULE_H_

/**
 * A structure to describe a module. This must be present in each module. See
 * the macro MLIB_MODULE().
 */
struct mlib_module {
	const char		*name;
	const char		*desc;
	int			(*init)(void);
	int			(*fini)(void);
	struct list_head	 list_node;
	void			*mod_syms;
};

/*
 * Module functions for everyone to use.
 */
int mlib_load_module(const char *path);

/*
 * Module functions for mlib core to use.
 */
int mlib_module_init(void);

/**
 * This macro should be called once and only once for each module to be loaded
 * by mlib.
 *
 * @name	Name of the module. Should be a 'char *'.
 * @init	Init function. Gets called right after being loaded.
 * @fini	Function called to cleanup any module resources on closuer.
 */
#define MLIB_MODULE(NAME, INIT, FINI, DESC)		\
	struct mlib_module __mlib_module_desc__ = {	\
		.desc = DESC,				\
		.name = NAME,				\
		.init = INIT,				\
		.fini = FINI,				\
	};

#endif
