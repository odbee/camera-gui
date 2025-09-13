#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define EGL_BUFFERS_IMPLEMENTATION // Define a special macro here

#include "EGLBuffers.hpp"
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

EglBuffers::EglBuffers(SharedContext &context)
    : shared(context),
      context(shared.get_context()),
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


void EglBuffers::makeBuffersSimple(const SharedMemoryBuffer* context)
{
    simpleBuffer.fd= getSharedProcFd(context->procid, context->fd_isp);
    console->info("got fd {} from procid {} and og fd {}", simpleBuffer.fd, context->procid, context->fd_isp);

    simpleBuffer.size = context->isp_length;
    simpleBuffer.info = context->isp;

    get_colour_space_info(simpleBuffer.info.colour_space, simpleBuffer.encoding, simpleBuffer.range);

	static const EGLint simpleAttribs[] = {
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
		EGL_YUV_COLOR_SPACE_HINT_EXT, simpleBuffer.encoding,
		EGL_SAMPLE_RANGE_HINT_EXT, simpleBuffer.range,
		EGL_NONE
	};

    console->info("Creating EGLImage from fd {} attribs: w:{} h:{} fourcc:{} p0fd:{} p0off:{} p0pitch:{} p1fd:{} p1off:{} p1pitch:{} p2fd:{} p2off:{} p2pitch:{} enc:{} range:{}", context->fd_isp,
        simpleAttribs[1], simpleAttribs[3], simpleAttribs[5],
        simpleAttribs[7], simpleAttribs[9], simpleAttribs[11],
        simpleAttribs[13], simpleAttribs[15], simpleAttribs[17],
        simpleAttribs[19], simpleAttribs[21], simpleAttribs[23],
        simpleAttribs[25], simpleAttribs[27]);

    
    EGLImage simpleImage = p_eglCreateImageKHR( egl_display_,
                                                EGL_NO_CONTEXT,
                                                EGL_LINUX_DMA_BUF_EXT,
                                                NULL,
                                                simpleAttribs);
                       
                                                
    MyEglError();
    if (!simpleImage){
        MyEglError();
        throw std::runtime_error("failed to import fd " + std::to_string(simpleBuffer.fd));
    }

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, textureID);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    p_glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, simpleImage);
    MyEglError();
    p_eglDestroyImageKHR(egl_display_, simpleImage);
    MyEglError();
    // glGenerateMipmap(GL_TEXTURE_2D);
    simpleBuffer.made=true;
    console->info("Simple Buffer mapped! isp:{}", simpleBuffer.fd);

}

void EglBuffers::makeBuffersFixedTexture(const SharedMemoryBuffer* context)
{


            // TEXTURE ESCAPADES
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int texturewidth, textureheight, nrChannels;
    unsigned char *data = stbi_load("assets/container.jpg", &texturewidth, &textureheight, &nrChannels, 0);
    if (data)
    {  
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texturewidth, textureheight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        // glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        console->error("Failed to load texture");
    }

    EGLClientBuffer clientBuffer = (EGLClientBuffer)(uintptr_t)textureID;
    EGLint attribs[] = { EGL_IMAGE_PRESERVED_KHR, EGL_TRUE, EGL_NONE };
    EGLImageKHR eglImage = p_eglCreateImageKHR(
        egl_display_,
        eglGetCurrentContext(),  // or EGL_NO_CONTEXT
        EGL_GL_TEXTURE_2D_KHR,
        clientBuffer,
        attribs
    );
        MyEglError();



    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    glGenTextures(1, &extTexture);
    glBindTexture(GL_TEXTURE_2D, extTexture);
    p_glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, eglImage);


    simpleBuffer.made=true;
    console->info("Simple Buffer mapped! isp:{}", simpleBuffer.fd);

}




void EglBuffers::simpleUpdate(){
    if(!shared.connected()){
        if(!first_time_){
            reset();
        }
        return;
    }
    
    if(first_time_)
        first_time_ = false;
    
    if (!simpleBuffer.made){
        makeBuffersSimple(context);
    }
    if (last_fd_ != context->frame){
        newFrame_ = true;
        console->info("FD: {}, FN: {}, FR: {}, SEQ: {}", simpleBuffer.fd, context->frame, context->metadata.focus, context->metadata.exposure_time);
    }
    last_fd_ = context->frame;

}

void EglBuffers::updateTexture(){
    glClear(GL_COLOR_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, extTexture);


    // glBindTexture(GL_TEXTURE_EXTERNAL_OES, textureID);
}