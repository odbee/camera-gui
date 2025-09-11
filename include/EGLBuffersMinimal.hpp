#pragma once
#ifndef EGLBUFFERS_HPP
#define EGLBUFFERS_HPP

#include "GetStreamData.hpp"

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



// EGL_KHR_image extensions
typedef EGLImage (EGLAPIENTRYP eglCreateImageKHR_type)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
typedef EGLBoolean (EGLAPIENTRYP eglDestroyImageKHR_type)(EGLDisplay dpy, EGLImage image);
// GL_OES_EGL_image extensions
typedef void (EGLAPIENTRYP glEGLImageTargetTexture2DOES_type)(GLenum target, GLeglImageOES image);
// EGL_EXT_platform_base and EGL_EXT_device_base extensions
typedef EGLBoolean (EGLAPIENTRYP eglQueryDevicesEXT_type)(EGLint max_devices, EGLDeviceEXT *devices, EGLint *num_devices);
typedef EGLDisplay (EGLAPIENTRYP eglGetPlatformDisplayEXT_type)(EGLenum platform, void *native_display, const EGLint *attrib_list);

#ifdef EGL_BUFFERS_IMPLEMENTATION
    // This part is the definition. It will be included only in EglBuffers.cpp.
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

enum BufferType {
    RAW,
    ISP,
    LORES,
    LUMA
};

struct Buffer
{
    Buffer() : fd(-1) {}
    int fd;
    size_t size;
    StreamInfo info;
    GLuint texture;
    EGLint encoding;
    EGLint range;
    bool made=false;
};

struct FrameBuffer
{
    FrameBuffer() : mapped(-1) {}
    Buffer raw;
    Buffer isp;
    Buffer lores;
    Buffer luma;
    libcamera::ControlList metadata;
    unsigned int sequence;
    float framerate;
    int mapped;
};


class EglBuffers {
    public:
        explicit EglBuffers(GetStreamData &context);
        ~EglBuffers();
        int initEGLExtensions();
        void simpleUpdate();
        void draw();
        bool new_frame(){
            bool val = newFrame_;
            newFrame_ = false;
            return val;
        }
        Buffer simpleBuffer;
        GLuint textureID;
        GLuint extTexture;
        GLuint fbo;


        FrameBuffer& getBuffer() {return buffers_[current_index_]; }

    private:

        void makeBuffer(const SharedStreamData* context);
        void reset();
        EGLDisplay egl_display_;
        GetStreamData &shared;
        const SharedStreamData* context;

        int current_index_;
        std::map<int, FrameBuffer> buffers_; // map the DMABUF's fd to the Buffer
        bool first_time_;
        int last_fd_;
        bool newFrame_;

        std::shared_ptr<spdlog::logger> console;
};

#endif // EGLBUFFERS_HPP