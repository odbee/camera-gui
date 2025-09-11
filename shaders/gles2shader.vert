// GLES2 vertex shader
attribute vec3 aPos;
attribute vec2 aTexCoord;

varying vec2 TexCoord;
varying vec4 DefaultColor;

void main()
{
    gl_Position = vec4(aPos, 1.0); // same as vec4(aPos.x, aPos.y, aPos.z, 1.0)
    TexCoord = aTexCoord;
    DefaultColor = vec4(0.1, 0.7, 0.2, 1.0);
}