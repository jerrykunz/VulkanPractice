
#version 450 core

layout(location = 0) out vec4 color;

struct VertexOutput
{
	vec4 Color;
	vec2 TexCoord;
	float TilingFactor;
};

layout (location = 0) in VertexOutput Input;
layout (location = 5) in flat float TexIndex;

layout (binding = 2) uniform sampler2D u_Textures[32];

void main()
{
	color = texture(u_Textures[int(TexIndex)], Input.TexCoord * Input.TilingFactor) * Input.Color;
	//color.a = 0.1f;
	// Discard to avoid depth write
	if (color.a == 0.0)
		discard;
}