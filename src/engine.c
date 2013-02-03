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
 * The media engine itself. Here we handle playback of media from libraries.
 * There is a hook for modules to register media playing backends that we can
 * use to play media.
 */

#include <stdlib.h>
#include <pthread.h>

#include <mlib/mlib.h>
#include <mlib/engine.h>

static LIST_HEAD(mlib_engines);

/*
 * Only one media stream should ever be active.
 */
static pthread_mutex_t mlib_playlock = PTHREAD_MUTEX_INITIALIZER;

/*
 * Internel non-locked version of mlib_play_media(). This is for the playlist
 * player which will take the play back lock on it's own.
 */
int __mlib_play_media(const struct mlib_library *lib,
		      const struct mlib_playlist *plist, const char *entry)
{
	return 0;
}

/**
 * Plays a media stream. The stream is searched for in the passed library. If
 * @plist is non-NULL then the only playlist that is searched is the playlist
 * pointed to by @plist. If @plist is NULL then the global playlist for the
 * library is searched.
 */
int mlib_play_media(const struct mlib_library *lib,
		    const struct mlib_playlist *plist, const char *entry)
{
	if (pthread_mutex_trylock(&mlib_playlock)) {
		mlib_user_error("Playback already running.\n");
		return -1;
	}

	pthread_mutex_lock(&mlib_playlock);
	pthread_mutex_unlock(&mlib_playlock);

	return 0;
}

/**
 * Plays a playlist from the passed library. How the play list is played is
 * controlled by the bitwise OR of some set of MLIB_ENGINE_* flags.
 *
 * @lib		Library to use.
 * @plist	Play list to play. If NULL .global is used.
 * @flags	Bitwise OR of flags that control playback behavior.
 */
int mlib_play(const struct mlib_library *lib,
	      const struct mlib_playlist *plist)
{
	if (pthread_mutex_trylock(&mlib_playlock)) {
		mlib_user_error("Playback already running.\n");
		return -1;
	}

	pthread_mutex_lock(&mlib_playlock);
	pthread_mutex_unlock(&mlib_playlock);

	return 0;
}

/**
 * Register a media playing engine with MLib.
 *
 * @engine	The engine to register.
 */
int mlib_engine_register(struct mlib_engine *engine)
{
	if (!engine->name) {
		mlib_error("Ignoring engine with no name.\n");
		return -1;
	}

	if (engine->caps & MLIB_ENGINE_AUDIO && !engine->play_audio) {
		mlib_error("Warning: engine reports audio capability but "
			   "does not provide and interface to do so.\n");
		engine->caps &= ~MLIB_ENGINE_AUDIO;
	}
	if (engine->caps & MLIB_ENGINE_VIDEO && !engine->play_video) {
		mlib_error("Warning: engine reports video capability but "
			   "does not provide and interface to do so.\n");
		engine->caps &= ~MLIB_ENGINE_VIDEO;
	}

	if (!engine->caps) {
		mlib_error("Ignoring engine %s - no capabilities.\n",
			   engine->name);
		return -1;
	}

	/* Ok, looks like the engine is ok to add. */
	list_add_tail(&engine->list, &mlib_engines);
	return 0;
}

/*
 * Display the list of currently loaded media engines. Usage
 *
 *   engines
 */
int __mlib_engines(int argc, char *argv[])
{
	struct list_head *elem;
	struct mlib_engine *engine;

	if (argc > 2) {
		mlib_printf("Usage: lseng [type]\n");
		return -1;
	}

	list_for_each(elem, &mlib_engines) {
		engine = list_entry(elem, struct mlib_engine, list);

		mlib_printf("%s - supports:", engine->name);
		if (engine->caps & MLIB_ENGINE_AUDIO)
			mlib_printf(" AUDIO");
		if (engine->caps & MLIB_ENGINE_VIDEO)
			mlib_printf(" VIDEO");

		mlib_printf("\n");
	}

	return 0;
}

static struct mlib_command mlib_command_engines = {
	.name = "engines",
	.desc = "Display loaded engines.",
	.main = __mlib_engines,
};

int mlib_engine_init()
{
	mlib_command_register(&mlib_command_engines);
	return 0;
}
