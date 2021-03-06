/*
 * This file is part of xReader.
 *
 * Copyright (C) 2008 hrimfaxi (outmatch@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "config.h"

#ifdef ENABLE_LYRIC

#include <pspkernel.h>
#include <stdlib.h>
#include <string.h>
#include "common/utils.h"
#include <mad.h>
#include "conf.h"
#include "charsets.h"
#include "lyric.h"
#include "xrhal.h"
#ifdef DMALLOC
#include "dmalloc.h"
#endif

__inline bool lyric_add(p_lyric l, dword sec, dword fra, const char *line,
						dword size)
{
	int i;
	mad_timer_t t;

	if (l->count == 0) {
		l->lines = malloc(sizeof(*l->lines) * 64);
	} else if (l->count % 64 == 0) {
		l->lines = safe_realloc(l->lines, sizeof(*l->lines) * (64 + l->count));
	}
	if (l->lines == NULL) {
		l->count = 0;
		return false;
	}

	t.seconds = sec;
	t.fraction = fra;

	for (i = 0; i < l->count; i++)
		if (mad_timer_compare(l->lines[i].t, t) > 0)
			break;
	if (i < l->count)
		memmove(&l->lines[i + 1], &l->lines[i],
				sizeof(t_lyricline) * (l->count - i));
	l->lines[i].t = t;
	l->lines[i].line = line;
	l->lines[i].size = size;
	l->count++;
	return true;
}

static void parse_lyric(p_lyric l)
{
	int i = 0;
	const char *ls = NULL, *rs = NULL, *ts = NULL;
	dword minute;
	dword *isec = NULL, *iex = NULL;
	int tc = 0;
	double sec;
	bool islyric = false;

	l->count = 0;

	while (i < l->size) {
		if (ls == NULL) {
			switch (l->whole[i]) {
				case '[':
					l->whole[i] = 0;
					if (islyric) {
						if (&ts[0] != &l->whole[i]) {
							int j;

							for (j = 0; j < tc; j++)
								lyric_add(l, isec[j], iex[j], ts, strlen(ts));
							if (tc > 0) {
								tc = 0;
								free(isec);
								free(iex);
							}
						}
					}
					ls = &l->whole[i + 1];
					break;
				case '\r':
				case '\n':
					l->whole[i] = 0;
					break;
			}
		} else {
			switch (l->whole[i]) {
				case ':':
					l->whole[i] = 0;
					if (rs != NULL)
						break;
					rs = &l->whole[i + 1];
					break;
				case ']':
					islyric = false;
					l->whole[i] = 0;
					if (ls != NULL && rs != NULL) {
						if (ls[0] == 0 && rs[0] == 0)
							break;
						if (strcmp(ls, "ar") == 0)
							l->ar = ls;
						else if (strcmp(ls, "ti") == 0)
							l->ti = ls;
						else if (strcmp(ls, "al") == 0)
							l->al = ls;
						else if (strcmp(ls, "by") == 0)
							l->by = ls;
						else if (strcmp(ls, "offset") == 0) {
							if (!utils_string2double(rs, &l->off))
								l->off = 0.0;
						} else if (utils_string2dword(ls, &minute)
								   && utils_string2double(rs, &sec)) {
							islyric = true;
							sec = sec + 60.0 * minute + l->off / 1000;
							if (tc == 0) {
								isec = malloc(sizeof(*isec)
											  * 16);
								iex = malloc(sizeof(*iex)
											 * 16);
							} else if (tc % 16 == 0) {
								isec = safe_realloc(isec, sizeof(*isec)
													* (tc + 16));
								iex = safe_realloc(iex, sizeof(*iex)
												   * (tc + 16));
							}
							if (isec == NULL || iex == NULL) {
								if (isec != NULL)
									free(isec);
								if (iex != NULL)
									free(iex);
								tc = 0;
							} else {
								isec[tc] = (dword) sec;
								iex[tc] = (dword) ((sec - isec[tc])
												   * MAD_TIMER_RESOLUTION);
								++tc;
							}
						}
						if (!islyric) {
							if (tc > 0) {
								tc = 0;
								free(isec);
								free(iex);
							}
						}
					}
					ts = &l->whole[i + 1];
					ls = rs = NULL;
					break;
				case '\r':
				case '\n':
					l->whole[i] = 0;
					ls = rs = NULL;
					break;
			}
		}
		++i;
	}
	if (islyric) {
		int j;

		for (j = 0; j < tc; j++)
			lyric_add(l, isec[j], iex[j], ts, strlen(ts));
	}
	if (tc > 0) {
		tc = 0;
		free(isec);
		free(iex);
	}
	l->idx = -1;
	l->changed = true;
	l->succ = true;
}

extern bool lyric_open_raw(p_lyric l, const char *lyric, size_t size)
{
	if (l == NULL)
		return false;
	memset(l, 0, sizeof(t_lyric));
	l->size = size;
	if ((l->whole = malloc(l->size + 1)) == NULL) {
		return false;
	}
	memcpy(l->whole, lyric, l->size);
	l->whole[l->size] = 0;

	parse_lyric(l);

	return true;
}

extern bool lyric_open(p_lyric l, const char *filename)
{
	int fd;

	if (l == NULL)
		return false;

	memset(l, 0, sizeof(t_lyric));
	fd = xrIoOpen(filename, PSP_O_RDONLY, 0777);

	if (fd < 0)
		return false;
	l->size = xrIoLseek32(fd, 0, PSP_SEEK_END);
	if ((l->whole = malloc(l->size + 1)) == NULL) {
		xrIoClose(fd);
		return false;
	}
	xrIoLseek32(fd, 0, PSP_SEEK_SET);
	xrIoRead(fd, l->whole, l->size);
	l->whole[l->size] = 0;
	xrIoClose(fd);

	parse_lyric(l);

	return true;
}

extern void lyric_init(p_lyric l)
{
	if (l == NULL)
		return;
	memset(l, 0, sizeof(t_lyric));
	l->sema = xrKernelCreateSema("lyric sem", 0, 1, 1, NULL);
}

extern void lyric_close(p_lyric l)
{
	if (l == NULL || !l->succ)
		return;
	xrKernelWaitSema(l->sema, 1, NULL);
	l->succ = false;
	if (l->whole != NULL)
		free(l->whole);
	if (l->count > 0 && l->lines != NULL)
		free(l->lines);
	xrKernelSignalSema(l->sema, 1);
}

extern void lyric_update_pos(p_lyric l, void *tm)
{
	mad_timer_t t;

	if (l == NULL || !l->succ)
		return;
	xrKernelWaitSema(l->sema, 1, NULL);
	t = *(mad_timer_t *) tm;

	while (l->idx >= 0 && mad_timer_compare(l->lines[l->idx].t, t) > 0) {
		l->idx--;
		l->changed = true;
	}
	while (l->idx < l->count - 1
		   && mad_timer_compare(l->lines[l->idx + 1].t, t) < 0) {
		l->idx++;
		l->changed = true;
	}
	xrKernelSignalSema(l->sema, 1);
}

extern bool lyric_get_cur_lines(p_lyric l, int extralines, const char **lines,
								dword * sizes)
{
	int i, j = 0;

	if (l == NULL || !l->succ)
		return false;
	xrKernelWaitSema(l->sema, 1, NULL);
	if (l->changed)
		l->changed = false;

	for (i = l->idx - extralines; i <= l->idx + extralines; i++) {
		if (i > -1 && i < l->count) {
			lines[j] = l->lines[i].line;
			sizes[j] = l->lines[i].size;
		} else
			lines[j] = NULL;
		++j;
	}
	xrKernelSignalSema(l->sema, 1);

	return true;
}

extern bool lyric_check_changed(p_lyric l)
{
	if (l == NULL || !l->succ)
		return false;
	xrKernelWaitSema(l->sema, 1, NULL);
	if (l->changed) {
		l->changed = false;
		return true;
	}
	xrKernelSignalSema(l->sema, 1);
	return false;
}

void lyric_decode(const char *lrcsrc, char *lrcdst, dword * size)
{
	t_conf_encode enc = config.lyricencode;

	switch (enc) {
		case conf_encode_gbk:
			strncpy(lrcdst, lrcsrc, *size);
			break;
		case conf_encode_big5:
			{
				int ilen = strnlen((const char *) lrcsrc, *size);
				int i = 0;

				i = charsets_bg5hk2cjk((const byte *) lrcsrc, ilen,
									   (byte *) lrcdst, *size);
				lrcdst[i] = 0;
			}
			break;
		case conf_encode_sjis:
			{
				byte *targ = NULL;

				charsets_sjis_conv((const byte *) lrcsrc,
								   (byte **) & targ, (dword *) size);
				strncpy(lrcdst, (const char *) targ, *size);
				free(targ);
			}
			break;
		case conf_encode_utf8:
			{
				int i = 0, j = 0, l = strnlen((const char *) lrcsrc, *size);

				while (i < l) {
					ucs4_t u = 0x1FFF;
					int p = utf8_mbtowc(&u, (const byte *) lrcsrc + i,
										l - i);

					if (p < 0)
						break;
					if (u > 0xFFFF)
						u = 0x1FFF;
					j += gbk_wctomb((byte *) lrcdst + j, u, 2);
					i += p;
				}
				lrcdst[j] = 0;
				*size = j;
			}
			break;
		default:
			strncpy(lrcdst, lrcsrc, *size);
			break;
	}
}

#endif
