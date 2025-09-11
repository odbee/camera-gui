#version 310 es

#extension GL_OES_EGL_image_external : require

precision mediump float;

in vec4 DefaultColor;
in vec2 TexCoord;

out vec4 FragColor;

uniform samplerExternalOES PreviewTexture;

void main()
{
   FragColor = DefaultColor+texture2D(PreviewTexture,TexCoord);
   
}