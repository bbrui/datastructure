/*
 * utils.c
 *
 * Copyright (C) 2009 Hector Martin <hector@marcansoft.com>
 * Copyright (C) 2009 Nikias Bassen <nikias@gmx.li>
 * Copyright (c) 2013 Federico Mena Quintero
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 2.1 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>

#include "util.h"

//#include "log.h"
//#define util_error(...) usbmuxd_log(LL_ERROR, __VA_ARGS__)

void fdlist_create(struct fdlist *list)
{
	list->count = 0;
	list->capacity = 4;
	list->owners = malloc(sizeof(*list->owners) * list->capacity);
	list->fds = malloc(sizeof(*list->fds) * list->capacity);
}
void fdlist_add(struct fdlist *list, enum fdowner owner, int fd, short events)
{
	if(list->count == list->capacity) {
		list->capacity *= 2;
		list->owners = realloc(list->owners, sizeof(*list->owners) * list->capacity);
		list->fds = realloc(list->fds, sizeof(*list->fds) * list->capacity);
	}
	list->owners[list->count] = owner;
	list->fds[list->count].fd = fd;
	list->fds[list->count].events = events;
	list->fds[list->count].revents = 0;
	list->count++;
}

void fdlist_free(struct fdlist *list)
{
	list->count = 0;
	list->capacity = 0;
	free(list->owners);
	list->owners = NULL;
	free(list->fds);
	list->fds = NULL;
}

void fdlist_reset(struct fdlist *list)
{
	list->count = 0;
}

#define CAPACITY_STEP 8

void collection_init(struct collection *col)
{
	col->list = malloc(sizeof(void *) * CAPACITY_STEP);
	memset(col->list, 0, sizeof(void *) * CAPACITY_STEP);
	col->capacity = CAPACITY_STEP;
}

void collection_free(struct collection *col)
{
	free(col->list);
	col->list = NULL;
	col->capacity = 0;
}

void collection_add(struct collection *col, void *element)
{
	int i;
	for(i=0; i<col->capacity; i++) {
		if(!col->list[i]) {
			col->list[i] = element;
			return;
		}
	}
	col->list = realloc(col->list, sizeof(void*) * (col->capacity + CAPACITY_STEP));
	memset(&col->list[col->capacity], 0, sizeof(void *) * CAPACITY_STEP);
	col->list[col->capacity] = element;
	col->capacity += CAPACITY_STEP;
}

void collection_remove(struct collection *col, void *element)
{
	int i;
	for(i=0; i<col->capacity; i++) {
		if(col->list[i] == element) {
			col->list[i] = NULL;
			return;
		}
	}
	printf("collection_remove: element %p not present in collection %p (cap %d)", element, col, col->capacity);
}

int collection_count(struct collection *col)
{
	int i, cnt = 0;
	for(i=0; i<col->capacity; i++) {
		if(col->list[i])
			cnt++;
	}
	return cnt;
}

void collection_copy(struct collection *dest, struct collection *src)
{
	if (!dest || !src) return;
	dest->capacity = src->capacity;
	dest->list = malloc(sizeof(void*) * src->capacity);
	memcpy(dest->list, src->list, sizeof(void*) * src->capacity);
}

