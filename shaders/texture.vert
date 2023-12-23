#version 450
#extension GL_ARB_separate_shader_objects : enable

layout( push_constant ) uniform ColorBlock {
  mat4 matrix;
  vec4 pos, uv;
} push;
layout(set = 0, binding = 0) uniform  CameraBuffer{
	mat4 view;
} dummy;

layout(location = 0) in vec2 vert;

layout(location = 0) out vec2 fragTexCoord;

void main() {
    // Pass the tex coord straight through to the fragment shader
    fragTexCoord = vec2(push.uv.x + vert.x*push.uv.z, push.uv.y + vert.y*push.uv.w);
    
    gl_Position = push.matrix*vec4(push.pos.x + vert.x*push.pos.z, push.pos.y + vert.y*push.pos.w, 0, 1);
}