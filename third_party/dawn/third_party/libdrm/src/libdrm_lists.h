/**************************************************************************
 *
 * Copyright 2006 Tungsten Graphics, Inc., Bismarck, ND. USA.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE COPYRIGHT HOLDERS, AUTHORS AND/OR ITS SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 */

/*
 * List macros heavily inspired by the Linux kernel
 * list handling. No list looping yet.
 */

#include <stddef.h>

typedef struct _drmMMListHead
{
    struct _drmMMListHead *prev;
    struct _drmMMListHead *next;
} drmMMListHead;

#define DRMINITLISTHEAD(__item)		       \
  do{					       \
    (__item)->prev = (__item);		       \
    (__item)->next = (__item);		       \
  } while (0)

#define DRMLISTADD(__item, __list)		\
  do {						\
    (__item)->prev = (__list);			\
    (__item)->next = (__list)->next;		\
    (__list)->next->prev = (__item);		\
    (__list)->next = (__item);			\
  } while (0)

#define DRMLISTADDTAIL(__item, __list)		\
  do {						\
    (__item)->next = (__list);			\
    (__item)->prev = (__list)->prev;		\
    (__list)->prev->next = (__item);		\
    (__list)->prev = (__item);			\
  } while(0)

#define DRMLISTDEL(__item)			\
  do {						\
    (__item)->prev->next = (__item)->next;	\
    (__item)->next->prev = (__item)->prev;	\
  } while(0)

#define DRMLISTDELINIT(__item)			\
  do {						\
    (__item)->prev->next = (__item)->next;	\
    (__item)->next->prev = (__item)->prev;	\
    (__item)->next = (__item);			\
    (__item)->prev = (__item);			\
  } while(0)

#define DRMLISTENTRY(__type, __item, __field)   \
    ((__type *)(((char *) (__item)) - offsetof(__type, __field)))

#define DRMLISTEMPTY(__item) ((__item)->next == (__item))

#define DRMLISTSINGLE(__list) \
	(!DRMLISTEMPTY(__list) && ((__list)->next == (__list)->prev))

#define DRMLISTFOREACH(__item, __list)					\
	for ((__item) = (__list)->next;					\
	     (__item) != (__list); (__item) = (__item)->next)

#define DRMLISTFOREACHSAFE(__item, __temp, __list)			\
	for ((__item) = (__list)->next, (__temp) = (__item)->next;	\
	     (__item) != (__list);					\
	     (__item) = (__temp), (__temp) = (__item)->next)

#define DRMLISTFOREACHSAFEREVERSE(__item, __temp, __list)		\
	for ((__item) = (__list)->prev, (__temp) = (__item)->prev;	\
	     (__item) != (__list);					\
	     (__item) = (__temp), (__temp) = (__item)->prev)

#define DRMLISTFOREACHENTRY(__item, __list, __head)                            \
	for ((__item) = DRMLISTENTRY(__typeof__(*__item), (__list)->next, __head); \
	     &(__item)->__head != (__list);                                        \
	     (__item) = DRMLISTENTRY(__typeof__(*__item),                          \
	                             (__item)->__head.next, __head))

#define DRMLISTFOREACHENTRYSAFE(__item, __temp, __list, __head)                \
	for ((__item) = DRMLISTENTRY(__typeof__(*__item), (__list)->next, __head), \
	     (__temp) = DRMLISTENTRY(__typeof__(*__item),                          \
	                             (__item)->__head.next, __head);               \
	     &(__item)->__head != (__list);                                        \
	     (__item) = (__temp),                                                  \
	     (__temp) = DRMLISTENTRY(__typeof__(*__item),                          \
	                             (__temp)->__head.next, __head))

#define DRMLISTJOIN(__list, __join) if (!DRMLISTEMPTY(__list)) {	\
	(__list)->next->prev = (__join);				\
	(__list)->prev->next = (__join)->next;				\
	(__join)->next->prev = (__list)->prev;				\
	(__join)->next = (__list)->next;				\
}
