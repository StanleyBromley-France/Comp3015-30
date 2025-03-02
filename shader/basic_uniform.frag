#version 460

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;
in vec3 Tangent;
in vec3 Bitangent;
in vec3 VecPos;

layout (binding = 0) uniform sampler2D Tex1;
layout (binding = 1) uniform sampler2D Tex2;
layout (binding = 2) uniform sampler2D NormalTex;

layout (binding = 3) uniform samplerCube SkyBoxTex;


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
uniform bool IsTextured;
uniform bool IsToonLighting;

uniform bool IsSkyBox;

const int levels = 3;
const float scaleFactor = 1.0 / levels;

vec3 BlinnPhongModel(vec3 pos, vec3 n){

    // determines whether tex or mat data should be used
    vec3 ambientBase, diffuseBase;
    if (IsTextured) { // tex data
        vec4 texColor1 =  texture(Tex1, TexCoord);
        vec4 texColor2 =  texture(Tex2, TexCoord);

        vec3 texColor = mix(texColor1.rgb, texColor2.rgb, 0.0);
        ambientBase = texColor;
        diffuseBase = texColor;
    } else { // mat data
        ambientBase = Material.Ka;
        diffuseBase = Material.Kd;
    }

    vec3 diffuse = vec3(0), spec = vec3(0);
    vec3 ambient = Spotlight.La * ambientBase;

    vec3 s = normalize(Spotlight.Position - pos);

    float cosAng = dot(-s, normalize(Spotlight.Direction));
    float angle = acos(cosAng);
    float spotScale;

    if (angle >= 0.0 && angle < Spotlight.Cutoff){
        spotScale = pow(cosAng, Spotlight.Exponent);
        float sDotN = max(dot(s,n), 0.0);

        if(IsToonLighting)
        diffuse = diffuseBase * floor(sDotN * levels) * scaleFactor;
        else
        diffuse = diffuseBase * sDotN;

        if(sDotN > 0.0){
            vec3 v = normalize(-pos.xyz);
            vec3 h = normalize(v + s);
            spec = Material.Ks * pow(max(dot(h, n), 0.0), Material.Shininess);
        }
    }

    return ambient + spotScale * (diffuse + spec) * Spotlight.L;
}

void main() {

    if(IsSkyBox){
    
        vec3 skyTexColor = texture(SkyBoxTex, normalize(VecPos)).rbg;
        skyTexColor = pow(skyTexColor, vec3(1.0 / 2.2));
        FragColor = vec4(skyTexColor, 1.0);
        return;
    }

    // Calculate the TBN matrix
    vec3 T = normalize(Tangent);
    vec3 B = normalize(Bitangent);
    vec3 N = normalize(Normal);
    mat3 TBN = mat3(T, B, N);

    // Sample the normal map and transform to world space
    vec3 normalMap = texture(NormalTex, TexCoord).rgb;
    normalMap = normalize(normalMap * 2.0 - 1.0); // Convert from [0,1] to [-1,1]
    vec3 worldNormal = normalize(TBN * normalMap);

    if(IsTextured)
    // Use the transformed normal in the lighting calculation
        FragColor = vec4(BlinnPhongModel(Position, worldNormal), 1.0);
    else
        FragColor = vec4(BlinnPhongModel(Position, Normal), 1.0);


}