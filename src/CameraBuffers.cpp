#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#define EGL_BUFFERS_IMPLEMENTATION // Define a special macro here

#include "CameraBuffers.hpp"
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
static GLint compile_shader(GLenum target, const char *source)
{
	GLuint s = glCreateShader(target);
	glShaderSource(s, 1, (const GLchar **)&source, NULL);
	glCompileShader(s);
	GLint ok;
	glGetShaderiv(s, GL_COMPILE_STATUS, &ok);

	if (!ok)
	{
		GLchar *info;
		GLint size;

		glGetShaderiv(s, GL_INFO_LOG_LENGTH, &size);
		info = (GLchar *)malloc(size);

		glGetShaderInfoLog(s, size, NULL, info);
		throw std::runtime_error("failed to compile shader: " + std::string(info) + "\nsource:\n" +
								 std::string(source));
	}

	return s;
}


GLint shaderProgram;
GLuint VAO, VBO;

static GLint link_program(GLint vs, GLint fs)
{
	GLint prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);

	GLint ok;
	glGetProgramiv(prog, GL_LINK_STATUS, &ok);
	if (!ok)
	{
		/* Some drivers return a size of 1 for an empty log.  This is the size
		 * of a log that contains only a terminating NUL character.
		 */
		GLint size;
		GLchar *info = NULL;
		glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &size);
		if (size > 1)
		{
			info = (GLchar *)malloc(size);
			glGetProgramInfoLog(prog, size, NULL, info);
		}

		throw std::runtime_error("failed to link: " + std::string(info ? info : "<empty log>"));
	}

	return prog;
}

void gl_setup()
{
    
	const char* vvss =
         "#version 300 es\n"
         "layout(location = 0) in vec4 pos;\n"
		"layout (location = 1) in vec2 aTexCoord;\n"

         "out vec2 texcoord;\n"
         "\n"
         "void main() {\n"
         "  gl_Position = pos;\n"
         "  texcoord = aTexCoord;\n"
         
         "}\n";
	GLint vs_s = compile_shader(GL_VERTEX_SHADER, vvss);
	const char* fs =
	"#version 300 es\n"
	"#extension GL_OES_EGL_image_external : require\n"
	"precision mediump float;\n"
	"in vec2 texcoord;\n"
	"out vec4 FragColor;\n"
	"uniform samplerExternalOES s;\n"
	"void main() {\n"
	"  FragColor = texture2D(s, texcoord);\n"
	"}\n";
	GLint fs_s = compile_shader(GL_FRAGMENT_SHADER, fs);
	shaderProgram = link_program(vs_s, fs_s);

	glUseProgram(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "s"), 0);
    
	glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
	
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// static const float verts[] = { -w_factor, -h_factor, w_factor, -h_factor, w_factor, h_factor, -w_factor, h_factor };
	static const float verts[] = {
    // x, y, z, u, v
    -0.9f, -0.9f, 0, 0, 0,
     0.9f, -0.9f, 0, 1, 0,
     0.9f,  0.9f, 0, 1, 1,
    -0.9f,  0.9f, 0, 0, 1
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1,2, GL_FLOAT, GL_FALSE, 5*sizeof(float),(void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    // glBindBuffer(GL_ARRAY_BUFFER, 0);


    glUseProgram(0);
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


CameraBuffers::CameraBuffers(): sharedData(),first_time_(true)
{
    console = spdlog::stdout_color_mt("egl_buffers");
    initEGLExtensions();
    gl_setup();
    egl_display_=eglGetCurrentDisplay();
    console->info("(EGL Vendor) {} (EGL Version) {}", eglQueryString(egl_display_, EGL_VENDOR), eglQueryString(egl_display_, EGL_VERSION) );


}
CameraBuffers::~CameraBuffers()
{
}



int CameraBuffers::initEGLExtensions() {
    
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

void CameraBuffers::Show()
{
    int fd = sharedData.get_shared_memory()->fd;
    int span_size = sharedData.get_shared_memory()->span_size;
    int procid = sharedData.get_shared_memory()->procid;
    StreamInfo info=sharedData.get_shared_memory()->stream_info;

    Buffer &buffer = buffers_[fd];

    if (first_time_)
    {
		
		first_time_ = false;
    }

	if (buffer.fd == -1)
    
		makeBuffer(procid, fd, span_size, info, buffer);
	last_fd_ = fd;

    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    


    glBindTexture(GL_TEXTURE_EXTERNAL_OES, buffer.texture);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glBindVertexArray(0); 


}

void CameraBuffers::resetBuffers()
{
    for (auto &it : buffers_)
		glDeleteTextures(1, &it.second.texture);
	buffers_.clear();
	last_fd_ = -1;
	first_time_ = true;
}

void CameraBuffers::makeBuffer(int procid, int fd, size_t size, StreamInfo const &info, Buffer &buffer)
{
    buffer.fd = getSharedProcFd(procid,fd);
    console->info("received PROCID {} and FD {} to Create {}",procid, fd, buffer.fd );

	buffer.size = size;
	buffer.info = info;

	EGLint encoding, range;
	get_colour_space_info(info.colour_space, encoding, range);

	EGLint attribs[] = {
		EGL_WIDTH, static_cast<EGLint>(info.width),
		EGL_HEIGHT, static_cast<EGLint>(info.height),
		EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_YUV420,
		EGL_DMA_BUF_PLANE0_FD_EXT, buffer.fd,
		EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
		EGL_DMA_BUF_PLANE0_PITCH_EXT, static_cast<EGLint>(info.stride),
		EGL_DMA_BUF_PLANE1_FD_EXT, buffer.fd,
		EGL_DMA_BUF_PLANE1_OFFSET_EXT, static_cast<EGLint>(info.stride * info.height),
		EGL_DMA_BUF_PLANE1_PITCH_EXT, static_cast<EGLint>(info.stride / 2),
		EGL_DMA_BUF_PLANE2_FD_EXT, buffer.fd,
		EGL_DMA_BUF_PLANE2_OFFSET_EXT, static_cast<EGLint>(info.stride * info.height + (info.stride / 2) * (info.height / 2)),
		EGL_DMA_BUF_PLANE2_PITCH_EXT, static_cast<EGLint>(info.stride / 2),
		EGL_YUV_COLOR_SPACE_HINT_EXT, encoding,
		EGL_SAMPLE_RANGE_HINT_EXT, range,
		EGL_NONE
	};


	PFNEGLCREATEIMAGEKHRPROC eglCreateImageKHR = nullptr;
    eglCreateImageKHR = reinterpret_cast<PFNEGLCREATEIMAGEKHRPROC>(eglGetProcAddress("eglCreateImageKHR"));

	EGLImage image = eglCreateImageKHR(egl_display_, EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, attribs);
	if (!image)
		throw std::runtime_error("failed to import fd " + std::to_string(buffer.fd));

    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC glEGLImageTargetTexture2DOES = nullptr;
    glEGLImageTargetTexture2DOES = reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(eglGetProcAddress("glEGLImageTargetTexture2DOES"));
    PFNEGLDESTROYIMAGEKHRPROC eglDestroyImageKHR = nullptr;
    eglDestroyImageKHR = reinterpret_cast<PFNEGLDESTROYIMAGEKHRPROC>(eglGetProcAddress("eglDestroyImageKHR"));
	glGenTextures(1, &buffer.texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_EXTERNAL_OES, buffer.texture);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, image);
	eglDestroyImageKHR(egl_display_, image);
}

