#version 460

in vec3 Position;
in vec3 Normal;
in vec2 TexCoord;
in vec3 Tangent;
in vec3 Bitangent;
in vec3 VecPos;

layout (binding = 0) uniform sampler2D HdrTex;
layout (binding = 1) uniform sampler2D BlurTex1;
layout (binding = 2) uniform sampler2D BlurTex2;

layout (binding = 3) uniform sampler2D Tex1;
layout (binding = 4) uniform sampler2D Tex2;
layout (binding = 5) uniform sampler2D NormalTex;

layout (binding = 6) uniform samplerCube SkyBoxTex;



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
uniform float mixLevel = 0.5;
uniform bool DoNormalMap = true;


uniform bool IsSkyBox;

uniform int Pass;


uniform float AveLum;
uniform float LumThresh;
uniform float PixOffset[10] = float[](0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0);
uniform float Weight[10];

uniform float Exposure = 0.15;
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

float luminance(vec3 colour)
{
    return colour.r * 0.2126 + colour.g * 0.7152 + colour.b * 0.0722;
}

const int levels = 3;
const float scaleFactor = 1.0 / levels;

vec3 BlinnPhongModel(vec3 pos, vec3 n){

    // determines whether tex or mat data should be used
    vec3 ambientBase, diffuseBase;
    if (IsTextured) { // tex data
        vec4 texColor1 =  texture(Tex1, TexCoord);
        vec4 texColor2 =  texture(Tex2, TexCoord);

        vec3 texColor = mix(texColor1.rgb, texColor2.rgb, mixLevel);
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
    vec3 worldNormal;

    // for normal map toggle
    if(DoNormalMap)
        worldNormal = normalize(TBN * normalMap);
    else
        worldNormal = Normal;

    if(IsTextured)
    // Use the transformed normal in the lighting calculation
        return vec4(BlinnPhongModel(Position, worldNormal), 1.0);
    else
        return vec4(BlinnPhongModel(Position, Normal), 1.0);

}

// Pass 2 is checking hdr texture sample against luminance threshold
vec4 pass2()
{
    vec4 val = texture(HdrTex, TexCoord);
    
    if(luminance(val.rgb) > LumThresh)
    {
        return val;
    }
    else
    {
        return vec4(0.0);
    }
}

vec4 pass3()
{
    float dy = 1.0 / (textureSize(BlurTex1, 0)).y;
    vec4 sum = texture(BlurTex1, TexCoord) * Weight[0];
    for(int i = 0; i < 10; i++)
    {
        sum += texture(BlurTex1, TexCoord + vec2(0.0, PixOffset[i]) * dy) * Weight[i];
        sum += texture(BlurTex1, TexCoord - vec2(0.0, PixOffset[i]) * dy) * Weight[i];
    }
    return sum;
}

vec4 pass4()
{
    float dx = 1.0 / (textureSize(BlurTex2, 0)).x;
    vec4 sum = texture(BlurTex2, TexCoord) * Weight[0];
    for(int i = 0; i < 10; i++)
    {
        sum += texture(BlurTex2, TexCoord + vec2(PixOffset[i], 0.0) * dx) * Weight[i];
        sum += texture(BlurTex2, TexCoord - vec2(PixOffset[i], 0.0) * dx) * Weight[i];
    }
    return sum;
}

vec4 pass5() {
    /////////////// Tone mapping ///////////////
    // Retrieve high-res color from texture
    vec4 color = texture( HdrTex, TexCoord );
    // Convert to XYZ
    vec3 xyzCol = rgb2xyz * vec3(color);
    // Convert to xyY
    float xyzSum = xyzCol.x + xyzCol.y + xyzCol.z;
    vec3 xyYCol = vec3( xyzCol.x / xyzSum, xyzCol.y / xyzSum, xyzCol.y);
    // Apply the tone mapping operation to the luminance (xyYCol.z or xyzCol.y)
    float L = (Exposure * xyYCol.z) / AveLum;
    L = (L * ( 1 + L / (White * White) )) / ( 1 + L );
    // Using the new luminance, convert back to XYZ
    xyzCol.x = (L * xyYCol.x) / (xyYCol.y);
    xyzCol.y = L;
    xyzCol.z = (L * (1 - xyYCol.x - xyYCol.y))/xyYCol.y;
    // Convert back to RGB
    vec4 toneMapColor = vec4( xyz2rgb * xyzCol, 1.0);
    ///////////// Combine with blurred texture /////////////
    // We want linear filtering on this texture access so that
    // we get additional blurring.
    vec4 blurTex = texture(BlurTex1, TexCoord);
    return toneMapColor + blurTex;
}

float Gamma = 1.0;
void main() {
    if (Pass == 1) FragColor = pass1();
    else if (Pass == 2) FragColor = pass2();
    else if (Pass == 3) FragColor = pass3();
    else if (Pass == 4) FragColor = pass4();
    else if (Pass == 5) FragColor = vec4(pow(vec3(pass5()), vec3(1.0 / Gamma)), 1.0);

}