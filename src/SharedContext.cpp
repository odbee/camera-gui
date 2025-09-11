#include "SharedContext.hpp"

#include <signal.h>
#include <errno.h>



bool isProcessAlive(pid_t pid) {
    if (pid < 1) {
        return false;
    }

    if (kill(pid, 0) == 0) {
        return true;  // Process exists
    } else {
        if (errno == ESRCH) {
            return false;  // Process does not exist
        }
        // Handle other errors like EPERM
        return false;
    }
}

SharedContext::SharedContext()
    : 
    state_(0),
    timedOut_(false),
    lastProcId_(-1),
    abortThread_(false)
                

    {
        console = spdlog::stdout_color_mt("shared_context");
        bind_shared_memory();
        main_thread_ = std::thread(std::bind(&SharedContext::threadTask, this));
    };
SharedContext::~SharedContext(){
    abortThread_ = true;
    main_thread_.join();
};

SharedMemoryBuffer* SharedContext::get_context() const{
    if(shared_memory == nullptr) {
        console->debug("shared_memory is nullptr");
        return nullptr;
    }
    if(shared_memory == (void*)-1) {
        console->debug("shared_memory is invalid (-1)");
        return nullptr;
    }
    return shared_memory;
}

uint64_t SharedContext::getTs(){
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return uint64_t(ts.tv_sec * 1000LL + ts.tv_nsec / 1000000);
}
    

key_t SharedContext::get_shared_memory_key() {
    return ftok("/tmp", PROJECT_ID);
}

void SharedContext::bind_shared_memory() {
    int segment_id;
    key_t key = get_shared_memory_key();

    bool shB = true, smB = true;
    segment_id = shmget(key, sizeof(SharedMemoryBuffer), S_IRUSR | S_IWUSR);
    if (segment_id == -1) {
        console->error("shmget failed with key \"0x{:08X}\", SharedMemoryBuffer size: {}  Error: \"{}\"",
            key, sizeof(SharedMemoryBuffer), strerror(errno));
        
        shB = false;
    }

    shared_memory = (SharedMemoryBuffer*)shmat(segment_id, NULL, 0);
    if (shared_memory == (void*) -1) {
        smB = false;
    }

    if (shB && smB) {
        struct shmid_ds buf;
        if (shmctl(segment_id, IPC_STAT, &buf) == 0) {
            if (buf.shm_segsz != sizeof(SharedMemoryBuffer)) {
                console->critical("SIZE MISMATCH: Expected {} bytes, but found {} bytes.", sizeof(SharedMemoryBuffer), buf.shm_segsz);
                // Size mismatch found. This is a fatal error.
                shmdt(shared_memory); // Detach the incorrect memory
                shared_memory = nullptr;
                smB = false; // Flag failure
            }
        } else {
            // shmctl failed, we can't trust the segment.
            smB = false;
        }
    }


    state_ |= (uint8_t)(shB && smB);
}

void SharedContext::threadTask(){
    console->info("thread started!");
    const SharedMemoryBuffer* context = get_context();
    console->info("received context!");
    
    while(!abortThread_){
        if(context == nullptr|| context == (SharedMemoryBuffer*)-1){
            console->critical("shared_memory dropped or failed to bind!");
            state_ |= STATE_NULL_REF;
            std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Sleep on error
            continue; // Skip the rest of the loop for this iteration
        } 
        
        state_ &= ~STATE_NULL_REF;
        // console->info("Checking process status with context at address: {}", fmt::ptr(context));

        if(!(isProcessAlive(context->procid))){
            state_ &= ~STATE_VALID;
        }else{
            state_ |= STATE_VALID;
        }

        if((getTs() - context->ts) > 1000 && !timedOut_){
            // console->info("connection to cinepi raw timed out!");
            state_ |= STATE_TIMEOUT;
            timedOut_ = true;
        } else {
            state_ &= ~STATE_TIMEOUT;
        }

        if(timedOut_ && (lastProcId_ != context->procid)){
            timedOut_ = false;
        }

        lastProcId_ = context->procid;

        


        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}