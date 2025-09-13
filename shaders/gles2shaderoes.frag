#extension GL_OES_EGL_image_external : enable
precision mediump float;

uniform samplerExternalOES PreviewTexture;
varying vec2 TexCoord;

void main() {
    gl_FragColor = texture2D(PreviewTexture, TexCoord);
}