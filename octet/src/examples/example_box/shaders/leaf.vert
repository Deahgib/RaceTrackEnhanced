
attribute vec4 pos;
uniform mat4 modelToProjection;

void main()
{
    gl_Position = modelToProjection * pos;
}