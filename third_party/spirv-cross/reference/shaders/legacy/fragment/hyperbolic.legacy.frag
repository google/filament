#version 100
precision mediump float;
precision highp int;

varying highp float scalar;
varying highp vec3 vector;

void main()
{
    gl_FragData[0] = vec4(1.0);
    gl_FragData[0].w *= ((exp(scalar) - exp(-scalar)) * 0.5);
    gl_FragData[0].w *= ((exp(scalar) + exp(-scalar)) * 0.5);
    highp float _125 = exp(scalar);
    highp float _126 = exp(-scalar);
    gl_FragData[0].w *= ((_125 - _126) / (_125 + _126));
    gl_FragData[0].w *= log(scalar + sqrt(scalar * scalar + 1.0));
    gl_FragData[0].w *= log(scalar + sqrt(scalar * scalar - 1.0));
    gl_FragData[0].w *= (log((1.0 + scalar) / (1.0 - scalar)) * 0.5);
    highp vec4 _58 = gl_FragData[0];
    highp vec3 _60 = _58.xyz * ((exp(vector) - exp(-vector)) * 0.5);
    gl_FragData[0].x = _60.x;
    gl_FragData[0].y = _60.y;
    gl_FragData[0].z = _60.z;
    highp vec4 _72 = gl_FragData[0];
    highp vec3 _74 = _72.xyz * ((exp(vector) + exp(-vector)) * 0.5);
    gl_FragData[0].x = _74.x;
    gl_FragData[0].y = _74.y;
    gl_FragData[0].z = _74.z;
    highp vec3 _127 = exp(vector);
    highp vec3 _128 = exp(-vector);
    highp vec4 _83 = gl_FragData[0];
    highp vec3 _85 = _83.xyz * ((_127 - _128) / (_127 + _128));
    gl_FragData[0].x = _85.x;
    gl_FragData[0].y = _85.y;
    gl_FragData[0].z = _85.z;
    highp vec4 _94 = gl_FragData[0];
    highp vec3 _96 = _94.xyz * log(vector + sqrt(vector * vector + 1.0));
    gl_FragData[0].x = _96.x;
    gl_FragData[0].y = _96.y;
    gl_FragData[0].z = _96.z;
    highp vec4 _105 = gl_FragData[0];
    highp vec3 _107 = _105.xyz * log(vector + sqrt(vector * vector - 1.0));
    gl_FragData[0].x = _107.x;
    gl_FragData[0].y = _107.y;
    gl_FragData[0].z = _107.z;
    highp vec4 _116 = gl_FragData[0];
    highp vec3 _118 = _116.xyz * (log((1.0 + vector) / (1.0 - vector)) * 0.5);
    gl_FragData[0].x = _118.x;
    gl_FragData[0].y = _118.y;
    gl_FragData[0].z = _118.z;
}

