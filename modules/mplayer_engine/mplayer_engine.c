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
 * Wrapper to use mplayer to play media URIs.
 */

#include <mlib/mlib.h>
#include <mlib/engine.h>
#include <mlib/module.h>

int mplayer_play_URI(const char *uri)
{
	mlib_queue_printf("Playing %s\n", uri);
	return 0;
}

static struct mlib_engine mplayer_engine = {
	.name = "mplayer-engine",
	.caps = MLIB_ENGINE_AUDIO | MLIB_ENGINE_VIDEO,
	.play_audio = mplayer_play_URI,
	.play_video = mplayer_play_URI,
};

int mplayer_engine_init(void)
{
	return mlib_engine_register(&mplayer_engine);
	return 0;
}

int mplayer_engine_fini(void)
{
	return 0;
}

MLIB_MODULE("mplayer-engine", mplayer_engine_init, mplayer_engine_fini,
	    "Mplayer backend to play media URIs.");
