#version 320 es
#if defined(GL_EXT_control_flow_attributes)
#extension GL_EXT_control_flow_attributes : require
#define SPIRV_CROSS_FLATTEN [[flatten]]
#define SPIRV_CROSS_BRANCH [[dont_flatten]]
#define SPIRV_CROSS_UNROLL [[unroll]]
#define SPIRV_CROSS_LOOP [[dont_unroll]]
#else
#define SPIRV_CROSS_FLATTEN
#define SPIRV_CROSS_BRANCH
#define SPIRV_CROSS_UNROLL
#define SPIRV_CROSS_LOOP
#endif
precision mediump float;
precision highp int;

layout(binding = 0, std140) uniform buf0
{
    highp vec2 resolution;
} _7;

layout(location = 0) out highp vec4 _GLF_color;
int map[256];
highp mat2x4 _60 = mat2x4(vec4(0.0), vec4(0.0));

void main()
{
    int _65 = 256 - 14;
    int _68 = -_65;
    highp vec2 pos = gl_FragCoord.xy / _7.resolution;
    ivec2 ipos = ivec2(int(pos.x * 16.0), int(pos.y * 16.0));
    int i = 0;
    for (; i < 256; i++)
    {
        map[i] = 0;
    }
    ivec2 p = ivec2(0);
    int v = 0;
    bool canwalk = true;
    do
    {
        v++;
        int directions = 0;
        bool _98 = p.x > 0;
        bool _111;
        if (_98)
        {
            _111 = map[(p.x - 2) + (p.y * 16)] == 0;
        }
        else
        {
            _111 = _98;
        }
        if (_111)
        {
            directions++;
        }
        bool _118 = p.y > 0;
        bool _131;
        if (_118)
        {
            _131 = map[p.x + ((p.y - 2) * 16)] == 0;
        }
        else
        {
            _131 = _118;
        }
        if (_131)
        {
            directions++;
        }
        bool _138 = p.x < 14;
        bool _151;
        if (_138)
        {
            _151 = map[(p.x + 2) + (p.y * 16)] == 0;
        }
        else
        {
            _151 = _138;
        }
        if (_151)
        {
            directions++;
        }
        int _156 = 256 - _68;
        bool _159 = p.y < 14;
        bool _172;
        if (_159)
        {
            _172 = map[p.x + ((p.y + 2) * 16)] == 0;
        }
        else
        {
            _172 = _159;
        }
        if (_172)
        {
            directions++;
        }
        if (directions == 0)
        {
            canwalk = false;
            i = 0;
            for (;;)
            {
                int _186 = i;
                if (_186 < 8)
                {
                    int j = 0;
                    _60 = mat2x4(vec4(0.0), vec4(0.0));
                    if (false)
                    {
                        int _216 = i;
                        i = _216 + 1;
                        continue;
                    }
                    else
                    {
                        SPIRV_CROSS_UNROLL
                        for (; j < 8; j++)
                        {
                            if (map[(j * 2) + ((i * 2) * 16)] == 0)
                            {
                                p.x = j * 2;
                                p.y = i * 2;
                                canwalk = true;
                            }
                        }
                        int _216 = i;
                        i = _216 + 1;
                        continue;
                    }
                }
                else
                {
                    break;
                }
            }
            map[p.x + (p.y * 16)] = 1;
        }
        else
        {
            int d = v % directions;
            v += directions;
            bool _232 = d >= 0;
            bool _238;
            if (_232)
            {
                _238 = p.x > 0;
            }
            else
            {
                _238 = _232;
            }
            bool _251;
            if (_238)
            {
                _251 = map[(p.x - 2) + (p.y * 16)] == 0;
            }
            else
            {
                _251 = _238;
            }
            if (_251)
            {
                d--;
                map[p.x + (p.y * 16)] = 1;
                map[(p.x - 1) + (p.y * 16)] = 1;
                map[(p.x - 2) + (p.y * 16)] = 1;
                p.x -= 2;
            }
            bool _284 = d >= 0;
            bool _290;
            if (_284)
            {
                _290 = p.y > 0;
            }
            else
            {
                _290 = _284;
            }
            bool _303;
            if (_290)
            {
                _303 = map[p.x + ((p.y - 2) * 16)] == 0;
            }
            else
            {
                _303 = _290;
            }
            if (_303)
            {
                d--;
                map[p.x + (p.y * 16)] = 1;
                map[p.x + ((p.y - 1) * 16)] = 1;
                map[p.x + ((p.y - 2) * 16)] = 1;
                p.y -= 2;
            }
            bool _336 = d >= 0;
            bool _342;
            if (_336)
            {
                _342 = p.x < 14;
            }
            else
            {
                _342 = _336;
            }
            bool _355;
            if (_342)
            {
                _355 = map[(p.x + 2) + (p.y * 16)] == 0;
            }
            else
            {
                _355 = _342;
            }
            if (_355)
            {
                d--;
                map[p.x + (p.y * 16)] = 1;
                map[(p.x + 1) + (p.y * 16)] = 1;
                map[(p.x + 2) + (p.y * 16)] = 1;
                p.x += 2;
            }
            bool _388 = d >= 0;
            bool _394;
            if (_388)
            {
                _394 = p.y < 14;
            }
            else
            {
                _394 = _388;
            }
            bool _407;
            if (_394)
            {
                _407 = map[p.x + ((p.y + 2) * 16)] == 0;
            }
            else
            {
                _407 = _394;
            }
            if (_407)
            {
                d--;
                map[p.x + (p.y * 16)] = 1;
                map[p.x + ((p.y + 1) * 16)] = 1;
                map[p.x + ((p.y + 2) * 16)] = 1;
                p.y += 2;
            }
        }
        if (map[(ipos.y * 16) + ipos.x] == 1)
        {
            _GLF_color = vec4(1.0);
            return;
        }
    } while (canwalk);
    _GLF_color = vec4(0.0, 0.0, 0.0, 1.0);
}

