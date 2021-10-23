# Dynamic API

Originally posted on Ryan's Google+ account.

Background:

- The Steam Runtime has (at least in theory) a really kick-ass build of SDL2, 
  but developers are shipping their own SDL2 with individual Steam games. 
  These games might stop getting updates, but a newer SDL2 might be needed later. 
  Certainly we'll always be fixing bugs in SDL, even if a new video target isn't 
  ever needed, and these fixes won't make it to a game shipping its own SDL.
- Even if we replace the SDL2 in those games with a compatible one, that is to 
  say, edit a developer's Steam depot (yuck!), there are developers that are 
  statically linking SDL2 that we can't do this for. We can't even force the 
  dynamic loader to ignore their SDL2 in this case, of course.
- If you don't ship an SDL2 with the game in some form, people that disabled the
  Steam Runtime, or just tried to run the game from the command line instead of 
  Steam might find themselves unable to run the game, due to a missing dependency.
- If you want to ship on non-Steam platforms like GOG or Humble Bundle, or target
  generic Linux boxes that may or may not have SDL2 installed, you have to ship 
  the library or risk a total failure to launch. So now, you might have to have 
  a non-Steam build plus a Steam build (that is, one with and one without SDL2 
  included), which is inconvenient if you could have had one universal build 
  that works everywhere.
- We like the zlib license, but the biggest complaint from the open source 
  community about the license change is the static linking. The LGPL forced this 
  as a legal, not technical issue, but zlib doesn't care. Even those that aren't
  concerned about the GNU freedoms found themselves solving the same problems: 
  swapping in a newer SDL to an older game often times can save the day. 
  Static linking stops this dead.

So here's what we did:

SDL now has, internally, a table of function pointers. So, this is what SDL_Init
now looks like:

```c
UInt32 SDL_Init(Uint32 flags)
{
    return jump_table.SDL_Init(flags);
}
```

Except that is all done with a bunch of macro magic so we don't have to maintain
every one of these.

What is jump_table.SDL_init()? Eventually, that's a function pointer of the real
SDL_Init() that you've been calling all this time. But at startup, it looks more 
like this:

```c
Uint32 SDL_Init_DEFAULT(Uint32 flags)
{
    SDL_InitDynamicAPI();
    return jump_table.SDL_Init(flags);
}
```

SDL_InitDynamicAPI() fills in jump_table with all the actual SDL function 
pointers, which means that this `_DEFAULT` function never gets called again. 
First call to any SDL function sets the whole thing up.

So you might be asking, what was the value in that? Isn't this what the operating
system's dynamic loader was supposed to do for us? Yes, but now we've got this 
level of indirection, we can do things like this:

```bash
export SDL_DYNAMIC_API=/my/actual/libSDL-2.0.so.0
./MyGameThatIsStaticallyLinkedToSDL2
```

And now, this game that is statically linked to SDL, can still be overridden 
with a newer, or better, SDL. The statically linked one will only be used as 
far as calling into the jump table in this case. But in cases where no override
is desired, the statically linked version will provide its own jump table, 
and everyone is happy.

So now:
- Developers can statically link SDL, and users can still replace it. 
  (We'd still rather you ship a shared library, though!)
- Developers can ship an SDL with their game, Valve can override it for, say, 
  new features on SteamOS, or distros can override it for their own needs, 
  but it'll also just work in the default case.
- Developers can ship the same package to everyone (Humble Bundle, GOG, etc), 
  and it'll do the right thing.
- End users (and Valve) can update a game's SDL in almost any case, 
  to keep abandoned games running on newer platforms.
- Everyone develops with SDL exactly as they have been doing all along. 
  Same headers, same ABI. Just get the latest version to enable this magic.


A little more about SDL_InitDynamicAPI():

Internally, InitAPI does some locking to make sure everything waits until a 
single thread initializes everything (although even SDL_CreateThread() goes 
through here before spinning a thread, too), and then decides if it should use
an external SDL library. If not, it sets up the jump table using the current 
SDL's function pointers (which might be statically linked into a program, or in
a shared library of its own). If so, it loads that library and looks for and 
calls a single function:

```c
SInt32 SDL_DYNAPI_entry(Uint32 version, void *table, Uint32 tablesize);
```

That function takes a version number (more on that in a moment), the address of
the jump table, and the size, in bytes, of the table. 
Now, we've got policy here: this table's layout never changes; new stuff gets 
added to the end. Therefore SDL_DYNAPI_entry() knows that it can provide all 
the needed functions if tablesize <= sizeof its own jump table. If tablesize is
bigger (say, SDL 2.0.4 is trying to load SDL 2.0.3), then we know to abort, but
if it's smaller, we know we can provide the entire API that the caller needs.

The version variable is a failsafe switch. 
Right now it's always 1. This number changes when there are major API changes 
(so we know if the tablesize might be smaller, or entries in it have changed). 
Right now SDL_DYNAPI_entry gives up if the version doesn't match, but it's not 
inconceivable to have a small dispatch library that only supplies this one 
function and loads different, otherwise-incompatible SDL libraries and has the
right one initialize the jump table based on the version. For something that 
must generically catch lots of different versions of SDL over time, like the 
Steam Client, this isn't a bad option.

Finally, I'm sure some people are reading this and thinking,
"I don't want that overhead in my project!"  

To which I would point out that the extra function call through the jump table 
probably wouldn't even show up in a profile, but lucky you: this can all be 
disabled. You can build SDL without this if you absolutely must, but we would 
encourage you not to do that. However, on heavily locked down platforms like 
iOS, or maybe when debugging, it makes sense to disable it. The way this is
designed in SDL, you just have to change one #define, and the entire system 
vaporizes out, and SDL functions exactly like it always did. Most of it is 
macro magic, so the system is contained to one C file and a few headers. 
However, this is on by default and you have to edit a header file to turn it 
off. Our hopes is that if we make it easy to disable, but not too easy, 
everyone will ultimately be able to get what they want, but we've gently 
nudged everyone towards what we think is the best solution.
