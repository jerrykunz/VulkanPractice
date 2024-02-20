#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec4 a_Color;

layout(binding = 0) uniform ViewProjectionUBO 
{
    mat4 view;
    mat4 proj;
} viewProjUBO;

layout (location = 0) out vec4 v_Color;

void main()
{
	v_Color = a_Color;
	gl_Position = viewProjUBO.proj * viewProjUBO.view * vec4(a_Position, 1.0);
}