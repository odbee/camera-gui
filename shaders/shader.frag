#version 310 es

precision mediump float;

in vec4 DefaultColor;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D PreviewTexture;

void main()
{
   FragColor = DefaultColor+texture(PreviewTexture,TexCoord);
   
}