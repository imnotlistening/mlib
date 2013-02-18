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
 *
 * This stock media playing engine can be replaced by a module if so desired.
 * All of the loaded media backends can be searched and used by other modules.
 * However, those modules must take care to ensure that they use the lock
 * provided by the backends to ensure no concurrency.
 */

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <mlib/mlib.h>
#include <mlib/engine.h>

struct mlib_thread_args {
	const struct mlib_library	*lib;
	char 			 	*elem; /* Save first 80 chars. */
	const struct mlib_playlist	*plist;
	int		use_plist;	/* Set if this is a playlist. */
	int		in_use;		/* Set if something is playing. */
	int		status;		/* Set if playing, 0 if paused. */
	pthread_mutex_t lock;
};

/*
 * Terrible hack: list of file name endings for audio/video. These can be added
 * to at run time by modules. This is how the engine distinguishes between
 * audio and video files. If the engine does not what type of file the passed
 * URL is then it tries to play the file as video first, then falls back to
 * audio if that fails.
 */
char *mlib_audio_ext[] = {
	"mp3", "aac", "ac3", "wav", "wave", "flac", "wma", NULL
};
char *mlib_video_ext[] = {
	"mkv", "mpeg", "avi", "wmv", NULL
};

static LIST_HEAD(mlib_engines);
static struct mlib_engine *current;

/*
 * Only one media stream should ever be active.
 */
static struct mlib_thread_args playing = {
	.lib = NULL,
	.elem = NULL,
	.plist = NULL,
	.in_use = 0,
	.lock = PTHREAD_MUTEX_INITIALIZER
};

/*
 * Check the file extension to guess what type of file we are playing.
 */
int __mlib_guess_media_type(const char *uri)
{
	return MLIB_ENGINE_UNKNOWN;
}

char *__build_uri(const struct mlib_library *lib, const char *entry)
{
	int entry_len = strlen(entry);
	int prefix_len = strlen(MLIB_LIB_PREFIX(lib));
	char *uri = malloc(entry_len + prefix_len + 2);

	strcat(uri, MLIB_LIB_PREFIX(lib));
	strcat(uri, "/");
	strcat(uri, entry);
	return uri;
}

/*
 * Play an audio stream.
 */
int __mlib_play_audio(const char *uri)
{
	if (!(current->caps & MLIB_ENGINE_AUDIO) ||
	    !current->play_audio) {
		mlib_error("Current media back end does not support audio.\n");
		return -1;
	}
	return 0;
}

/*
 * Play a video stream.
 */
int __mlib_play_video(const char *uri)
{
	if (!(current->caps & MLIB_ENGINE_VIDEO) ||
	    !current->play_video) {
		mlib_error("Current media back end does not support video.\n");
		return -1;
	}
	return current->play_video(uri);
}

/*
 * Plays the passed media entry from the passed library. This assumes that the
 * entry is actually in the library. This is also not locked or synchronized in
 * any way. This expects that you have alreayd ensured that no other thread is
 * playing media on this engine.
 */
int __mlib_play_media(const struct mlib_library *lib, const char *entry)
{
	int type, stat;
	char *uri;

	if (!current) {
		mlib_error("No media engine set.\n");
		return -1;
	}

	uri = __build_uri(lib, entry);
	type = __mlib_guess_media_type(uri);

	switch (type) {
	case MLIB_ENGINE_VIDEO:
	case MLIB_ENGINE_UNKNOWN:
		/*
		 * Treat unknown streams as video: try to play it as a video
		 * source but if that fails try to play it as an audio source.
		 * Some engines may also do this inherently as well.
		 */
		stat = __mlib_play_video(uri);
		if (stat) {
			mlib_queue_printf("Falling back to audio: %s\n", uri);
			stat = __mlib_play_audio(uri);
		}
		break;
	case MLIB_ENGINE_AUDIO:
		stat = __mlib_play_audio(uri);
		break;
	default:
		mlib_error("BUG: invalid media type descriptor.\n");
		stat = -1;
		break;
	}

	free(uri);
	return stat;
}

/*
 * Thread for playing a single media URI.
 */
void *__mlib_play_media_thread(void *targs)
{
	__mlib_play_media(playing.lib, playing.elem);

	pthread_mutex_lock(&playing.lock);
	playing.in_use = 0;
	free(playing.elem);
	pthread_mutex_unlock(&playing.lock);

	return NULL;
}

/*
 * Thread to play a playlist.
 */
void *__mlib_play_playlist_thread(void *targs)
{
	mlib_queue_printf(">> Playing playlist (pretend): %s\n",
			  MLIB_PLIST_NAME(playing.plist));
	sleep(15);
	mlib_queue_printf(">> Done\n");

	pthread_mutex_lock(&playing.lock);
	playing.in_use = 0;
	pthread_mutex_unlock(&playing.lock);

	return NULL;
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
	pthread_t thr;

	pthread_mutex_lock(&playing.lock);
	if (playing.in_use) {
		mlib_printf("Already playing.\n");
		pthread_mutex_unlock(&playing.lock);
		return -1;
	}

	playing.lib = lib;
	playing.use_plist = 0;
	playing.in_use = 1;

	/* With elems, get a copy. Freed by the thread when its done.  */
	playing.elem = strdup(entry);

	pthread_mutex_unlock(&playing.lock);

	if (pthread_create(&thr, NULL, __mlib_play_media_thread, NULL)) {
		mlib_perror("Failed to make thread");
		return -1;
	}

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
	      const struct mlib_playlist *plist, int flags)
{
	pthread_t thr;

	pthread_mutex_lock(&playing.lock);
	if (playing.in_use) {
		mlib_printf("Already playing.\n");
		pthread_mutex_unlock(&playing.lock);
		return -1;
	}

	playing.lib = lib;
	playing.plist = plist;
	playing.use_plist = 1;
	playing.in_use = 1;
	pthread_mutex_unlock(&playing.lock);

	if (pthread_create(&thr, NULL, __mlib_play_playlist_thread, NULL)) {
		mlib_perror("Failed to make thread");
		return -1;
	}

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
	if (!current)
		current = engine;

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

/*
 * Play a playlist or media URI. Usage:
 *
 *   play [<lib> <playlist|elem>]
 *
 * If the argument matches a playlist, then that playlist is played. If no
 * playlist of the passed name exists then the library is searched to see if
 * the passed elem exists as a path. If so, that path is individually played.
 */
int __mlib_play(int argc, char *argv[])
{
	struct mlib_library *lib;
	struct mlib_playlist *plist;
	int ret;

	if (argc != 3) {
		mlib_printf("Usage: play [<lib> <playlist|elem>]\n");
		return -1;
	}

	lib = mlib_find_library(argv[1]);
	if (!lib) {
		mlib_printf("Unknown library - %s\n", argv[1]);
		return -1;
	}

	plist = mlib_find_playlist(lib, argv[2]);

	/* Now, depending on plist, we either play the plist or treat argv[2]
	 * as an element in the passed library. */
	if (plist)
		ret = mlib_play(lib, plist, 0);
	else
		ret = mlib_play_media(lib, NULL, argv[2]);

	if (ret)
		mlib_printf("Status = failed\n");
	else
		mlib_printf("Status = success\n");

	return 0;
}

static struct mlib_command mlib_command_play = {
	.name = "play",
	.desc = "Play media.",
	.main = __mlib_play,
};

/*
 * Pause the current media stream. If the stream is already paused, then the
 * stream is restarted. Usage:
 *
 *   pause
 */
int __mlib_pause(int argc, char *argv[])
{
	if (!current || !current->toggle)
		return -1;

	if (!playing.in_use)
		return 0;

	current->toggle();
	return 0;
}

static struct mlib_command mlib_command_pause = {
	.name = "pause",
	.desc = "Pause the currently playing media.",
	.main = __mlib_pause,
};

/*
 * Stop the current media stream. Usage:
 *
 *   stop
 */
int __mlib_stop(int argc, char *argv[])
{
	if (!current || !current->stop)
		return -1;

	if (!playing.in_use)
		return 0;

	current->stop();
	return 0;
}

static struct mlib_command mlib_command_stop = {
	.name = "stop",
	.desc = "Stop the currently playing media.",
	.main = __mlib_stop,
};

int mlib_engine_init()
{
	mlib_command_register(&mlib_command_play);
	mlib_command_register(&mlib_command_pause);
	mlib_command_register(&mlib_command_stop);
	mlib_command_register(&mlib_command_engines);
	return 0;
}
