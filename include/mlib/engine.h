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
 * Engine definitions and data types.
 */

#ifndef _MLIB_ENGINE_H_
#define _MLIB_ENGINE_H_

#include <mlib/list.h>

struct mlib_library;
struct mlib_playlist;

/*
 * Capability flags.
 */
#define MLIB_ENGINE_UNKNOWN	(0)
#define MLIB_ENGINE_AUDIO	(0x1 << 0)
#define MLIB_ENGINE_VIDEO	(0x1 << 1)

/*
 * Playback flags.
 */
#define MLIB_ENGINE_REPEAT	(0x1 << 0)
#define MLIB_ENGINE_RANDOM	(0x1 << 1)

extern char *mlib_audio_ext[];
extern char *mlib_video_ext[];

/**
 * Struct for describing an engine for media playing.
 */
struct mlib_engine {
	const char	*name;
	int		 caps; /* Bitwise OR of capabilities (video/audio). */
	int		(*play_audio)(const char *uri);
	int		(*play_video)(const char *uri);
	int		(*toggle)(void);
	int		(*stop)(void);

	struct list_head	 list;
};

/*
 * Engine functions.
 */
int	 mlib_engine_init();
int	 mlib_engine_register(struct mlib_engine *engine);
int	 mlib_play_media(const struct mlib_library *lib,
			 const struct mlib_playlist *plist, const char *entry);
int	 mlib_play(const struct mlib_library *lib,
		   const struct mlib_playlist *plist, int flags);

#endif
