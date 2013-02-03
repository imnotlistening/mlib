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
 * I/O handler. Control input from the user vs script files.
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <mlib/mlib.h>
#include <mlib/mlib_shell.h>

struct mlib_input_stream {
	FILE			*stream;
	struct list_head	 list;
};

/* static LIST_HEAD(completions); */

static LIST_HEAD(input_streams);
FILE *current_stream;
char *mlib_text;

extern int yylex(void);

/**
 * Initilize the MLib terminal I/O.
 */
int mlib_init_io()
{
	using_history();
	current_stream = stdin;

	return 0;
}

/**
 * Push a stream onto the MLib IO stack. If the stream is NULL then stdin will
 * be used. Returns < 0 on failure and 0 on success.
 *
 * @stream:	Name of the input stream.
 */
int mlib_push_stream(const char *stream)
{
	struct mlib_input_stream *str;

	if (!stream)
		return -1;

	str = malloc(sizeof(struct mlib_input_stream));
	if (!str)
		return -1;

	str->stream = fopen(stream, "r");
	if (!str->stream) {
		mlib_error("Failed to open %s: %s\n", stream,
			   strerror(errno));
		goto fail;
	}
	list_add(&str->list, &input_streams);
	current_stream = str->stream;
	return 0;

fail:
	free(str);
	return -1;
}

/**
 * Pops the current stream. If the stream stack is empty, then return -1.
 */
int mlib_pop_stream()
{
	struct mlib_input_stream *top;

	if (list_empty(&input_streams))
		return -1;

	top = list_first_entry(&input_streams, struct mlib_input_stream, list);
	if (top->stream != stdin)
		fclose(top->stream);
	list_del(&top->list);
	free(top);

	/* If we poped the last stream, use stdin. */
	if (list_empty(&input_streams))
		current_stream = stdin;

	return -1;
}

/**
 * Reads a line of input. The input could be from a file or the terminal.
 */
int mlib_read_line(int *argc, char ***argv)
{
	int lex_result;
	int local_argc = 0, len = 0;
	char **local_argv = NULL, **tmp;

	do {
restart:
		lex_result = yylex();
		switch (lex_result) {
		case MLIB_LEX_DONE:
			mlib_pop_stream();
		case MLIB_LEX_EOL:
			if (local_argc == 0)
				goto restart;
			goto done_line;
		case MLIB_LEX_ERROR:
			goto error;
		}

		/*
		 * If we did not hit one of the special cases above, add the
		 * passed string to argv.
		 */
		if ((local_argc + 1) >= len) {
			len += 8;
			tmp = realloc(local_argv, len * sizeof(char *));
			if (!tmp)
				goto error;
			local_argv = tmp;
		}
		local_argv[local_argc++] = mlib_text;
	} while (lex_result == MLIB_LEX_STRING);

done_line:
	*argc = local_argc;
	*argv = local_argv;
	if (lex_result == MLIB_LEX_DONE && current_stream != stdin)
		return -1;
	else
		return 0;

error:
	mlib_error("Error reading line.\n");
	free(local_argv);
	*argv = NULL;
	return -1;
}

/*
 * Read a line from the terminal. Store the number of bytes read in *bytes.
 */
char *__mlib_readbuf_term(char *buffer, size_t max)
{
	int max_bytes;
	static int buff_size;
	static int read_bytes;
	static char *readline_buff = NULL;

	if (!readline_buff) {
		while (1) {
			readline_buff = readline(MLIB_PROMPT);
			if (!readline_buff)
				return NULL;
			buff_size = strlen(readline_buff);
			if (buff_size > 0)
				break;
		}
		read_bytes = 0;
	}

	/* Reset condition. */
	if (read_bytes >= buff_size) {
		free(readline_buff);
		readline_buff = NULL;
		buff_size = read_bytes = 0;
		return NULL;
	}

	/* Otherwise, copy bytes from the read line into buffer. */
	memset(buffer, 0, max);
	max_bytes = buff_size - read_bytes;
	if (max_bytes > max)
		max_bytes = max;
	memcpy(buffer, readline_buff + read_bytes, max_bytes);
	read_bytes += max_bytes;

	return buffer;
}

/*
 * Read a buffer from the current input.
 */
size_t mlib_readbuf(char *buffer, size_t max)
{
	size_t ret;
	char *stat = NULL;

	if (!current_stream) {
		mlib_error("No current stream.\n");
		return 0;
	}

	if (current_stream != stdin) {
		stat = fgets(buffer, max, current_stream);
	} else {
		stat = __mlib_readbuf_term(buffer, max);
		if (stat && strlen(stat) > 0)
			add_history(stat);
	}

	if (!stat)
		return 0;

	ret = strlen(stat);
	return ret;
}
