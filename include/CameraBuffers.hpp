#pragma once

#include "StreamDataHandler.hpp"

#include <map>
#include <unistd.h>
#include <sys/syscall.h>
#include <iostream>



#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl31.h>

#include <GLES2/gl2ext.h> // this is correct

#include <libcamera/control_ids.h>
#include <libcamera/controls.h>

// DEBUG LIBS
#include <sys/mman.h>

#include <cstdio> // for fprintf

int getSharedProcFd(int procid, int fd);

// EGL_KHR_image extensions
typedef EGLImage (EGLAPIENTRYP eglCreateImageKHR_type)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP eglDestroyImageKHR_type)(EGLDisplay dpy, EGLImage image);
// GL_OES_EGL_image extensions
typedef void (EGLAPIENTRYP glEGLImageTargetTexture2DOES_type)(GLenum target, GLeglImageOES image);
// EGL_EXT_platform_base and EGL_EXT_device_base extensions
typedef EGLBoolean (EGLAPIENTRYP eglQueryDevicesEXT_type)(EGLint max_devices, EGLDeviceEXT *devices, EGLint *num_devices);
typedef EGLDisplay (EGLAPIENTRYP eglGetPlatformDisplayEXT_type)(EGLenum platform, void *native_display, const EGLint *attrib_list);

#ifdef EGL_BUFFERS_IMPLEMENTATION
    // This part is the definition. It will be included only in CameraBuffers.cpp.
    eglCreateImageKHR_type p_eglCreateImageKHR = nullptr;
    eglDestroyImageKHR_type p_eglDestroyImageKHR = nullptr;
    glEGLImageTargetTexture2DOES_type p_glEGLImageTargetTexture2DOES = nullptr;
    eglQueryDevicesEXT_type p_eglQueryDevicesEXT = nullptr;
    eglGetPlatformDisplayEXT_type p_eglGetPlatformDisplayEXT = nullptr;
#else
    // This part is the declaration with 'extern'.
    // It will be included in all other files that include this header.
    extern eglCreateImageKHR_type p_eglCreateImageKHR;
    extern eglDestroyImageKHR_type p_eglDestroyImageKHR;
    extern glEGLImageTargetTexture2DOES_type p_glEGLImageTargetTexture2DOES;
    extern eglQueryDevicesEXT_type p_eglQueryDevicesEXT;
    extern eglGetPlatformDisplayEXT_type p_eglGetPlatformDisplayEXT;
#endif





namespace controls = libcamera::controls;



class CameraBuffers {
    public:
        explicit CameraBuffers();
        ~CameraBuffers();
        int initEGLExtensions();
        void Show();
        void resetBuffers();

    private:
        struct Buffer
        {
            Buffer() : fd(-1) {}
            int fd;
            size_t size;
            StreamInfo info;
            GLuint texture;
        };

        void makeBuffer(int procid, int fd, size_t size, StreamInfo const &info, Buffer &buffer);
        StreamDataHandler sharedData;
        EGLDisplay egl_display_;
        std::map<int, Buffer> buffers_;
        int last_fd_;
        bool first_time_;
        std::shared_ptr<spdlog::logger> console;
};

