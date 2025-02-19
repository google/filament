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
} _9;

layout(location = 0) out highp vec4 _GLF_color;
int map[256];
highp mat2x4 _558 = mat2x4(vec4(0.0), vec4(0.0));

void main()
{
    int _564 = 256 - 14;
    int _566 = -_564;
    highp vec2 pos = gl_FragCoord.xy / _9.resolution;
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
        bool _77 = p.x > 0;
        bool _92;
        if (_77)
        {
            _92 = map[(p.x - 2) + (p.y * 16)] == 0;
        }
        else
        {
            _92 = _77;
        }
        if (_92)
        {
            directions++;
        }
        bool _99 = p.y > 0;
        bool _112;
        if (_99)
        {
            _112 = map[p.x + ((p.y - 2) * 16)] == 0;
        }
        else
        {
            _112 = _99;
        }
        if (_112)
        {
            directions++;
        }
        bool _120 = p.x < 14;
        bool _133;
        if (_120)
        {
            _133 = map[(p.x + 2) + (p.y * 16)] == 0;
        }
        else
        {
            _133 = _120;
        }
        if (_133)
        {
            directions++;
        }
        int _594 = 256 - _566;
        bool _140 = p.y < 14;
        bool _153;
        if (_140)
        {
            _153 = map[p.x + ((p.y + 2) * 16)] == 0;
        }
        else
        {
            _153 = _140;
        }
        if (_153)
        {
            directions++;
        }
        if (directions == 0)
        {
            canwalk = false;
            i = 0;
            for (;;)
            {
                int _168 = i;
                if (_168 < 8)
                {
                    int j = 0;
                    _558 = mat2x4(vec4(0.0), vec4(0.0));
                    if (false)
                    {
                        int _198 = i;
                        i = _198 + 1;
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
                        int _198 = i;
                        i = _198 + 1;
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
            bool _216 = d >= 0;
            bool _222;
            if (_216)
            {
                _222 = p.x > 0;
            }
            else
            {
                _222 = _216;
            }
            bool _235;
            if (_222)
            {
                _235 = map[(p.x - 2) + (p.y * 16)] == 0;
            }
            else
            {
                _235 = _222;
            }
            if (_235)
            {
                d--;
                map[p.x + (p.y * 16)] = 1;
                map[(p.x - 1) + (p.y * 16)] = 1;
                map[(p.x - 2) + (p.y * 16)] = 1;
                p.x -= 2;
            }
            bool _268 = d >= 0;
            bool _274;
            if (_268)
            {
                _274 = p.y > 0;
            }
            else
            {
                _274 = _268;
            }
            bool _287;
            if (_274)
            {
                _287 = map[p.x + ((p.y - 2) * 16)] == 0;
            }
            else
            {
                _287 = _274;
            }
            if (_287)
            {
                d--;
                map[p.x + (p.y * 16)] = 1;
                map[p.x + ((p.y - 1) * 16)] = 1;
                map[p.x + ((p.y - 2) * 16)] = 1;
                p.y -= 2;
            }
            bool _320 = d >= 0;
            bool _326;
            if (_320)
            {
                _326 = p.x < 14;
            }
            else
            {
                _326 = _320;
            }
            bool _339;
            if (_326)
            {
                _339 = map[(p.x + 2) + (p.y * 16)] == 0;
            }
            else
            {
                _339 = _326;
            }
            if (_339)
            {
                d--;
                map[p.x + (p.y * 16)] = 1;
                map[(p.x + 1) + (p.y * 16)] = 1;
                map[(p.x + 2) + (p.y * 16)] = 1;
                p.x += 2;
            }
            bool _372 = d >= 0;
            bool _378;
            if (_372)
            {
                _378 = p.y < 14;
            }
            else
            {
                _378 = _372;
            }
            bool _391;
            if (_378)
            {
                _391 = map[p.x + ((p.y + 2) * 16)] == 0;
            }
            else
            {
                _391 = _378;
            }
            if (_391)
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

