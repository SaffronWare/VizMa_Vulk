#version 450

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

float terrain(vec3 p)
{
    return abs(p.y - noise_value_plane(0, p.xz));
}

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uv;

const float NORMAL_EPSILON = 0.01;
const float MARCH_EPSILON = 0.0001;
const float MAX_MARCH_DIST = 1000;
const int MAX_NUM_MARCHES = 10000;
const float MARCH_COEFF = 0.02;


vec3 camera_position = vec3(0.0, 0.0, -3.0);
float focal = 2.0;

vec3 front = vec3(0.0, 0.0, 1.0);
vec3 right = vec3(1.0, 0.0, 0.0);
vec3 up = vec3(0.0, 1.0, 1.0);


struct HitInfo 
{
    vec3 position;
    vec3 normal;
    int matID;
    bool hit;
    float dist;
    vec3 rd;
};

struct Ray {
    vec3 rp;
    vec3 rd;
};

HitInfo march_ray(vec3 rp) {
    HitInfo hit;

    hit.dist = length(rp - vec3(0,0,3)) - 1.0f;
    hit.matID = 0;
    
    float d = terrain(rp+vec3(0,-1,0));
    if (d < hit.dist)
    {
        hit.dist = d;
        hit.matID = 1;
    }

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

    return computed_normal;
}

vec3 light_dir = normalize(vec3(0.3f, -1.0f, -0.4f));
float ambient = 0.2f;
float diffuse = 1.0f;
float norm_shade(vec3 norm)
{
    return max(ambient, diffuse * dot(norm,light_dir));
}

vec4 lerp(vec4 a, vec4 b, float t)
{
    return a * (1-t) + b*t;
}

vec4 skylow = vec4(0.9, 0.9, 1.0, 1.0);
vec4 skyhigh =  vec4(0.4, 0.6, 1.0, 1.0);
vec4 sky(vec3 dir)
{
    float t = clamp(abs(acos(length(vec2(dir.x, dir.z)))),0,1) / acos(0);
    return lerp(skylow, skyhigh, abs(6*t*t));

}

vec4 get_shade(HitInfo info)
{
    return (info.hit) ? vec4(1.0) * norm_shade(info.normal) : sky(info.rd);
}

void main() {
    Ray ray;
    ray.rd = normalize(right * uv.x + up * uv.y + front * focal);
    ray.rp = camera_position;

    HitInfo hit;
    hit.rd = ray.rd;

    outColor = vec4(0.6f, 0.7f, 1.0f,1.0f);

    for (int i = 0; i < MAX_NUM_MARCHES; i++)
    {
        hit = march_ray(ray.rp);

        if (hit.hit)
        {
            
            hit.normal = normal(ray.rp);
            if (dot(hit.normal, ray.rd) > 0)
            {
                hit.normal = -hit.normal;
            }
            outColor = get_shade(hit);
            break;
        }
        else if (hit.dist > MAX_MARCH_DIST)
        {
            break;
        }
        
        ray.rp += hit.dist * ray.rd * MARCH_COEFF;

    }

    if (!hit.hit)
    {
        hit.rd = ray.rd;
        outColor = get_shade(hit);
    }
}