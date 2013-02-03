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
 * Load modules.
 */

#include <dlfcn.h>
#include <stdlib.h>
#include <unistd.h>

#include <mlib/mlib.h>
#include <mlib/module.h>

static LIST_HEAD(mlib_modules);

/*
 * Quick check to see if a module is loaded already. Returns non-zero if the
 * passed module shares a name with any already loaded module.
 */
int mlib_module_loaded(const struct mlib_module *mod)
{
	struct list_head *elem;
	struct mlib_module *module;

	list_for_each(elem, &mlib_modules) {
		module = list_entry(elem, struct mlib_module, list_node);
		if (module == mod)
			return 1;
	}

	return 0;
}

/**
 * Load the module in the passed string. A 0 is returned if successful, other
 * wise < 0 will be returned.
 *
 * @path	Path to the module. Will be resolved based on shared library
 * 		loading rules.
 */
int mlib_load_module(const char *path)
{
	void *mod_syms;
	struct mlib_module *module;

	mod_syms = dlopen(path, RTLD_NOW | RTLD_GLOBAL);
	if (!mod_syms) {
		mlib_error("Failed to load %s: %s\n", path, dlerror());
		return -1;
	}

	module = dlsym(mod_syms, "__mlib_module_desc__");
	if (!module) {
		mlib_error("Failed to find module definition: %s\n",
			    dlerror());
		return 0;
	}
	module->mod_syms = mod_syms;

	if (!module->name) {
		mlib_error("Module must have a name.\n");
		goto fail;
	}

	if (mlib_module_loaded(module)) {
		mlib_user_error("Module '%s' already loaded.\n", module->name);
		return -1; /* We don't want to free the module... */
	}

	if (module->init && module->init())
		goto fail;

	list_add_tail(&module->list_node, &mlib_modules);
	return 0;

 fail:
	dlclose(mod_syms);
	return -1;
}

/*
 * Load a module. Usage:
 *
 *   load <mod-name>
 */
int __mlib_load_module(int argc, char *argv[])
{
	if (argc != 2) {
		mlib_printf("Usage: load <mod-name>\n");
		return -1;
	}

	if (mlib_load_module(argv[1]))
		return -1;

	mlib_printf("Loaded %s\n", argv[1]);
	return 0;
}

static struct mlib_command mlib_command_load = {
	.name = "load",
	.desc = "Load a module.",
	.main = __mlib_load_module,
};

/*
 * Display the list of currently loaded modules. Usage
 *
 *   lsmod
 */
int __mlib_lsmod(int argc, char *argv[])
{
	struct list_head *elem;
	struct mlib_module *module;

	if (argc != 1) {
		mlib_printf("Usage: lsmod\n");
		return -1;
	}

	mlib_printf("Loaded modules:\n");
	list_for_each(elem, &mlib_modules) {
		module = list_entry(elem, struct mlib_module, list_node);

		mlib_printf("  %-11s", module->name);
		if (module->desc)
			mlib_printf(" %s\n", module->desc);
		else
			mlib_printf("\n");
	}

	return 0;
}

static struct mlib_command mlib_command_lsmod = {
	.name = "lsmod",
	.desc = "Display loaded modules.",
	.main = __mlib_lsmod,
};

/*
 * Init the module loading code.
 */
int mlib_module_init(void)
{
	mlib_command_register(&mlib_command_load);
	mlib_command_register(&mlib_command_lsmod);
	return 0;
}
