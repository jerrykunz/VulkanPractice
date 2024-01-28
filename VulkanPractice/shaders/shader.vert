#version 450

#define MAX_MODELS 100

struct InstanceData 
{
    mat4 transform;
};

layout(binding = 0) uniform ViewProjectionUBO 
{
    mat4 view;
    mat4 proj;
} viewProjUBO;

layout(binding = 1) uniform InstanceDataUBO 
{
    InstanceData instances[MAX_MODELS]; 
} instanceDataUBO;


layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

//layout(push_constant) uniform PushConstants 
//{
//    int ModelIndex;
//} pushConstants;

void main() 
{
    // Retrieve the model matrix based on the index
    mat4 modelMatrix = instanceDataUBO.instances[gl_InstanceIndex].transform;

    gl_Position = viewProjUBO.proj * viewProjUBO.view * modelMatrix * vec4(inPosition, 1.0);
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
