#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define EGL_BUFFERS_IMPLEMENTATION // Define a special macro here

#include "EGLBuffersMinimal.hpp"
#include "EGLErrors.hpp"
#include <libdrm/drm_fourcc.h>

int getSharedProcFd(int procid, int fd){
    int pid_fd = syscall(SYS_pidfd_open, procid, 0);
    if (pid_fd == -1) {
        perror("pidfd_open");
        return -1;
    }

    int new_fd_raw = syscall(SYS_pidfd_getfd, pid_fd, fd, 0);
    if (new_fd_raw == -1) {
        printf("%d , %d\r\n", pid_fd, fd);
        perror("pidfd_getfd");
        close(pid_fd);
        return -1;
    }

    close(pid_fd);

    return new_fd_raw;
}

static void get_colour_space_info(std::optional<libcamera::ColorSpace> const &cs, EGLint &encoding, EGLint &range)
{
	encoding = EGL_ITU_REC601_EXT;
	range = EGL_YUV_NARROW_RANGE_EXT;

	if (cs == libcamera::ColorSpace::Sycc)
		range = EGL_YUV_FULL_RANGE_EXT;
	else if (cs == libcamera::ColorSpace::Smpte170m)
		/* all good */;
	else if (cs == libcamera::ColorSpace::Rec709)
		encoding = EGL_ITU_REC709_EXT;
}

EglBuffers::EglBuffers(GetStreamData &context)
    : shared(context),
      context(shared.get_shared_memory()),
      current_index_(-1),
      first_time_(true),
      last_fd_(-1),
      newFrame_(false)
{
    console = spdlog::stdout_color_mt("egl_buffers");
    initEGLExtensions();
}

EglBuffers::~EglBuffers()
{
    reset();
}


void EglBuffers::reset(){
        for (auto &it : buffers_){
        glDeleteTextures(1, &it.second.raw.texture);
        glDeleteTextures(1, &it.second.isp.texture);
        glDeleteTextures(1, &it.second.lores.texture);
        glDeleteTextures(1, &it.second.luma.texture);
        close(it.second.raw.fd);
        close(it.second.isp.fd);
        close(it.second.lores.fd);
    }
        buffers_.clear();
        last_fd_ = -1;
        first_time_ = true;
        console->info("Reset buffers!");
}
int EglBuffers::initEGLExtensions() {
    
    // 1. Check for EGL_KHR_image support
    const char* platform_extensions = eglQueryString(EGL_NO_DISPLAY, EGL_EXTENSIONS);
    if (platform_extensions == nullptr) {
        console->error("Failed to query EGL platform extensions.");
        return false;
    }

    if (strstr(platform_extensions, "EGL_EXT_device_base") == nullptr) {
        console->error("EGL_EXT_device_base extension not supported.");
        return false;
    }

    if (strstr(platform_extensions, "EGL_EXT_platform_base") == nullptr) {
        console->error("EGL_EXT_platform_base extension not supported.");
        return false;
    }
    
    p_eglQueryDevicesEXT = (eglQueryDevicesEXT_type)eglGetProcAddress("eglQueryDevicesEXT");
    p_eglGetPlatformDisplayEXT = (eglGetPlatformDisplayEXT_type)eglGetProcAddress("eglGetPlatformDisplayEXT");

    if (!p_eglQueryDevicesEXT || !p_eglGetPlatformDisplayEXT) {
        console->error("Failed to load EGL platform/device functions.");
        return false;
    }

    // Example call to get the display from a device.
    static const int MAX_DEVICES = 4;
    EGLDeviceEXT devices[MAX_DEVICES];
    EGLint num_devices;
    p_eglQueryDevicesEXT(1, devices, &num_devices);
    console->info("Detected {} devices", num_devices);
    if (num_devices < 1) {
        console->error("No EGL devices found.");
        return false;
    }

    egl_display_ = p_eglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT, (void*)devices[0], nullptr);

    if (egl_display_ == EGL_NO_DISPLAY) {
        console->error("Failed to get platform display.");
        return false;
    }
    console->info("EGLDisplay handle: {}", egl_display_);

    EGLBoolean initialized = eglInitialize(egl_display_, nullptr, nullptr);
    if (initialized == EGL_FALSE) {
        console->error("Failed to initialize EGL display. EGL error: {}", eglGetError());
        return false;
    }

    const char* display_extensions = eglQueryString(egl_display_, EGL_EXTENSIONS);
    if (display_extensions == nullptr) {
        console->error("EGL display failed to return extension string.");
        return false;
    }
    
    if (strstr(display_extensions , "EGL_KHR_image") == nullptr) {
        console->error("EGL_KHR_image extension not supported.");
        return false;
    }

    p_eglCreateImageKHR = (eglCreateImageKHR_type)eglGetProcAddress("eglCreateImageKHR");
    p_eglDestroyImageKHR = (eglDestroyImageKHR_type)eglGetProcAddress("eglDestroyImageKHR");

    if (!p_eglCreateImageKHR || !p_eglDestroyImageKHR) {
        console->error("Failed to load EGL_KHR_image functions.");
        return false;
    }

    const char* gl_extensions = (const char*)glGetString(GL_EXTENSIONS);
    if (gl_extensions == nullptr) {
        // This will happen if no context is current, which is expected.
        // It's often handled in the main render loop, not init().
        // For this example, let's assume it's valid.
    } else {
        if (strstr(gl_extensions, "GL_OES_EGL_image") == nullptr) {
            console->error("GL_OES_EGL_image extension not supported.");
            return false;
        }

        p_glEGLImageTargetTexture2DOES = (glEGLImageTargetTexture2DOES_type)eglGetProcAddress("glEGLImageTargetTexture2DOES");
        if (!p_glEGLImageTargetTexture2DOES) {
            console->error("Failed to load glEGLImageTargetTexture2DOES.");
            return false;
        }
    }

    EGLBoolean bound = eglBindAPI(EGL_OPENGL_ES_API);
    if (bound == EGL_FALSE) {
        console->error("Failed to bind EGL_OPENGL_ES_API.");
        return false;
    }

    // If we reach here, all functions are loaded successfully
    return true;

}


void EglBuffers::makeBuffer(const SharedStreamData* context)
{   console->info("Making simple buffer!");
    MyEglError();

    simpleBuffer.fd=context->fd;
    simpleBuffer.size=context->span_size;
    simpleBuffer.info=context->stream_info;
    EGLint encoding, range;
    get_colour_space_info(simpleBuffer.info.colour_space, encoding, range);
    console->info("Planning to make buffer: {}x{} stride {} fd {} texture {}", simpleBuffer.info.width, simpleBuffer.info.height, simpleBuffer.info.stride, simpleBuffer.fd, simpleBuffer.texture);
	EGLint attribs[] = {
		EGL_WIDTH, static_cast<EGLint>(simpleBuffer.info.width),
		EGL_HEIGHT, static_cast<EGLint>(simpleBuffer.info.height),
		EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_YUV420,
		EGL_DMA_BUF_PLANE0_FD_EXT, simpleBuffer.fd,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
		EGL_DMA_BUF_PLANE0_PITCH_EXT, static_cast<EGLint>(simpleBuffer.info.stride),
		EGL_DMA_BUF_PLANE1_FD_EXT, simpleBuffer.fd,
		EGL_DMA_BUF_PLANE1_OFFSET_EXT, static_cast<EGLint>(simpleBuffer.info.stride * simpleBuffer.info.height),
		EGL_DMA_BUF_PLANE1_PITCH_EXT, static_cast<EGLint>(simpleBuffer.info.stride / 2),
		EGL_DMA_BUF_PLANE2_FD_EXT, simpleBuffer.fd,
		EGL_DMA_BUF_PLANE2_OFFSET_EXT, static_cast<EGLint>(simpleBuffer.info.stride * simpleBuffer.info.height + (simpleBuffer.info.stride / 2) * (simpleBuffer.info.height / 2)),
		EGL_DMA_BUF_PLANE2_PITCH_EXT, static_cast<EGLint>(simpleBuffer.info.stride / 2),
		EGL_YUV_COLOR_SPACE_HINT_EXT, encoding,
		EGL_SAMPLE_RANGE_HINT_EXT, range,
		EGL_NONE
	};
    console->info("Creating EGLImage from fd {} attribs: w:{} h:{} fourcc:{} p0fd:{} p0off:{} p0pitch:{} p1fd:{} p1off:{} p1pitch:{} p2fd:{} p2off:{} p2pitch:{} enc:{} range:{}", simpleBuffer.fd,
        attribs[1], attribs[3], attribs[5],
        attribs[7], attribs[9], attribs[11],
        attribs[13], attribs[15], attribs[17],
        attribs[19], attribs[21], attribs[23],
        attribs[25], attribs[27]);
    
    EGLImage image = p_eglCreateImageKHR(egl_display_, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, attribs);

	MyEglError();

    if (!image)
		throw std::runtime_error("failed to import fd " + std::to_string(simpleBuffer.fd));

    glGenTextures(1, &simpleBuffer.texture);
	glBindTexture(GL_TEXTURE_EXTERNAL_OES, simpleBuffer.texture);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	p_glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, image);
    p_eglDestroyImageKHR(egl_display_, image);
    simpleBuffer.encoding = encoding;
    simpleBuffer.range = range;
    simpleBuffer.made = true;




}


void EglBuffers::simpleUpdate(){

    if (first_time_){
        first_time_ = false;
    }
    

    if(simpleBuffer.fd==-1){
        makeBuffer(context);
    }


}

void EglBuffers::draw() {
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, simpleBuffer.texture);

}