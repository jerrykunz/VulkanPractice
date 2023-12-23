#version 450

#define MAX_MODELS 100

struct ModelObject 
{
    mat4 transform;
};

layout(binding = 0) uniform UniformBufferObject 
{
    //mat4 model; //just so we can test both normal and array
    mat4 view;
    mat4 proj;
    ModelObject models[MAX_MODELS]; 
} ubo;



layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout(push_constant) uniform PushConstants 
{
    int ModelIndex;
} pushConstants;

void main() 
{

    // Retrieve the model matrix based on the index
    mat4 modelMatrix = ubo.models[pushConstants.ModelIndex].transform;

    //gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
    gl_Position = ubo.proj * ubo.view * modelMatrix * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
