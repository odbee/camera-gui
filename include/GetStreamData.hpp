#pragma once
#include <cstdint>
#include <ctime>

#include <sys/ipc.h>
#include <sys/shm.h>


#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <thread>

#include <signal.h>
#include <errno.h>

#include <libcamera/stream.h>
#include <libcamera/color_space.h>
#include <libcamera/pixel_format.h>
#include <libcamera/controls.h>
#include <optional>
#define STATE_TIMEOUT 0x08
#define STATE_NULL_REF 0x04
#define STATE_VALID 0x01


#define PROJECT_ID 0x4341494D // ASCII for "CAIM`"

    struct StreamInfo
    {
    StreamInfo() : width(0), height(0), stride(0) {}
        unsigned int width;
        unsigned int height;
        unsigned int stride;
        libcamera::PixelFormat pixel_format;
        std::optional<libcamera::ColorSpace> colour_space;
    };

struct SharedStreamData{
        SharedStreamData() : procid(-1) {}
        StreamInfo stream_info;
        int procid;
        int fd;
        int span_size;
    };


class GetStreamData {
public:
    GetStreamData();
    ~GetStreamData();
    SharedStreamData* get_shared_memory() const{ return shared_memory; }
    bool connected(){return state_ == STATE_VALID && !timedOut_;}

private:




    uint64_t getTs();
    void threadTask();
    bool isProcessAlive(pid_t pid);
    

    SharedStreamData* shared_memory;
    std::shared_ptr<spdlog::logger> console;
    std::atomic<uint8_t> state_{0};
    std::atomic<bool> timedOut_{false};
    bool abortThread_;
    std::thread main_thread_;
    int lastProcId_=-1;
};  
