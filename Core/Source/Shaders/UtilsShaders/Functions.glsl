

#ifndef FUNCTIONS 
#define FUNCTIONS 

float Saturate(float x) {
    return clamp(x, 0.0, 1.0);
}

vec2 Saturate(vec2 x) {
    return clamp(x, 0.0, 1.0);
}

vec3 Saturate(vec3 x) {
    return clamp(x, 0.0, 1.0);
}

vec4 Saturate(vec4 x) {
    return clamp(x, 0.0, 1.0);
}
float Lerp(float x, float y, float t){
    return x * (1 - t) + (y * t);
}
vec2 Lerp(vec2 x, vec2 y, float t){
    return vec2(x.x * (1 - t) + (y.x * t),
                x.y * (1 - t) + (y.y * t));
}
vec3 Lerp(vec3 x, vec3 y, float t){
    return vec3(x.x * (1 - t) + (y.x * t),
                x.y * (1 - t) + (y.y * t),
                x.z * (1 - t) + (y.z * t));
}
void CreateOrthonormalBasis(in vec3 N, out vec3 T, out vec3 B)
{
    if (abs(N.z) > 0.999)
    {
        T = vec3(1.0, 0.0, 0.0);
    }
    else
    {
        T = normalize(cross(vec3(0.0, 0.0, 1.0), N));
    }
    B = cross(N, T);
}
float CosTheta(vec3 x, vec3 y){
    return dot(x,y);
}

float CosTheta(vec3 x){
    return x.z;
}
float Cos2Theta(vec3 w)
{
    return w.z * w.z;
}
float Sin2Theta(vec3 w)
{
    return max(1.0f - Cos2Theta(w), 0.0f);
}

float SinTheta(vec3 w)
{
    return sqrt(Sin2Theta(w));
}

float TanTheta(vec3 w)
{
    return SinTheta(w) / CosTheta(w);
}

float Tan2Theta(vec3 w)
{
    return Sin2Theta(w) / Cos2Theta(w);
}

float CosPhi(vec3 w)
{
    float sinTheta = SinTheta(w);
    return (sinTheta == 0) ? 1.0f : clamp(w.x / sinTheta, -1.0f, 1.0f);
}
float SinPhi(vec3 w)
{
    float sinTheta = SinTheta(w);
    return (sinTheta == 0) ? 1.0f : clamp(w.z / sinTheta, -1.0f, 1.0f);
}

float Cos2Phi(vec3 w)
{
    float cosPhi = CosPhi(w);
    return cosPhi * cosPhi;
}
float Sin2Phi(vec3 w)
{
    float sinPhi = SinPhi(w);
    return sinPhi * sinPhi;
}
#endif 