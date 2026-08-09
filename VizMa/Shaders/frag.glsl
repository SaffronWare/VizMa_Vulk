#version 450

vec2 rotate(vec2 p, float t)
{
    float c = cos(t);
    float s = sin(t);
    return vec2(p.x * c - p.y * s, p.x * s + p.y * c);
}

float chash11(float p) {
    p = fract(p * .1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}

float scubic(float t)
{
    return 3*t*t - 2 * t*t*t;
}


float noise_value_plane(int seed, vec2 p)
{
    float val = 0;

    float xt = fract(p.x);
    float i = p.x - xt;

    float yt = fract(p.y);
    float j = p.y - yt;

    float a = chash11(849 * seed + 384 * i +  1911 * j);
    i += 1;
    float b = chash11(849 * seed + 384 * i +  1911 * j);
    j += 1;
    float d = chash11(849 * seed + 384 * i +  1911 * j);
    i -= 1;
    float c = chash11(849 * seed + 384 * i +  1911 * j);
            
    val += a;
    val += (b-a) * scubic(xt);
    val += (c-a) * scubic(yt);
    val += (d-b-c+a) * scubic(xt) * scubic(yt);

    return val;
}

float terrain(vec2 xz, int quality)
{

    float ty = 0;
    //xz /= 1.4;
    float a = 1.0f;
    float theta = 0.0f;

    for (int i = 0; i < quality; i++)
    {
        ty += a * noise_value_plane(0, xz);
        theta = chash11(i) * acos(1);
        xz = rotate(xz,theta);
        a /= 2.3;
        xz *= 2.0;
    }

    return ty;
}

float terrainSDF(vec3 p, int quality)
{

    if (abs(p.y) > 2)
    {
        return abs(p.y) - 1;
    }

    return abs(p.y - terrain(p.xz, quality));
}

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uv;

const float NORMAL_EPSILON = 0.0001;
const float MARCH_EPSILON = 0.01;
const float MAX_MARCH_DIST = 5;
const int MAX_NUM_MARCHES = 10000;
const float MARCH_COEFF = 0.05;


vec3 camera_position = vec3(0.0, -0.0, -3.0);
float focal = 2.0;

vec3 front = vec3(0.0, 0.0, 1.0);
vec3 right = vec3(1.0, 0.0, 0.0);
vec3 up = vec3(0.0, 1.0, 0.0);


struct HitInfo 
{
    vec3 position;
    vec3 normal;
    int matID;
    bool hit;
    float dist;
    float accdist;
    vec3 rd;
};

struct Ray {
    vec3 rp;
    vec3 rd;
};

HitInfo march_ray(vec3 rp) {
    HitInfo hit;


    float tq = 12;
    tq *= float(exp(-0.04 * length(rp-camera_position)));
    float d = terrainSDF(rp+vec3(-1,0.75,0), int(tq));
 
    hit.dist = d;
    hit.matID = 0;
    

    hit.hit = (hit.dist < MARCH_EPSILON);
    return hit;
}

vec3 normal(vec3 pos)
{
    vec3 computed_normal;

    vec3 offx = pos + vec3(NORMAL_EPSILON, 0,0);
    vec3 offy = pos + vec3(0.0, NORMAL_EPSILON, 0.0);
    vec3 offz = pos + vec3(0.0, 0.0, NORMAL_EPSILON);

    HitInfo base = march_ray(pos);
    computed_normal.x = (march_ray(offx).dist - base.dist) / NORMAL_EPSILON;
    computed_normal.y = (march_ray(offy).dist - base.dist) / NORMAL_EPSILON;
    computed_normal.z = (march_ray(offz).dist - base.dist) / NORMAL_EPSILON;

    return normalize(computed_normal);
}

HitInfo getHit(Ray r) {
    HitInfo hit;
    hit.hit = false;
    float travel_dist = 0;
    hit.rd = r.rd;

    outColor = vec4(0.6f, 0.7f, 1.0f,1.0f);

    for (int i = 0; i < MAX_NUM_MARCHES; i++)
    {
        hit = march_ray(r.rp);
        

        if (hit.hit)
        {
            break;
        }
        else if (hit.dist > MAX_MARCH_DIST)
        {
            break;
        }
        travel_dist += hit.dist;
        r.rp += hit.dist * r.rd * MARCH_COEFF;

    }

    hit.normal = normal(r.rp);
    if (dot(hit.normal, r.rd) > 0)
    {
        hit.normal = -hit.normal;
    }
    hit.accdist = travel_dist;
    hit.position = r.rp;
    hit.rd = r.rd;

    return hit;
}


vec3 light_dir = normalize(vec3(0.0f, -1.0f, -0.0f));
float ambient = 0.2f;
float diffuse = 1.0f;
float norm_shade(HitInfo info)
{   
    Ray outray;
    outray.rd = light_dir;
    outray.rp = info.position + outray.rd * 2 * MARCH_EPSILON;
    
    if (!getHit(outray).hit)
    {
        return max(ambient, diffuse * dot(info.normal,light_dir));
    }
    return ambient;
}

vec4 lerp(vec4 a, vec4 b, float t)
{
    return a * (1-t) + b*t;
}

vec4 skylow = vec4(0.9, 0.9, 1.0, 1.0);
vec4 skyhigh =  vec4(0.1, 0.3, 1.0, 1.0);
float assumed_sky_height = 40;
vec4 sky(HitInfo info)
{
    vec4 def_sky;
    float t = clamp(10*abs(acos(length(vec2(info.rd.x, info.rd.z)))),0,1);
    def_sky = lerp(skylow, skyhigh, abs(t));

    
    float cm = 0.0;
    if (abs(info.rd.y) > MARCH_EPSILON)
    {
        vec2 projected = (camera_position + info.rd * (assumed_sky_height - camera_position.y) / info.rd.y).xz;
        projected /= 80;
        cm = terrain(projected, 12);
        t = smoothstep(-1, 1.5, cm);
        t *= t*t*t*t*t;
        def_sky = lerp(def_sky, vec4(1), t);
    }
    return def_sky;
    

}

vec4 foghit(vec4 c, float d)
{
    float t= 1-exp(-0.01 * d);
    return lerp(c, vec4(1.0), t);
}

vec4 fogsky(vec4 c, vec3 rd)
{
    float t = smoothstep(-1,1,acos(length(rd.xz)) / acos(0));
    return lerp(c, vec4(1.0), 1 - t);

}

vec4 get_mat_color(HitInfo info)
{
    vec4  c;
    switch (info.matID)
    {
        case 0:
            float t = smoothstep(-1, 0.3, info.position.y-0.1);
            return lerp(vec4(1.0), vec4(0.3,0.3,0.3,1), t);
            break;
    }

    return c;
}

vec4 get_shade(HitInfo info)
{
    vec4 col = (info.hit) ? get_mat_color(info) * norm_shade(info) : sky(info);
    if (info.hit)
    {
        return foghit(col, info.accdist);
    }
    return fogsky(col, info.rd);
}

void main() {
    Ray ray;
    ray.rd = normalize(right * uv.x + up * uv.y + front * focal);
    ray.rp = camera_position;

    HitInfo hit = getHit(ray);
    outColor = get_shade(hit);
    
}