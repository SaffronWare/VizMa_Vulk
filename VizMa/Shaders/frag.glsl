#version 450

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 uv;

const float NORMAL_EPSILON = 0.01;
const float MARCH_EPSILON = 0.0001;
const float MAX_MARCH_DIST = 1000;
const int MAX_NUM_MARCHES = 100;


vec3 camera_position = vec3(0.0, 0.0, -1.0);
float focal = 0.5;

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
};

struct Ray {
    vec3 rp;
    vec3 rd;
};

HitInfo march_ray(vec3 rp) {
    HitInfo hit;

    hit.dist = length(rp - vec3(0,0,3)) - 1.0f;
    hit.matID = 0;

    hit.hit = (abs(hit.dist) < MARCH_EPSILON);
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

vec4 get_shade(HitInfo info)
{
    return (info.hit) ? vec4(0.0) : vec4(1.0);
}

void main() {
    Ray ray;
    ray.rd = normalize(right * uv.x + up * uv.y + front * focal);
    ray.rp = camera_position;

    HitInfo hit;

    outColor = vec4(0.6f, 0.7f, 1.0f,1.0f);

    for (int i = 0; i < MAX_NUM_MARCHES; i++)
    {
        hit = march_ray(ray.rp);

        if (hit.hit)
        {
            outColor = get_shade(hit);
            break;
        }
        else if (hit.dist > MAX_MARCH_DIST)
        {
            break;
        }
        
        ray.rp += hit.dist * ray.rd;

    }
}