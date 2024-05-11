/*
  Copyright (C) 1997-2018 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely.
*/
/*  Usage:
 *  Spacebar to begin recording a gesture on all touches.
 *  s to save all touches into "./gestureSave"
 *  l to load all touches from "./gestureSave"
 */

#include "SDL.h"
#include <stdlib.h> /* for exit() */

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#define WIDTH 640
#define HEIGHT 480
#define BPP 4

/* MUST BE A POWER OF 2! */
#define EVENT_BUF_SIZE 256


#define VERBOSE 0

static SDL_Event events[EVENT_BUF_SIZE];
static int eventWrite;


static int colors[7] = {0xFF,0xFF00,0xFF0000,0xFFFF00,0x00FFFF,0xFF00FF,0xFFFFFF};

SDL_Surface *screen;
SDL_Window *window;
SDL_bool quitting = SDL_FALSE;

typedef struct {
  float x,y;
} Point;

typedef struct {
  float ang,r;
  Point p;
} Knob;

static Knob knob;

void setpix(SDL_Surface *screen, float _x, float _y, unsigned int col)
{
  Uint32 *pixmem32;
  Uint32 colour;
  Uint8 r,g,b;
  int x = (int)_x;
  int y = (int)_y;
  float a;

  if(x < 0 || x >= screen->w) return;
  if(y < 0 || y >= screen->h) return;

  pixmem32 = (Uint32*) screen->pixels  + y*screen->pitch/BPP + x;

  SDL_memcpy(&colour,pixmem32,screen->format->BytesPerPixel);

  SDL_GetRGB(colour,screen->format,&r,&g,&b);
  /* r = 0;g = 0; b = 0; */
  a = (float)((col>>24)&0xFF);
  if(a == 0) a = 0xFF; /* Hack, to make things easier. */
  a /= 0xFF;
  r = (Uint8)(r*(1-a) + ((col>>16)&0xFF)*(a));
  g = (Uint8)(g*(1-a) + ((col>> 8)&0xFF)*(a));
  b = (Uint8)(b*(1-a) + ((col>> 0)&0xFF)*(a));
  colour = SDL_MapRGB( screen->format,r, g, b);


  *pixmem32 = colour;
}

void drawLine(SDL_Surface *screen,float x0,float y0,float x1,float y1,unsigned int col) {
  float t;
  for(t=0;t<1;t+=(float)(1.f/SDL_max(SDL_fabs(x0-x1),SDL_fabs(y0-y1))))
    setpix(screen,x1+t*(x0-x1),y1+t*(y0-y1),col);
}

void drawCircle(SDL_Surface* screen,float x,float y,float r,unsigned int c)
{
  float tx,ty;
  float xr;
  for(ty = (float)-SDL_fabs(r);ty <= (float)SDL_fabs((int)r);ty++) {
    xr = (float)SDL_sqrt(r*r - ty*ty);
    if(r > 0) { /* r > 0 ==> filled circle */
      for(tx=-xr+.5f;tx<=xr-.5;tx++) {
    setpix(screen,x+tx,y+ty,c);
      }
    }
    else {
      setpix(screen,x-xr+.5f,y+ty,c);
      setpix(screen,x+xr-.5f,y+ty,c);
    }
  }
}

void drawKnob(SDL_Surface* screen,Knob k) {
  drawCircle(screen,k.p.x*screen->w,k.p.y*screen->h,k.r*screen->w,0xFFFFFF);
  drawCircle(screen,(k.p.x+k.r/2*SDL_cosf(k.ang))*screen->w,
                (k.p.y+k.r/2*SDL_sinf(k.ang))*screen->h,k.r/4*screen->w,0);
}

void DrawScreen(SDL_Surface* screen, SDL_Window* window)
{
  int i;
#if 1
  SDL_FillRect(screen, NULL, 0);
#else
  int x, y;
  for(y = 0;y < screen->h;y++)
    for(x = 0;x < screen->w;x++)
    setpix(screen,(float)x,(float)y,((x%255)<<16) + ((y%255)<<8) + (x+y)%255);
#endif

  /* draw Touch History */
  for(i = eventWrite; i < eventWrite+EVENT_BUF_SIZE; ++i) {
    const SDL_Event *event = &events[i&(EVENT_BUF_SIZE-1)];
    float age = (float)(i - eventWrite) / EVENT_BUF_SIZE;
    float x, y;
    unsigned int c, col;

    if(event->type == SDL_FINGERMOTION ||
       event->type == SDL_FINGERDOWN ||
       event->type == SDL_FINGERUP) {
      x = event->tfinger.x;
      y = event->tfinger.y;

      /* draw the touch: */
      c = colors[event->tfinger.fingerId%7];
      col = ((unsigned int)(c*(.1+.85))) | (unsigned int)(0xFF*age)<<24;

      if(event->type == SDL_FINGERMOTION)
    drawCircle(screen,x*screen->w,y*screen->h,5,col);
      else if(event->type == SDL_FINGERDOWN)
    drawCircle(screen,x*screen->w,y*screen->h,-10,col);
    }
  }

  if(knob.p.x > 0)
    drawKnob(screen,knob);

  SDL_UpdateWindowSurface(window);
}

/* Returns a new SDL_Window if window is NULL or window if not. */
SDL_Window* initWindow(SDL_Window *window, int width,int height)
{
  if (!window) {
    window = SDL_CreateWindow("Gesture Test",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              width, height, SDL_WINDOW_RESIZABLE);
  }
  return window;
}

void loop()
{
    SDL_Event event;
    SDL_RWops *stream;

    while(SDL_PollEvent(&event))
    {
    /* Record _all_ events */
    events[eventWrite & (EVENT_BUF_SIZE-1)] = event;
    eventWrite++;

    switch (event.type)
      {
      case SDL_QUIT:
        quitting = SDL_TRUE;
        break;
      case SDL_KEYDOWN:
        switch (event.key.keysym.sym)
          {
              case SDLK_i:
              {
                  int i;
                  for (i = 0; i < SDL_GetNumTouchDevices(); ++i) {
                      SDL_TouchID id = SDL_GetTouchDevice(i);
                      SDL_Log("Fingers Down on device %"SDL_PRIs64": %d", id, SDL_GetNumTouchFingers(id));
                  }
                  break;
              }
          case SDLK_SPACE:
        SDL_RecordGesture(-1);
        break;
          case SDLK_s:
        stream = SDL_RWFromFile("gestureSave", "w");
        SDL_Log("Wrote %i templates", SDL_SaveAllDollarTemplates(stream));
        SDL_RWclose(stream);
        break;
          case SDLK_l:
        stream = SDL_RWFromFile("gestureSave", "r");
        SDL_Log("Loaded: %i", SDL_LoadDollarTemplates(-1, stream));
        SDL_RWclose(stream);
        break;
          case SDLK_ESCAPE:
        quitting = SDL_TRUE;
        break;
        }
        break;
      case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
          if (!(window = initWindow(window, event.window.data1, event.window.data2)) ||
              !(screen = SDL_GetWindowSurface(window)))
          {
        SDL_Quit();
        exit(1);
          }
            }
        break;
      case SDL_FINGERMOTION:
#if VERBOSE
        SDL_Log("Finger: %"SDL_PRIs64",x: %f, y: %f",event.tfinger.fingerId,
               event.tfinger.x,event.tfinger.y);
#endif
        break;
      case SDL_FINGERDOWN:
#if VERBOSE
        SDL_Log("Finger: %"SDL_PRIs64" down - x: %f, y: %f",
           event.tfinger.fingerId,event.tfinger.x,event.tfinger.y);
#endif
        break;
      case SDL_FINGERUP:
#if VERBOSE
        SDL_Log("Finger: %"SDL_PRIs64" up - x: %f, y: %f",
               event.tfinger.fingerId,event.tfinger.x,event.tfinger.y);
#endif
        break;
      case SDL_MULTIGESTURE:
#if VERBOSE
        SDL_Log("Multi Gesture: x = %f, y = %f, dAng = %f, dR = %f",
           event.mgesture.x,
           event.mgesture.y,
           event.mgesture.dTheta,
           event.mgesture.dDist);
        SDL_Log("MG: numDownTouch = %i",event.mgesture.numFingers);
#endif
        knob.p.x = event.mgesture.x;
        knob.p.y = event.mgesture.y;
        knob.ang += event.mgesture.dTheta;
        knob.r += event.mgesture.dDist;
        break;
      case SDL_DOLLARGESTURE:
        SDL_Log("Gesture %"SDL_PRIs64" performed, error: %f",
           event.dgesture.gestureId,
           event.dgesture.error);
        break;
      case SDL_DOLLARRECORD:
        SDL_Log("Recorded gesture: %"SDL_PRIs64"",event.dgesture.gestureId);
        break;
      }
    }
    DrawScreen(screen, window);

#ifdef __EMSCRIPTEN__
    if (quitting) {
        emscripten_cancel_main_loop();
    }
#endif
}

int main(int argc, char* argv[])
{
  window = NULL;
  screen = NULL;
  quitting = SDL_FALSE;

  /* Enable standard application logging */
  SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);

  /* gesture variables */
  knob.r = .1f;
  knob.ang = 0;

  if (SDL_Init(SDL_INIT_VIDEO) < 0 ) return 1;

  if (!(window = initWindow(window, WIDTH, HEIGHT)) ||
      !(screen = SDL_GetWindowSurface(window)))
  {
      SDL_Quit();
      return 1;
  }

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(loop, 0, 1);
#else
    while(!quitting) {
        loop();
    }
#endif

  SDL_Quit();
  return 0;
}

