#include "log.h"
#include "block_queue.h"
#include <string>
#include <sys/time.h>
#include <pthread.h>
#include <stdarg.h>
#include <memory>
#include <cstring>


log::log()
{
    _async = false;
    _log_line_count = 0;
}

log::~log()
{
    if(_fp != NULL)
        fclose(_fp);
}

bool log::init(const char* file_name, bool close_log, unsigned int log_buf_size, unsigned int max_lines, int max_queue_size)
{
    if(max_queue_size >= 1)
    {
        _async = true;
        _log_queue = std::shared_ptr<block_queue<std::string>>(new block_queue<std::string>(max_queue_size));
        pthread_t tid;
        pthread_create(&tid, NULL, flush_log_thread, NULL);
    }

    _close_log = close_log;
    _log_buf_size = log_buf_size;
    _buf = new char[_log_buf_size];
    memset(_buf, '\0', sizeof _buf);
    _max_lines = max_lines;

    time_t t = time(NULL);
    struct tm* sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;

    const char* fp = std::strrchr(file_name, '/');
    char log_full_name[256] = {0};

    if(fp == nullptr)
        snprintf(log_full_name, 255, "%d_%02d_%02d_%s", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, file_name);
    else{
        strcpy(log_name, fp + 1);
        strncpy(dir_name, file_name, fp - file_name + 1);
        snprintf(log_full_name, 255, "%d_%02d_%02d_%s", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, log_name);
    }

    _days = my_tm.tm_mday;
    _fp = fopen(log_full_name, "a");

    if(_fp == nullptr)
        return false;
    

    return true;

}

void log::write_log(unsigned int level, const char* format, ...)
{
    struct timeval now = {0, 0};
    gettimeofday(&now, NULL);
    time_t t = time(NULL);
    struct tm* sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;

    char s[16] = {0};
    switch(level)
    {
        case 0:
            strcpy(s, "[debug]:");
            break;
        case 1:
            strcpy(s, "[info]:");
            break;
        case 2:
            strcpy(s, "[warn]:");
            break;
        case 3:
            strcpy(s, "[error]:");
            break;
        default:
            strcpy(s, "[info]:");
            break;
    }

    _mutex.lock();
    _log_line_count ++;

    if(my_tm.tm_mday != _days ||  _log_line_count % _max_lines == 0)
    {
        char new_log[256] = {0};
        fflush(_fp);
        fclose(_fp);
        char tail[16] = {0};

        snprintf(tail, 16, "%d_%02d_%02d_%s", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);

        if(my_tm.tm_mday != _days){
            snprintf(new_log, 255, "%s%s%s", dir_name, tail, log_name);
            _days = my_tm.tm_mday;
            _log_line_count = 0;
        }else  
            snprintf(new_log, 255, "%s%s%s.%lld", dir_name, tail, log_name, _log_line_count / _max_lines);

        _fp = fopen(new_log, "a");
    }

    _mutex.unlock();

    va_list args;
    va_start(args, format);

    std::string log_str;

    _mutex.lock();

    int n = snprintf(_buf, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ",
                     my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday,
                     my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, now.tv_usec, s);
    
    int m = vsnprintf(_buf + n, _log_buf_size - n - 1, format, args);
    _buf[n + m] = '\n';
    _buf[n + m + 1] = '\0';
    log_str = _buf;

    _mutex.unlock();

    if(_async && !_log_queue->full())
        _log_queue->push(log_str);
    else{
        _mutex.lock();
        fputs(log_str.c_str(), _fp);
        _mutex.unlock();
    }

    va_end(args); 
}

void log::flush()
{
    _mutex.lock();
    fflush(_fp);
    _mutex.unlock();
}