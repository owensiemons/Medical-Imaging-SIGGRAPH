#shader VERTEX
#version 420 core

layout(location = 0) in vec3 position;

void main() {
    gl_Position = vec4(position, 1.0);
}



#shader FRAGMENT
#version 420 core

layout(std140, binding = 0) uniform uniforms {
    float screen_width;
    float screen_height;
};

uniform sampler3D tex0;

out vec4 frag_color;

struct Ray {
    vec3 ro;
    vec3 rd;
};

struct AABB {
    vec3 min;
    vec3 max;
};

bool intersectBox(Ray r, AABB aabb, out float t0, out float t1) {
    vec3 invR = 1.0 / r.rd;
    vec3 tbot = invR * (aabb.min-r.ro);
    vec3 ttop = invR * (aabb.max-r.ro);
    vec3 tmin = min(ttop, tbot);
    vec3 tmax = max(ttop, tbot);
    vec2 t = max(tmin.xx, tmin.yz);
    t0 = max(t.x, t.y);
    t = min(tmax.xx, tmax.yz);
    t1 = min(t.x, t.y);
    return t0 <= t1;
}

void main() {
    vec2 resolution = vec2(screen_width, screen_height);
    vec2 uv = (gl_FragCoord.xy / resolution) * 2.0 - 1.0;
    vec3 ro = vec3(0, 0, -2.75);
    float aspect = screen_width / screen_height;

    float stepSize = 0.01;

    vec3 rd = normalize(vec3(uv.x * aspect, uv.y, 1.0));
    Ray mainRay = Ray(ro, rd);
    AABB aabb = AABB(vec3(-1.0), vec3(+1.0));

    float tnear, tfar;

    intersectBox(mainRay, aabb, tnear, tfar);
    if (tnear < 0.0) { tnear = 0.0; }

    vec3 rayStart = mainRay.ro + mainRay.rd * tnear;
    vec3 rayStop = mainRay.ro + mainRay.rd * tfar;

    rayStart = 0.5 * (rayStart + 1.0);
    rayStop = 0.5 * (rayStop + 1.0);

    vec3 ray = rayStop - rayStart;
    float rayLen = length(ray);
    vec3 stepVec = stepSize * ray / rayLen;
    //todo: add jitter

    vec3 pos = rayStart;

    vec4 dst = vec4(0, 0, 0, 0);
    vec4 src = vec4(0, 0, 0, 0);

    float value = 0;

    while (rayLen > 0) {
        value = texture(tex0, pos).x;

        src = vec4(value);
        src.a *= 0.5;

        src.xyz *= src.a;

        dst = (1.0 - dst.a) * src + dst;

        if (dst.a >= 0.95f) { break; }

        rayLen -= stepSize;
        pos += stepVec;
    }

    frag_color = dst;
}