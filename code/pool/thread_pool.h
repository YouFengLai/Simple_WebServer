#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#include "../lock/locker.h"
#include <list>
#include <pthread.h>
#include <exception>
#include <memory>

template<typename T>
class thread_pool{
public:
    thread_pool(int _max_requests = 10000, int _pthread_num = 8);
    ~thread_pool();
    bool append(T* request);

private:
    static void* worker(void* arg);
    void run();

private:
    int max_requests;
    int pthread_num;
    bool stop;
    std::unique_ptr<pthread_t []> threads;
    std::list<T*> Request_queue;
    locker queue_lock;
    sem queue_state;

};

template<typename T>
thread_pool<T>::thread_pool(int _max_requests, int _pthread_num) :
    max_requests(_max_requests), pthread_num(_pthread_num)
    {
        if(max_requests <= 0 || pthread_num <= 0)
            throw std::exception();
        
        threads = new pthread_t[pthread_num];
        stop = false;

        for(int i = 0; i < pthread_num; i ++)
        {
            if(pthread_create(threads + i, NULL, worker, this) != 0){
                delete[] threads;
                throw std::exception();
            }

            if(pthread_detach(threads[i]) != 0){
                delete[] threads;
                throw std::exception();
            }
        }

        
    }

template<typename T>
thread_pool<T>::~thread_pool(){
    stop = true;
    delete[] threads;
}

template<typename T>
void* thread_pool<T>::worker(void* arg){
    thread_pool* pool = static_cast<thread_pool>(arg);
    pool->run();
    return pool;
}

template<typename T>
void thread_pool<T>::run()
{
    while(true){
        queue_state.wait();
        queue_lock.lock();
        if(Request_queue.empty()){
            queue_lock.unlock();
            continue;
        }

        auto request = Request_queue.front();
        Request_queue.pop_front();
        queue_lock.unlock();

        if(!request) continue;

        // 待完成..........
        // request->process();
        // 待完成..........
    }
}

template<typename T>
bool thread_pool<T>::append(T* request){
    queue_lock.lock();
    if(Request_queue.size() >= max_requests){
        queue_lock.unlock();
        return false;
    }

    Request_queue.push_back(request);
    queue_state.post();
    queue_lock.unlock();

    return true;
}

#endif