#version 460
#extension GL_EXT_ray_tracing : require

hitAttributeEXT vec3 attribs;

#include "../UtilsShaders/ShadersUtility.glsl"

struct RayPayload{
    vec3 color;
    vec3 colorLit;
    float distance;
    vec3 normal;
    vec3 origin;
    vec3 direction;
    vec3 sampleDir;
    float roughness;
    float reflectivity;
    bool shadow;
    bool emissive;
    bool isMiss;
};

layout(set = 0, binding = 7, std140) buffer Materials {
    MaterialData materials[];
};

layout(location = 0) rayPayloadEXT RayPayload rayPayload;


struct Sphere {
    vec3 center;
    float radius;
    uint matId;
};


bool intersectSphere(vec3 rayOrigin, vec3 rayDirection, vec3 sphereCenter, float sphereRadius, out float t) {
    vec3 oc = rayOrigin - sphereCenter;
    float a = dot(rayDirection, rayDirection);
    float b = 2.0 * dot(oc, rayDirection);
    float c = dot(oc, oc) - sphereRadius * sphereRadius;
    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0.0) return false;
    t = (-b - sqrt(discriminant)) / (2.0 * a);
    return true;
}

void main() {
    vec3 rayOrigin = gl_WorldRayOriginEXT;
    vec3 rayDirection = gl_WorldRayDirectionEXT;

    uint instanceID = gl_InstanceCustomIndexEXT;
    Sphere sphere;

    float t;
    if (intersectSphere(rayOrigin, rayDirection, sphere.center, sphere.radius, t)) {
        //reportIntersectionEXT(t, 0);
    }
}