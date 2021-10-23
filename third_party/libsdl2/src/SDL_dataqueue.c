/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2021 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "./SDL_internal.h"

#include "SDL.h"
#include "./SDL_dataqueue.h"

typedef struct SDL_DataQueuePacket
{
    size_t datalen;  /* bytes currently in use in this packet. */
    size_t startpos;  /* bytes currently consumed in this packet. */
    struct SDL_DataQueuePacket *next;  /* next item in linked list. */
    Uint8 data[SDL_VARIABLE_LENGTH_ARRAY];  /* packet data */
} SDL_DataQueuePacket;

struct SDL_DataQueue
{
    SDL_DataQueuePacket *head; /* device fed from here. */
    SDL_DataQueuePacket *tail; /* queue fills to here. */
    SDL_DataQueuePacket *pool; /* these are unused packets. */
    size_t packet_size;   /* size of new packets */
    size_t queued_bytes;  /* number of bytes of data in the queue. */
};

static void
SDL_FreeDataQueueList(SDL_DataQueuePacket *packet)
{
    while (packet) {
        SDL_DataQueuePacket *next = packet->next;
        SDL_free(packet);
        packet = next;
    }
}


/* this all expects that you managed thread safety elsewhere. */

SDL_DataQueue *
SDL_NewDataQueue(const size_t _packetlen, const size_t initialslack)
{
    SDL_DataQueue *queue = (SDL_DataQueue *) SDL_malloc(sizeof (SDL_DataQueue));

    if (!queue) {
        SDL_OutOfMemory();
        return NULL;
    } else {
        const size_t packetlen = _packetlen ? _packetlen : 1024;
        const size_t wantpackets = (initialslack + (packetlen - 1)) / packetlen;
        size_t i;

        SDL_zerop(queue);
        queue->packet_size = packetlen;

        for (i = 0; i < wantpackets; i++) {
            SDL_DataQueuePacket *packet = (SDL_DataQueuePacket *) SDL_malloc(sizeof (SDL_DataQueuePacket) + packetlen);
            if (packet) { /* don't care if this fails, we'll deal later. */
                packet->datalen = 0;
                packet->startpos = 0;
                packet->next = queue->pool;
                queue->pool = packet;
            }
        }
    }

    return queue;
}

void
SDL_FreeDataQueue(SDL_DataQueue *queue)
{
    if (queue) {
        SDL_FreeDataQueueList(queue->head);
        SDL_FreeDataQueueList(queue->pool);
        SDL_free(queue);
    }
}

void
SDL_ClearDataQueue(SDL_DataQueue *queue, const size_t slack)
{
    const size_t packet_size = queue ? queue->packet_size : 1;
    const size_t slackpackets = (slack + (packet_size-1)) / packet_size;
    SDL_DataQueuePacket *packet;
    SDL_DataQueuePacket *prev = NULL;
    size_t i;

    if (!queue) {
        return;
    }

    packet = queue->head;

    /* merge the available pool and the current queue into one list. */
    if (packet) {
        queue->tail->next = queue->pool;
    } else {
        packet = queue->pool;
    }

    /* Remove the queued packets from the device. */
    queue->tail = NULL;
    queue->head = NULL;
    queue->queued_bytes = 0;
    queue->pool = packet;

    /* Optionally keep some slack in the pool to reduce malloc pressure. */
    for (i = 0; packet && (i < slackpackets); i++) {
        prev = packet;
        packet = packet->next;
    }

    if (prev) {
        prev->next = NULL;
    } else {
        queue->pool = NULL;
    }

    SDL_FreeDataQueueList(packet);  /* free extra packets */
}

static SDL_DataQueuePacket *
AllocateDataQueuePacket(SDL_DataQueue *queue)
{
    SDL_DataQueuePacket *packet;

    SDL_assert(queue != NULL);

    packet = queue->pool;
    if (packet != NULL) {
        /* we have one available in the pool. */
        queue->pool = packet->next;
    } else {
        /* Have to allocate a new one! */
        packet = (SDL_DataQueuePacket *) SDL_malloc(sizeof (SDL_DataQueuePacket) + queue->packet_size);
        if (packet == NULL) {
            return NULL;
        }
    }

    packet->datalen = 0;
    packet->startpos = 0;
    packet->next = NULL;
                
    SDL_assert((queue->head != NULL) == (queue->queued_bytes != 0));
    if (queue->tail == NULL) {
        queue->head = packet;
    } else {
        queue->tail->next = packet;
    }
    queue->tail = packet;
    return packet;
}


int
SDL_WriteToDataQueue(SDL_DataQueue *queue, const void *_data, const size_t _len)
{
    size_t len = _len;
    const Uint8 *data = (const Uint8 *) _data;
    const size_t packet_size = queue ? queue->packet_size : 0;
    SDL_DataQueuePacket *orighead;
    SDL_DataQueuePacket *origtail;
    size_t origlen;
    size_t datalen;

    if (!queue) {
        return SDL_InvalidParamError("queue");
    }

    orighead = queue->head;
    origtail = queue->tail;
    origlen = origtail ? origtail->datalen : 0;

    while (len > 0) {
        SDL_DataQueuePacket *packet = queue->tail;
        SDL_assert(!packet || (packet->datalen <= packet_size));
        if (!packet || (packet->datalen >= packet_size)) {
            /* tail packet missing or completely full; we need a new packet. */
            packet = AllocateDataQueuePacket(queue);
            if (!packet) {
                /* uhoh, reset so we've queued nothing new, free what we can. */
                if (!origtail) {
                    packet = queue->head;  /* whole queue. */
                } else {
                    packet = origtail->next;  /* what we added to existing queue. */
                    origtail->next = NULL;
                    origtail->datalen = origlen;
                }
                queue->head = orighead;
                queue->tail = origtail;
                queue->pool = NULL;

                SDL_FreeDataQueueList(packet);  /* give back what we can. */
                return SDL_OutOfMemory();
            }
        }

        datalen = SDL_min(len, packet_size - packet->datalen);
        SDL_memcpy(packet->data + packet->datalen, data, datalen);
        data += datalen;
        len -= datalen;
        packet->datalen += datalen;
        queue->queued_bytes += datalen;
    }

    return 0;
}

size_t
SDL_PeekIntoDataQueue(SDL_DataQueue *queue, void *_buf, const size_t _len)
{
    size_t len = _len;
    Uint8 *buf = (Uint8 *) _buf;
    Uint8 *ptr = buf;
    SDL_DataQueuePacket *packet;

    if (!queue) {
        return 0;
    }

    for (packet = queue->head; len && packet; packet = packet->next) {
        const size_t avail = packet->datalen - packet->startpos;
        const size_t cpy = SDL_min(len, avail);
        SDL_assert(queue->queued_bytes >= avail);

        SDL_memcpy(ptr, packet->data + packet->startpos, cpy);
        ptr += cpy;
        len -= cpy;
    }

    return (size_t) (ptr - buf);
}

size_t
SDL_ReadFromDataQueue(SDL_DataQueue *queue, void *_buf, const size_t _len)
{
    size_t len = _len;
    Uint8 *buf = (Uint8 *) _buf;
    Uint8 *ptr = buf;
    SDL_DataQueuePacket *packet;

    if (!queue) {
        return 0;
    }

    while ((len > 0) && ((packet = queue->head) != NULL)) {
        const size_t avail = packet->datalen - packet->startpos;
        const size_t cpy = SDL_min(len, avail);
        SDL_assert(queue->queued_bytes >= avail);

        SDL_memcpy(ptr, packet->data + packet->startpos, cpy);
        packet->startpos += cpy;
        ptr += cpy;
        queue->queued_bytes -= cpy;
        len -= cpy;

        if (packet->startpos == packet->datalen) {  /* packet is done, put it in the pool. */
            queue->head = packet->next;
            SDL_assert((packet->next != NULL) || (packet == queue->tail));
            packet->next = queue->pool;
            queue->pool = packet;
        }
    }

    SDL_assert((queue->head != NULL) == (queue->queued_bytes != 0));

    if (queue->head == NULL) {
        queue->tail = NULL;  /* in case we drained the queue entirely. */
    }

    return (size_t) (ptr - buf);
}

size_t
SDL_CountDataQueue(SDL_DataQueue *queue)
{
    return queue ? queue->queued_bytes : 0;
}

void *
SDL_ReserveSpaceInDataQueue(SDL_DataQueue *queue, const size_t len)
{
    SDL_DataQueuePacket *packet;

    if (!queue) {
        SDL_InvalidParamError("queue");
        return NULL;
    } else if (len == 0) {
        SDL_InvalidParamError("len");
        return NULL;
    } else if (len > queue->packet_size) {
        SDL_SetError("len is larger than packet size");
        return NULL;
    }

    packet = queue->head;
    if (packet) {
        const size_t avail = queue->packet_size - packet->datalen;
        if (len <= avail) {  /* we can use the space at end of this packet. */
            void *retval = packet->data + packet->datalen;
            packet->datalen += len;
            queue->queued_bytes += len;
            return retval;
        }
    }

    /* Need a fresh packet. */
    packet = AllocateDataQueuePacket(queue);
    if (!packet) {
        SDL_OutOfMemory();
        return NULL;
    }

    packet->datalen = len;
    queue->queued_bytes += len;
    return packet->data;
}

/* vi: set ts=4 sw=4 expandtab: */

