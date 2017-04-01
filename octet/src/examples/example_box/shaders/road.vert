#version 330

attribute vec4 pos;

uniform mat4 modelToProjection;

varying vec4 coord;

void main()
{
	coord = pos;
    gl_Position = modelToProjection * pos;
}