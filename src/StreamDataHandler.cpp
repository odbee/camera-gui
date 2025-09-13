#include "StreamDataHandler.hpp"


bool StreamDataHandler::isProcessAlive(pid_t pid) {
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
StreamDataHandler::StreamDataHandler(){
    console = spdlog::stdout_color_mt("shared_context");
    int segment_id;
    key_t key = ftok("/tmp", PROJECT_ID);
    bool shB = true, smB = true;
    segment_id = shmget(key, sizeof(SharedStreamData), S_IRUSR | S_IWUSR);
    if (segment_id == -1) {
        console->error("shmget failed with key \"0x{:08X}\", SharedMemoryBuffer size: {}  Error: \"{}\"",
            key, sizeof(SharedStreamData), strerror(errno));
        
        shB = false;
    }

    shared_memory = (SharedStreamData*)shmat(segment_id, NULL, 0);
    if (shared_memory == (void*) -1) {
        smB = false;
    }

    if (shB && smB) {
        struct shmid_ds buf;
        if (shmctl(segment_id, IPC_STAT, &buf) == 0) {
            if (buf.shm_segsz != sizeof(SharedStreamData)) {
                console->critical("SIZE MISMATCH: Expected {} bytes, but found {} bytes.", sizeof(SharedStreamData), buf.shm_segsz);
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
    main_thread_ = std::thread(std::bind(&StreamDataHandler::threadTask, this));

}


uint64_t StreamDataHandler::getTs(){
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return uint64_t(ts.tv_sec * 1000LL + ts.tv_nsec / 1000000);
}



void StreamDataHandler::threadTask(){
    console->info("thread started!");
    const SharedStreamData* context = get_shared_memory();
    console->info("received context!");
    
    while(!abortThread_){
        if(context == nullptr|| context == (SharedStreamData*)-1){
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

        // if((getTs() - context->ts) > 1000 && !timedOut_){          // IMPLEMENT TIMESTAMP IN SHARED STRUCTURE
        //     // console->info("connection to cinepi raw timed out!");
        //     state_ |= STATE_TIMEOUT;
        //     timedOut_ = true;
        // } else {
        //     state_ &= ~STATE_TIMEOUT;
        // }

        if(timedOut_ && (lastProcId_ != context->procid)){
            timedOut_ = false;
        }

        lastProcId_ = context->procid;

        


        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}


StreamDataHandler::~StreamDataHandler(){
       abortThread_ = true;
       if (main_thread_.joinable()) main_thread_.join();
       if (shared_memory && shared_memory != (void*)-1) shmdt(shared_memory);
}
