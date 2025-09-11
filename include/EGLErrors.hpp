#pragma once
#include <EGL/egl.h>
#include <iostream>

static void MyEglError(){
    EGLint error = eglGetError();  // Get the last error
    std::string errorMessage;

    // Match the error code with predefined EGL error codes
    switch(error) {
        case EGL_SUCCESS:
            errorMessage = "The last function succeeded without error.";
            return;
            break;
        case EGL_NOT_INITIALIZED:
            errorMessage = "EGL is not initialized, or could not be initialized, for the specified EGL display connection.";
            break;
        case EGL_BAD_ACCESS:
            errorMessage = "EGL cannot access a requested resource.";
            break;
        case EGL_BAD_ALLOC:
            errorMessage = "EGL failed to allocate resources for the requested operation.";
            break;
        case EGL_BAD_ATTRIBUTE:
            errorMessage = "An unrecognized attribute or attribute value was passed in the attribute list.";
            break;
        case EGL_BAD_CONTEXT:
            errorMessage = "An EGLContext argument does not name a valid EGL rendering context.";
            break;
        case EGL_BAD_CONFIG:
            errorMessage = "An EGLConfig argument does not name a valid EGL frame buffer configuration.";
            break;
        case EGL_BAD_CURRENT_SURFACE:
            errorMessage = "The current surface of the calling thread is a window, pixel buffer or pixmap that is no longer valid.";
            break;
        case EGL_BAD_DISPLAY:
            errorMessage = "An EGLDisplay argument does not name a valid EGL display connection.";
            break;
        case EGL_BAD_SURFACE:
            errorMessage = "An EGLSurface argument does not name a valid surface (window, pixel buffer or pixmap) configured for GL rendering.";
            break;
        default:
            errorMessage = "Unknown EGL error.";
    }

    std::cout << "EGL ERROR: "
                    << errorMessage << " :::: " << error << std::endl;
}
