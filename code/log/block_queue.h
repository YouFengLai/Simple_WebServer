#ifndef __BLOCK_QUEUE_H__
#define __BLOCK_QUEUE_H__

#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include <sys/time.h>
#include "../lock/locker.h"

/*
*  主要操作：
*  push 和 pop 构成生产者消费者模型
*  每次对阻塞队列进行操作都上锁保证线程安全
*/

template<typename T>
class block_queue{
public:
    block_queue(int max_size = 1000){
        if(max_size <= 0) 
            exit(-1);
        
        _max_size = max_size;
        _array = new T[_max_size];
        _size = 0;
        _front = -1;
        _back = -1;
    }

    ~block_queue(){
        _mutex.lock();
        if(_array)
            delete[] _array;
        _mutex.unlock();
    }

    void clear();
    bool full();
    bool empty();
    int get_size();
    int get_max_size(); 
    bool get_front(T& value);
    bool get_back(T& value);
    bool push(const T& item);
    bool pop(T& item);

private:
    locker _mutex;
    condition _cond;

    T* _array;
    int _size;
    int _max_size;
    int _front;
    int _back;
};

template<typename T>
void block_queue<T>::clear(){
    _mutex.lock();
    _size = 0;
    _front = -1;
    _back = -1;
    _mutex.unlock();
}

template<typename T>
bool block_queue<T>::full(){
    _mutex.lock();
    if(_size >= _max_size){
        _mutex.unlock();
        return true;
    }

    _mutex.unlock();

    return false;
}

template<typename T>
bool block_queue<T>::empty(){
    _mutex.lock();
    if(_size == 0){
        _mutex.unlock();
        return true;
    }

    _mutex.unlock();
    return false;
}

template<typename T>
bool block_queue<T>::get_front(T& value){
    if(empty()) return false;

    _mutex.lock();
    value = _array[_front];
    _mutex.unlock();

    return true;
}

template<typename T>
bool block_queue<T>::get_back(T& value){
    if(empty()) return false;

    _mutex.lock();
    value = _array[_back];
    _mutex.unlock();

    return true;
}

template<typename T>
int block_queue<T>::get_size(){
    _mutex.lock();
    int ret = _size;
    _mutex.unlock();

    return ret;
}

template<typename T>
int block_queue<T>::get_max_size(){
    _mutex.lock();
    int ret = _max_size;
    _mutex.unlock();
    
    return ret;
}

template<typename T>
bool block_queue<T>::push(const T& item)
{
    _mutex.lock();
    if(_size >= _max_size){
        _cond.broadcast();
        _mutex.unlock();

        return false;
    }

    _back = (_back + 1) % _max_size;
    _array[_back] = item;
    _size ++;
    _cond.broadcast();

    _mutex.unlock();

    return true;
}

template<typename T>
bool block_queue<T>::pop(T& item)
{
    _mutex.lock();
    if(_size <= 0){
        if(_cond.wait(_mutex.get()) == 0)
        {
            _mutex.unlock();
            return false;
        }
    }

    _front = (_front + 1) % _max_size;
    item = _array[_front];
    _size --;
    _mutex.unlock();

    return true;
}

#endif