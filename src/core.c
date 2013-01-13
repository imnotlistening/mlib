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
 * Core code that all modules and programs will need.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>

#include <mlib/mlib.h>

#include <curl/curl.h>

static pthread_mutex_t mlib_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static LIST_HEAD(mlib_print_queue);

/**
 * Must be called prior to using the MLib library.
 */
int mlib_init()
{
	if (mlib_register_builtins())
		return -1;

	/* Init libcurl. Necessary for loading mlib libraries. */
	curl_global_init(CURL_GLOBAL_ALL);

	return 0;
}

/**
 * Print a message printf() style. This function is for modules and other
 * code to print general info. This must only be used from a foreground
 * context. Back ground messages can be queued with mlib_queue_printf().
 *
 * @fmt:	The printf() format.
 */
int mlib_printf(const char *fmt, ...)
{
	int ret;
	va_list args;

	va_start(args, fmt);
	ret = vprintf(fmt, args);
	va_end(args);
	return ret;
}

/**
 * Queue a message to be printed printf() style. These messages will be printed
 * right before a new prompt is displayed.
 *
 * @fmt:	The printf() format.
 */
int mlib_queue_printf(const char *fmt, ...)
{
	int msg_size = 32, n;
	char *msg_text, *msg_tmp;
	va_list args;
	struct mlib_msg *msg;

	msg_text = malloc(msg_size);
	if (!msg_text)
		return -1;

	/*
	 * Print the message into a string. Try not to waste too much space.
	 */
	while (1) {
		va_start(args, fmt);
		n = vsnprintf(msg_text, msg_size, fmt, args);
		va_end(args);

		if (n > -1 && n < msg_size)
			break;

		if (n > -1)
			msg_size = n + 1;
		else
			msg_size += 32;

		msg_tmp = realloc(msg_text, msg_size);
		if (!msg_tmp) {
			free(msg_text);
			return -1;
		} else {
			msg_text = msg_tmp;
		}
	}

	msg = malloc(sizeof(struct mlib_msg));
	if (!msg) {
		free(msg_text);
		return -1;
	}

	msg->text = msg_text;

	pthread_mutex_lock(&mlib_queue_mutex);
	list_add_tail(&msg->queue, &mlib_print_queue);
	pthread_mutex_unlock(&mlib_queue_mutex);
	return 0;
}

/**
 * Empty the queued messages.
 */
int mlib_empty_print_queue()
{
	struct mlib_msg *msg;
	struct list_head *elem;

	pthread_mutex_lock(&mlib_queue_mutex);

	while (!list_empty(&mlib_print_queue)) {
		elem = mlib_print_queue.next;
		list_del(elem);

		msg = list_entry(elem, struct mlib_msg, queue);
		printf("%s", msg->text);

		free(msg->text);
		free(msg);
	}

	pthread_mutex_unlock(&mlib_queue_mutex);
	return 0;
}
