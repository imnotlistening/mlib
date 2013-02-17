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

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/wait.h>
#include <sys/types.h>

#include <mlib/mlib.h>
#include <mlib/engine.h>
#include <mlib/module.h>

static pid_t mplayer_pid;
static int   mplayer_write_fd = -1;

//static char **mplayer_args = "";

int mplayer_exec(const char *uri)
{
	int io_pipe[2];

	if (pipe(io_pipe)) {
		mlib_queue_printf("mplayer pipe() failed.\n");\
		perror("pipe");
		return -1;
	}

	mplayer_pid = fork();
	if (mplayer_pid < 0) {
		mlib_queue_printf("mplayer fork() failed.\n");
		perror("fork");
		return -1;
	}

	/* Parent. We are done. */
	if (mplayer_pid) {
		close(io_pipe[0]);
		mplayer_write_fd = io_pipe[1];
		return 0;
	}

	if (dup2(0, io_pipe[0]) < 0) {
		mlib_queue_printf("mplayer dup2() failed.\n");
		perror("dup2");
		exit(1);
	}

	/* Child. */
	close(io_pipe[1]);
	close(1);
	close(2); /* We don't care about the output. */

	execlp("mplayer", "mplayer", "-slave", uri, NULL);
	exit(1);
	return 0; /* Should never happen. */
}

/*
 * This should block until the stream is done. This is threaded internally to
 * mlib.
 */
int mplayer_play_URI(const char *uri)
{
	int status;

	mlib_queue_printf("[mplayer] Playing %s\n", uri);
	if (mplayer_exec(uri))
		return -1;

	mlib_queue_printf("mplayer pid = %d\n", mplayer_pid);
	mlib_queue_printf("mplayer fd  = %d\n", mplayer_write_fd);

	waitpid(mplayer_pid, &status, 0);
	if (!WIFEXITED(status))
		return -1;
	if (WEXITSTATUS(status))
		return -1;
	mlib_queue_printf("mplayer done.\n");
	close(mplayer_write_fd);
	return 0;
}

int mplayer_pause(void)
{
	int ret;

	mlib_queue_printf("Pausing mplayer.\n");	
	ret = write(mplayer_write_fd, " ", 1) == 1 ? 0 : -1;
	if (ret < 1) {
		perror("write");
		mlib_printf("Failed to pause/play mplayer.\n");
		return -1;
	}

	return fsync(mplayer_write_fd);
}

int mplayer_stop(void)
{
	int ret;

	mlib_queue_printf("Killing mplayer.\n");	
	ret = write(mplayer_write_fd, "q", 1) == 1 ? 0 : -1;
	if (ret < 1) {
		mlib_queue_printf("Failed to kill mplayer.\n");
		return -1;
	}

	return fsync(mplayer_write_fd);
}

static struct mlib_engine mplayer_engine = {
	.name = "mplayer-engine",
	.caps = MLIB_ENGINE_AUDIO | MLIB_ENGINE_VIDEO,
	.play_audio = mplayer_play_URI,
	.play_video = mplayer_play_URI,
	.toggle = mplayer_pause,
	.stop = mplayer_stop,
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
