#version 460

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTexCoord;

out vec3 Position;
out vec3 Normal;
out vec2 TexCoord;
out vec3 Tangent;
out vec3 Bitangent;

out vec3 VecPos;

uniform mat4 ModelViewMatrix;
uniform mat3 NormalMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 MVP;

void main()
{
    // Pass-through texture coordinates
    TexCoord = VertexTexCoord;

    // Transform normal to view space
    Normal = normalize(NormalMatrix * VertexNormal);

    // Transform position to view space
    Position = (ModelViewMatrix * vec4(VertexPosition, 1.0)).xyz;

    // Calculate tangent and bitangent
    vec3 pos0 = (ModelViewMatrix * vec4(VertexPosition, 1.0)).xyz;
    vec3 pos1 = (ModelViewMatrix * vec4(VertexPosition + vec3(1.0, 0.0, 0.0), 1.0)).xyz; // Example neighbor vertex
    vec3 pos2 = (ModelViewMatrix * vec4(VertexPosition + vec3(0.0, 1.0, 0.0), 1.0)).xyz; // Example neighbor vertex

    vec2 uv0 = VertexTexCoord;
    vec2 uv1 = VertexTexCoord + vec2(1.0, 0.0); // Example neighbor UV
    vec2 uv2 = VertexTexCoord + vec2(0.0, 1.0); // Example neighbor UV

    // Edges of the triangle
    vec3 deltaPos1 = pos1 - pos0;
    vec3 deltaPos2 = pos2 - pos0;

    // UV delta
    vec2 deltaUV1 = uv1 - uv0;
    vec2 deltaUV2 = uv2 - uv0;

    // Calculate tangent and bitangent
    float r = 1.0 / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
    Tangent = normalize((deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r);
    Bitangent = normalize((deltaPos2 * deltaUV1.x - deltaPos1 * deltaUV2.x) * r);

    // Transform tangent and bitangent to view space
    Tangent = normalize(NormalMatrix * Tangent);
    Bitangent = normalize(NormalMatrix * Bitangent);


    VecPos = VertexPosition;

    // Output vertex position
    gl_Position = MVP * vec4(VertexPosition, 1.0);
}
