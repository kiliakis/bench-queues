#pragma once

#include <readerwriterqueue.h>
#include <stdio.h>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

#ifdef QUEUE_SIZE
static int const queue_size = QUEUE_SIZE;
#else
static int const queue_size = 100000;
#endif

template<class T>
class circ_buffer {
private:
    moodycamel::ReaderWriterQueue<T> *queue;
    uint32_t piece_size, size;
    uint32_t sleep;

public:
    int pushes = 0, pops = 0;
    double sleep_time = 0;
    circ_buffer(uint32_t size, uint32_t _piece_size)
    {
        sleep = 100;
        piece_size = _piece_size;
        queue = new moodycamel::ReaderWriterQueue<T>(size);

    };

    ~circ_buffer() {};
    bool push(const T &val) const;
    bool pop(T &val) const;
};


template<class T>
bool circ_buffer<T>::push(const T &val) const
{
    return queue->try_enqueue(val);
    // std::this_thread::sleep_for(std::chrono::nanoseconds(sleep));
}

template<class T>
bool circ_buffer<T>::pop(T &val) const
{
    return queue->try_dequeue(val);
}
