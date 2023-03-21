#ifndef __LOCKER_H__
#define __LOCKER_H__

#include <pthread.h>
#include <exception>
#include <semaphore.h>

class locker{
public:
    locker(){
        if(pthread_mutex_init(&mutex, NULL) != 0)
            throw std::exception();
    }

    bool lock(){
        return pthread_mutex_lock(&mutex) == 0;
    }

    bool unlock(){
        return pthread_mutex_unlock(&mutex) == 0;
    }

    ~locker(){
        pthread_mutex_destroy(&mutex);
    }

    pthread_mutex_t* get(){
        return &mutex;
    }

private:
    pthread_mutex_t mutex;
};

class condition{
public:
    condition(){
        if(pthread_cond_init(&cond, NULL) != 0)
            throw std::exception();
    }

    ~condition(){
        pthread_cond_destroy(&cond);
    }

    bool wait(pthread_mutex_t* _mutex){
        int res = 0;
        // pthread_mutex_lock(&mutex);
        res = pthread_cond_wait(&cond, _mutex);
        // pthread_mutex_unlock(&mutex);
        return res == 0;
    }

    bool singal(){
        return pthread_cond_signal(&cond) == 0;
    }

    bool broadcast(){
        return pthread_cond_broadcast(&cond) == 0;
    }
private:
    // pthread_mutex_t mutex;
    pthread_cond_t cond;
};

class sem{
public:
    sem(){
        if(sem_init(&m_sem, 0, 0) != 0)
            throw std::exception();
    }

    ~sem(){
        sem_destroy(&m_sem);
    }

    bool wait(){
        return sem_wait(&m_sem) == 0;
    }

    bool post(){
        return sem_post(&m_sem) == 0;
    }

private:
    sem_t m_sem;
};








#endif