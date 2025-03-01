#version 460

in vec3 Position;
in vec3 Normal;

layout (location = 0) out vec4 FragColor;

struct SpotLightInfo{
    vec3 Position;
    vec3 La;
    vec3 L;
    vec3 Direction;
    float Exponent;
    float Cutoff;
};

struct MaterialInfo{
    vec3 Kd;
    vec3 Ka;
    vec3 Ks;
    float Shininess;
};

uniform SpotLightInfo Spotlight;
uniform MaterialInfo Material;

vec3 BlinnPhongModel(vec3 pos, vec3 n){
    vec3 diffuse = vec3(0), spec = vec3(0);
    vec3 ambient = Spotlight.La * Material.Ka;

    vec3 s = normalize(Spotlight.Position - pos);

    float cosAng = dot(-s, normalize(Spotlight.Direction));
    float angle = acos(cosAng);
    float spotScale;

    if (angle >= 0.0 && angle < Spotlight.Cutoff){
        spotScale = pow(cosAng, Spotlight.Exponent);
        float sDotN = max(dot(s,n), 0.0);
        diffuse = Material.Kd * sDotN;

        if(sDotN > 0.0){
            vec3 v = normalize(-pos.xyz);
            vec3 h = normalize(v + s);
            spec = Material.Ks * pow(max(dot(h, n), 0.0), Material.Shininess);
        }
    }

    return ambient + spotScale * (diffuse + spec) * Spotlight.L;
}

void main() {
    FragColor = vec4(BlinnPhongModel(Position, normalize(Normal)), 1.0);
}