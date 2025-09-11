// GLES2 fragment shader
precision mediump float;

varying vec4 DefaultColor;
varying vec2 TexCoord;

uniform sampler2D PreviewTexture;

void main()
{
    gl_FragColor = DefaultColor + texture2D(PreviewTexture, TexCoord);
}
