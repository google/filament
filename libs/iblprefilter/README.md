# IBL Prefilter

This library can be used to generate the `reflections` texture used by filament's `IndirectLight` 
class. It is similar to the `cmgen` tool except that all computations are performed on the GPU and
are therefore significantly faster. `cmgen` however offers more functionalities.

`IBL Prefilter` is designed entirely as a client of filament, that is, it only uses filament
public APIs.

## Library and headers

The library is called `libfilament-iblprefilter.a` and its public headers can be found in 
`<filament-iblprefilter/*.h>`.

## Performance

Expect a total processing time of about 100ms to 300ms for a 5-levels 256 x 256 cubemap with 1024
samples.

## Example

```c++
#include <filament/Engine.h>
#include <filament-iblprefilter/IBLPrefilterContext.h>

using namespace filament;

Engine* engine = Engine::create();

// create an IBLPrefilterContext, keep it around if several cubemap will be processed.
IBLPrefilterContext context(engine);

// create the specular (reflections) filter. This operation generates the kernel, so it's important
// to keep it around if it will be reused for several cubemaps.
IBLPrefilterContext::SpecularFilter filter(context);

// launch the heaver computation. Expect 100-100ms on the GPU.
Texture* texture = filter(environment_cubemap);

IndirectLight* indirectLight = IndirectLight::Builder()
    .reflections(texture)
    .build(engine);
```
