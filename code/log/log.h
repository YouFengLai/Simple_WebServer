#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include "block_queue.h"
#include <pthread.h>
#include <string>
#include <iostream>
#include <stdarg.h>
#include <memory>

#define LEN 128
class log{
public:
    static log& get_instance(){
        static log instance;
        return instance;
    }

    static void* flush_log_thread(void* arg){
        log::get_instance().async_write_log();
    }

    // bool init(const char* file_name, bool close_log, unsigned int log_buf_size = 8192, unsigned int max_lines = 5000000, int max_queue_size = 0);
    bool init(const char* file_name, bool close_log, unsigned int log_buf_size, unsigned int max_lines, int max_queue_size);

    void write_log(unsigned int level, const char* format, ...);

    void flush();

private:
    log();
    ~log();
    log(const log& _log) = delete;
    long& operator= (const long& _log) = delete;

    void* async_write_log(){
        std::string single_log;
        while(_log_queue->pop(single_log)){
            _mutex.lock();
            fputs(single_log.c_str(), _fp);
            _mutex.unlock();
        }
    }

private:
    char dir_name[LEN];
    char log_name[LEN];
    unsigned int _max_lines;
    unsigned int _log_buf_size;
    unsigned long long _log_line_count;
    unsigned int _days;
    FILE* _fp;
    char* _buf;
    std::shared_ptr<block_queue<std::string>> _log_queue;
    bool _async;
    locker _mutex;
    bool _close_log;

};

#define LOG_DEBUG(format, ...) if(!_close_log){log::get_instance().write_log(0, format, ##_VA_ARGS__); log::get_instance().flush();}
#define LOG_INFO(format, ...) if(!_close_log){log::get_instance().write_log(1, format, ##_VA_ARGS__); log::get_instance().flush();}
#define LOG_WARN(format, ...) if(!_close_log){log::get_instance().write_log(2, format, ##_VA_ARGS__); log::get_instance().flush();}
#define LOG_ERROR(format, ...) if(!_close_log){log::get_instance().write_log(3, format, ##_VA_ARGS__); log::get_instance().flush();}

#endif