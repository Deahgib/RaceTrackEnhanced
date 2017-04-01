#version 150

varying vec4 coord;

out vec4 outColor;

void main()
{
    outColor = vec4(coord.z, coord.z, coord.z, 1.0);
}