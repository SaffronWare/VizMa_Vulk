#version 450

const float MARCH_EPSILON = 0.0001;
const float MAX_MARCH_DIST = 1000;
const int MAX_NUM_MARCHES = 100;


vec2 positions[3] = vec2[](
    vec2(0, 3),
    vec2(-2, -1),
    vec2(2, -1)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 uv;

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


HitInfo march_ray(Ray ray) {
    HitInfo hit;

    hit.dist = length(ray.rp - vec3(0,0,3));
    hit.matID = 0;

    hit.hit = (abs(hit.dist) < MARCH_EPSILON);
    return hit;
};

vec3 normal(Ray ray)
{
    vec3 computed_normal;

    return computed_normal;
}

vec4 get_shade(HitInfo info)
{
    return (info.hit) ? vec4(0.0) : vec4(1.0);
};

void main() {
    Ray ray;
    ray.rd = normalize(right * uv.x + up * uv.y + front * focal);
    ray.rp = camera_position;

    HitInfo hit;

    for (int i = 0; i < MAX_NUM_MARCHES; i++)
    {
        hit = march_ray(Ray ray);
    };
    

    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    fragColor = colors[gl_VertexIndex];
    uv = positions[gl_VertexIndex];
}
