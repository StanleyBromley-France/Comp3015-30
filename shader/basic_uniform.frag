#version 460

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;
in vec3 Tangent;
in vec3 Bitangent;
in vec3 VecPos;

layout (binding = 1) uniform sampler2D Tex1;
layout (binding = 2) uniform sampler2D Tex2;
layout (binding = 3) uniform sampler2D NormalTex;

layout (binding = 4) uniform samplerCube SkyBoxTex;

layout (binding = 0) uniform sampler2D HdrTex;

layout (location = 0) out vec4 FragColor;
layout (location = 1) out vec3 HdrColor;

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

uniform int Pass;


uniform float AveLum;
uniform float Exposure = 0.35;
uniform float White = 0.928;
uniform bool DoToneMap = true;

uniform mat3 rgb2xyz = mat3(
    0.4142564, 0.2126729, 0.0193339,
    0.3575761, 0.7151522, 0.1191920,
    0.1804375, 0.0721750, 0.9503041
);

uniform mat3 xyz2rgb = mat3(
    3.2404542, -0.9692660, 0.0556434,
    -1.5371385, 1.8760108, -0.2040259,
    -0.4985314, 0.0415560, 1.0572252
);

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

// For non normal map lighting
vec4 pass1(){
    if(IsSkyBox){
    
        vec3 skyTexColor = texture(SkyBoxTex, normalize(VecPos)).rbg;
        skyTexColor = pow(skyTexColor, vec3(1.0 / 2.2));
        return vec4(skyTexColor, 1.0);
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
        return vec4(BlinnPhongModel(Position, worldNormal), 1.0);
    else
        return vec4(BlinnPhongModel(Position, Normal), 1.0);

}

// for hdr
vec4 pass2() 
{
    vec4 color = texture(HdrTex, TexCoord);
    vec3 xyzCol = rgb2xyz * vec3(color);
    float xyzSum = xyzCol.x + xyzCol.y + xyzCol.z;
    vec3 xyYCol = vec3( xyzCol.x / xyzSum, xyzCol.y / xyzSum, xyzCol.y);

    float L = (Exposure * xyYCol.z) / AveLum;
    L = (L * ( 1 + L / (White * White) )) / ( 1 + L );

    xyzCol.x = (L * xyYCol.x) / (xyYCol.y);
    xyzCol.y = L;
    xyzCol.z = (L * (1 - xyYCol.x - xyYCol.y))/xyYCol.y;


    if(DoToneMap)
        return vec4(xyz2rgb * xyzCol, 1.0);
    else
        return color;
}

void main() {

    if (Pass == 1) FragColor = pass1();
    else if (Pass == 2) FragColor = pass2();

}