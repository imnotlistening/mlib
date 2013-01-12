%{
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
 * Lexxer to parse strings from files/terminal.
 */

#include <mlib/mlib.h>
#include <mlib/mlib_shell.h>

extern char *mlib_text;

/*
 * We define our own YY_INPUT routine so that we can use the lexxer for both
 * files and the terminal with gnu readline support.
 */
#define YY_INPUT(buf, result, max_size) result = mlib_readbuf(buf, max_size)
#define YY_NO_INPUT

%}

WORD		[[:graph:]]{-}[^\"\']+

%s SINGLE_QUOTE DOUBLE_QUOTE

%option nounput

%%
#.*\n	{ /* Eat comments */
		mlib_text = NULL;
		return MLIB_LEX_EOL;
	}

\'	BEGIN(SINGLE_QUOTE);
\"	BEGIN(DOUBLE_QUOTE);

[[:graph:]]{-}[\"\']+	{ /* Just a regular word, no quotes. */
		mlib_text = strdup(yytext);
		return MLIB_LEX_STRING;
	}
<SINGLE_QUOTE>[[:print:]^\']+\'	{
		mlib_text = strdup(yytext);
		mlib_text[strlen(mlib_text) - 1] = 0;
		BEGIN(INITIAL);
		return MLIB_LEX_STRING;
	}
<DOUBLE_QUOTE>[[:print:]^\"]+\"	{
		mlib_text = strdup(yytext);
		mlib_text[strlen(mlib_text) - 1] = 0;
		BEGIN(INITIAL);
		return MLIB_LEX_STRING;
	}

\n	{ /* End of line. */
		mlib_text = NULL;
		return MLIB_LEX_EOL;
	}

[[:blank:]]+ /* Eat spaces. */
.	{ /* Eat anything else. */
		mlib_printf("[lexxer] error: unknown character: %c\n",
			    *yytext);
		return MLIB_LEX_ERROR;
	}

%%

/*
 * Let the scanner terminate. We will handle this ourselves.
 */
int yywrap() {
	return 1;
}
