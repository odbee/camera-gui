#extension GL_OES_EGL_image_external : enable
precision mediump float;

uniform sampler2D screenTexture;
varying vec2 TexCoord;

void main() {
    gl_FragColor = texture2D(screenTexture, TexCoord)+vec4(0.1,0.2,0.2,1.0);
}