# Introduction

There are many situations in which it's useful for WebGL applications to
transform shaders in various ways. ANGLE's shader translator can be used for
this purpose: compiling it with [Emscripten](http://emscripten.org/) allows it
to be invoked from a web page. This wiki page provides some preliminary details
about how to do this.

# Details

Pull top of tree ANGLE.

Install the Emscripten toolchain per the [instructions](http://emscripten.org/).

Symlink (preferred) or copy the ANGLE directory into ...emsdk/emscripten/master.

Put a shader to compile into a file (named with .vert or .frag suffix) in the
same directory. For example, put the following shader from the [WebGL Aquarium]
(http://webglsamples.org/aquarium/aquarium.html) into `aq-fish-nm.frag`:

```
precision mediump float;
uniform vec4 lightColor;
varying vec4 v_position;
varying vec2 v_texCoord;
varying vec3 v_tangent;  // #normalMap
varying vec3 v_binormal;  // #normalMap
varying vec3 v_normal;
varying vec3 v_surfaceToLight;
varying vec3 v_surfaceToView;

uniform vec4 ambient;
uniform sampler2D diffuse;
uniform vec4 specular;
uniform sampler2D normalMap;  // #normalMap
uniform float shininess;
uniform float specularFactor;
// #fogUniforms

vec4 lit(float l ,float h, float m) {
  return vec4(1.0,
              max(l, 0.0),
              (l > 0.0) ? pow(max(0.0, h), m) : 0.0,
              1.0);
}
void main() {
  vec4 diffuseColor = texture2D(diffuse, v_texCoord);
  mat3 tangentToWorld = mat3(v_tangent,  // #normalMap
                             v_binormal,  // #normalMap
                             v_normal);  // #normalMap
  vec4 normalSpec = texture2D(normalMap, v_texCoord.xy);  // #normalMap
  vec3 tangentNormal = normalSpec.xyz - vec3(0.5, 0.5, 0.5);  // #normalMap
  tangentNormal = normalize(tangentNormal + vec3(0, 0, 2));  // #normalMap
  vec3 normal = (tangentToWorld * tangentNormal);  // #normalMap
  normal = normalize(normal);  // #normalMap
  vec3 surfaceToLight = normalize(v_surfaceToLight);
  vec3 surfaceToView = normalize(v_surfaceToView);
  vec3 halfVector = normalize(surfaceToLight + surfaceToView);
  vec4 litR = lit(dot(normal, surfaceToLight),
                    dot(normal, halfVector), shininess);
  vec4 outColor = vec4(
    (lightColor * (diffuseColor * litR.y + diffuseColor * ambient +
                  specular * litR.z * specularFactor * normalSpec.a)).rgb,
      diffuseColor.a);
  // #fogCode
  gl_FragColor = outColor;
}
```

Compile the shader translator, the translator sample, and the shader all
together:

```
./emcc -Iangle/include -Iangle/src angle/samples/translator/translator.cpp angle/src/compiler/preprocessor/*.cpp angle/src/compiler/translator/*.cpp angle/src/compiler/translator/timing/*.cpp angle/src/compiler/translator/depgraph/*.cpp angle/src/third_party/compiler/*.cpp angle/src/common/*.cpp -o translator.html --preload-file aq-fish-nm.frag -s NO_EXIT_RUNTIME=1
```

Serve up the resulting translator.html via `python -m SimpleHTTPServer`.
Navigate the browser to localhost:8000.

The translator sample will run, displaying its output into the text area on the
page. Since it isn't receiving any input, it simply outputs a help message and
exits.

To invoke the translator again, processing the shader we included along with the
source code, open the JavaScript console and type:

```
Module['callMain'](['-s=w', '-u', 'aq-fish-nm.frag'])
```

The active uniforms and their types will be printed to the text area after the
translator sample processes the shader.

# Issues and Next Steps

It's clearly not useful to have to compile the shader in to the
Emscripten-translated executable. It would be helpful to define a simple wrapper
function which can easily be called from JavaScript and which defines enough
parameters to pass in a shader as a string, transform it somehow or compile it
to another language target, and return the compiled result (or other
information). A simple JavaScript library that wraps all of the interactions
with the Emscripten binary would be useful.

It's not feasible to interact with the translator's data structures, nor
traverse the AST from JavaScript. The code that operates upon the shader must be
written in C++ and compiled in to the shader translator.

emcc should be integrated better with ANGLE's build system.
