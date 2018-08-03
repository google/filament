/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

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

#include "../../SDL_internal.h"

#if SDL_VIDEO_DRIVER_WAYLAND

#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>

#include "SDL_stdinc.h"
#include "SDL_assert.h"
#include "../../core/unix/SDL_poll.h"

#include "SDL_waylandvideo.h"
#include "SDL_waylanddatamanager.h"

#include "SDL_waylanddyn.h"

static ssize_t
write_pipe(int fd, const void* buffer, size_t total_length, size_t *pos)
{
    int ready = 0;
    ssize_t bytes_written = 0;
    ssize_t length = total_length - *pos;

    sigset_t sig_set;
    sigset_t old_sig_set;
    struct timespec zerotime = {0};

    ready = SDL_IOReady(fd, SDL_TRUE, 1 * 1000);

    sigemptyset(&sig_set);
    sigaddset(&sig_set, SIGPIPE);  

    pthread_sigmask(SIG_BLOCK, &sig_set, &old_sig_set); 

    if (ready == 0) {
        bytes_written = SDL_SetError("Pipe timeout");
    } else if (ready < 0) {
        bytes_written = SDL_SetError("Pipe select error");
    } else {
        if (length > 0) {
            bytes_written = write(fd, (Uint8*)buffer + *pos, SDL_min(length, PIPE_BUF));
        }

        if (bytes_written > 0) {
            *pos += bytes_written;
        }
    }

    sigtimedwait(&sig_set, 0, &zerotime);
    pthread_sigmask(SIG_SETMASK, &old_sig_set, NULL);

    return bytes_written;
}

static ssize_t
read_pipe(int fd, void** buffer, size_t* total_length, SDL_bool null_terminate)
{
    int ready = 0;
    void* output_buffer = NULL;
    char temp[PIPE_BUF];
    size_t new_buffer_length = 0;
    ssize_t bytes_read = 0;
    size_t pos = 0;

    ready = SDL_IOReady(fd, SDL_FALSE, 1 * 1000);
  
    if (ready == 0) {
        bytes_read = SDL_SetError("Pipe timeout");
    } else if (ready < 0) {
        bytes_read = SDL_SetError("Pipe select error");
    } else {
        bytes_read = read(fd, temp, sizeof(temp));
    }

    if (bytes_read > 0) {
        pos = *total_length;
        *total_length += bytes_read;

        if (null_terminate == SDL_TRUE) {
            new_buffer_length = *total_length + 1;
        } else {
            new_buffer_length = *total_length;
        }

        if (*buffer == NULL) {
            output_buffer = SDL_malloc(new_buffer_length);
        } else {
            output_buffer = SDL_realloc(*buffer, new_buffer_length);
        }           
        
        if (output_buffer == NULL) {
            bytes_read = SDL_OutOfMemory();
        } else {
            SDL_memcpy((Uint8*)output_buffer + pos, temp, bytes_read);

            if (null_terminate == SDL_TRUE) {
                SDL_memset((Uint8*)output_buffer + (new_buffer_length - 1), 0, 1);
            }
            
            *buffer = output_buffer;
        }
    }

    return bytes_read;
}

#define MIME_LIST_SIZE 4

static const char* mime_conversion_list[MIME_LIST_SIZE][2] = {
    {"text/plain", TEXT_MIME},
    {"TEXT", TEXT_MIME},
    {"UTF8_STRING", TEXT_MIME},
    {"STRING", TEXT_MIME}
};

const char*
Wayland_convert_mime_type(const char *mime_type)
{
    const char *found = mime_type;

    size_t index = 0;

    for (index = 0; index < MIME_LIST_SIZE; ++index) {
        if (strcmp(mime_conversion_list[index][0], mime_type) == 0) {
            found = mime_conversion_list[index][1];
            break;
        }
    }
    
    return found;
}

static SDL_MimeDataList*
mime_data_list_find(struct wl_list* list, 
                    const char* mime_type)
{
    SDL_MimeDataList *found = NULL;

    SDL_MimeDataList *mime_list = NULL;
    wl_list_for_each(mime_list, list, link) { 
        if (strcmp(mime_list->mime_type, mime_type) == 0) {
            found = mime_list;
            break;
        }
    }    
    return found;
}

static int
mime_data_list_add(struct wl_list* list, 
                   const char* mime_type,
                   void* buffer, size_t length)
{
    int status = 0;
    size_t mime_type_length = 0;

    SDL_MimeDataList *mime_data = NULL;

    mime_data = mime_data_list_find(list, mime_type);

    if (mime_data == NULL) {
        mime_data = SDL_calloc(1, sizeof(*mime_data));
        if (mime_data == NULL) {
            status = SDL_OutOfMemory();
        } else {
            WAYLAND_wl_list_insert(list, &(mime_data->link));

            mime_type_length = strlen(mime_type) + 1;
            mime_data->mime_type = SDL_malloc(mime_type_length);
            if (mime_data->mime_type == NULL) {
                status = SDL_OutOfMemory();
            } else {
                SDL_memcpy(mime_data->mime_type, mime_type, mime_type_length);
            }
        }
    }
    
    if (mime_data != NULL && buffer != NULL && length > 0) {
        if (mime_data->data != NULL) {
            SDL_free(mime_data->data);
        }
        mime_data->data = buffer;
        mime_data->length = length;
    }

    return status;
}

static void
mime_data_list_free(struct wl_list *list)
{
    SDL_MimeDataList *mime_data = NULL; 
    SDL_MimeDataList *next = NULL;

    wl_list_for_each_safe(mime_data, next, list, link) {
        if (mime_data->data != NULL) {
            SDL_free(mime_data->data);
        }        
        if (mime_data->mime_type != NULL) {
            SDL_free(mime_data->mime_type);
        }
        SDL_free(mime_data);       
    } 
}

ssize_t 
Wayland_data_source_send(SDL_WaylandDataSource *source,  
                         const char *mime_type, int fd)
{
    size_t written_bytes = 0;
    ssize_t status = 0;
    SDL_MimeDataList *mime_data = NULL;
 
    mime_type = Wayland_convert_mime_type(mime_type);
    mime_data = mime_data_list_find(&source->mimes,
                                                      mime_type);

    if (mime_data == NULL || mime_data->data == NULL) {
        status = SDL_SetError("Invalid mime type");
        close(fd);
    } else {
        while (write_pipe(fd, mime_data->data, mime_data->length,
                          &written_bytes) > 0);
        close(fd);
        status = written_bytes;
    }
    return status;
}

int Wayland_data_source_add_data(SDL_WaylandDataSource *source,
                                 const char *mime_type,
                                 const void *buffer,
                                 size_t length) 
{
    int status = 0;
    if (length > 0) {
        void *internal_buffer = SDL_malloc(length);
        if (internal_buffer == NULL) {
            status = SDL_OutOfMemory();
        } else {
            SDL_memcpy(internal_buffer, buffer, length);
            status = mime_data_list_add(&source->mimes, mime_type, 
                                        internal_buffer, length);
        }
    }
    return status;
}

SDL_bool 
Wayland_data_source_has_mime(SDL_WaylandDataSource *source,
                             const char *mime_type)
{
    SDL_bool found = SDL_FALSE;

    if (source != NULL) {
        found = mime_data_list_find(&source->mimes, mime_type) != NULL;
    }
    return found;
}

void* 
Wayland_data_source_get_data(SDL_WaylandDataSource *source,
                             size_t *length, const char* mime_type,
                             SDL_bool null_terminate)
{
    SDL_MimeDataList *mime_data = NULL;
    void *buffer = NULL;
    *length = 0;

    if (source == NULL) {
        SDL_SetError("Invalid data source");
    } else {
        mime_data = mime_data_list_find(&source->mimes, mime_type);
        if (mime_data != NULL && mime_data->length > 0) {
            buffer = SDL_malloc(mime_data->length);
            if (buffer == NULL) {
                *length = SDL_OutOfMemory();
            } else {
                *length = mime_data->length;
                SDL_memcpy(buffer, mime_data->data, mime_data->length);
            }
       }
    }

    return buffer;
}

void
Wayland_data_source_destroy(SDL_WaylandDataSource *source)
{
    if (source != NULL) {
        wl_data_source_destroy(source->source);
        mime_data_list_free(&source->mimes);
        SDL_free(source);
    }
}

void* 
Wayland_data_offer_receive(SDL_WaylandDataOffer *offer,
                           size_t *length, const char* mime_type,
                           SDL_bool null_terminate)
{
    SDL_WaylandDataDevice *data_device = NULL;
 
    int pipefd[2];
    void *buffer = NULL;
    *length = 0;

    if (offer == NULL) {
        SDL_SetError("Invalid data offer");
    } else if ((data_device = offer->data_device) == NULL) {
        SDL_SetError("Data device not initialized");
    } else if (pipe2(pipefd, O_CLOEXEC|O_NONBLOCK) == -1) {
        SDL_SetError("Could not read pipe");
    } else {
        wl_data_offer_receive(offer->offer, mime_type, pipefd[1]);

        /* TODO: Needs pump and flush? */
        WAYLAND_wl_display_flush(data_device->video_data->display);

        close(pipefd[1]);
        
        while (read_pipe(pipefd[0], &buffer, length, null_terminate) > 0);
        close(pipefd[0]);
    }
    return buffer;
}

int 
Wayland_data_offer_add_mime(SDL_WaylandDataOffer *offer,
                            const char* mime_type)
{
    return mime_data_list_add(&offer->mimes, mime_type, NULL, 0);
}


SDL_bool 
Wayland_data_offer_has_mime(SDL_WaylandDataOffer *offer,
                            const char *mime_type)
{
    SDL_bool found = SDL_FALSE;

    if (offer != NULL) {
        found = mime_data_list_find(&offer->mimes, mime_type) != NULL;
    }
    return found;
}

void
Wayland_data_offer_destroy(SDL_WaylandDataOffer *offer)
{
    if (offer != NULL) {
        wl_data_offer_destroy(offer->offer);
        mime_data_list_free(&offer->mimes);
        SDL_free(offer);
    }
}

int
Wayland_data_device_clear_selection(SDL_WaylandDataDevice *data_device)
{
    int status = 0;

    if (data_device == NULL || data_device->data_device == NULL) {
        status = SDL_SetError("Invalid Data Device");
    } else if (data_device->selection_source != 0) {
        wl_data_device_set_selection(data_device->data_device, NULL, 0);
        data_device->selection_source = NULL;
    }
    return status;
}

int
Wayland_data_device_set_selection(SDL_WaylandDataDevice *data_device,
                                  SDL_WaylandDataSource *source)
{
    int status = 0;
    size_t num_offers = 0;
    size_t index = 0;

    if (data_device == NULL) {
        status = SDL_SetError("Invalid Data Device");
    } else if (source == NULL) {
        status = SDL_SetError("Invalid source");
    } else {
        SDL_MimeDataList *mime_data = NULL;

        wl_list_for_each(mime_data, &(source->mimes), link) {
            wl_data_source_offer(source->source,
                                 mime_data->mime_type); 

            /* TODO - Improve system for multiple mime types to same data */
            for (index = 0; index < MIME_LIST_SIZE; ++index) {
                if (strcmp(mime_conversion_list[index][1], mime_data->mime_type) == 0) {
                    wl_data_source_offer(source->source,
                                         mime_conversion_list[index][0]);
               }
            }
            /* */
 
            ++num_offers;
        } 

        if (num_offers == 0) {
            Wayland_data_device_clear_selection(data_device);
            status = SDL_SetError("No mime data");
        } else {
            /* Only set if there is a valid serial if not set it later */
            if (data_device->selection_serial != 0) {
                wl_data_device_set_selection(data_device->data_device,
                                             source->source,
                                             data_device->selection_serial); 
            }
            data_device->selection_source = source;
        }
    }

    return status;
}

int
Wayland_data_device_set_serial(SDL_WaylandDataDevice *data_device,
                               uint32_t serial)
{
    int status = -1;
    if (data_device != NULL) {
        status = 0;

        /* If there was no serial and there is a pending selection set it now. */
        if (data_device->selection_serial == 0
            && data_device->selection_source != NULL) {
            wl_data_device_set_selection(data_device->data_device,
                                         data_device->selection_source->source,
                                         serial); 
        }

        data_device->selection_serial = serial;
    }

    return status; 
}

#endif /* SDL_VIDEO_DRIVER_WAYLAND */

/* vi: set ts=4 sw=4 expandtab: */
