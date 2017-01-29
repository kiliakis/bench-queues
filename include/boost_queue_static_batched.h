#pragma once

#include <boost/lockfree/spsc_queue.hpp>
#include <stdio.h>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

#ifdef QUEUE_SIZE
static int const queue_size = QUEUE_SIZE;
#else
static int const queue_size = 10001;
#endif
static int const buffer_size = 100;
//#define queue_size 10000
//const size_t queue_size = 2000;
// using namespace boost::lockfree;
template<class T>
class circ_buffer {
private:
    boost::lockfree::spsc_queue<T, boost::lockfree::capacity<queue_size> > *queue;
    // boost::lockfree::spsc_queue<T> *queue;
    T buffer[buffer_size];
    uint32_t batch, size;
    uint32_t sleep;
    int i;
    uint32_t limit;
public:
    // int pushes = 0, pops = 0;
    double sleep_time = 0;
    circ_buffer(uint32_t _size, uint32_t _batch, uint32_t _sleep_nanosec = 100)
    {
        size = _size;
        sleep = _sleep_nanosec;
        batch = _batch;
        i = 0;
        limit = buffer_size;
        // buffer = new T[buffer_size];
        // queue = new boost::lockfree::spsc_queue<T>(size);
        queue = new boost::lockfree::spsc_queue<T, boost::lockfree::capacity<queue_size> >();
    };

    ~circ_buffer() {};
    bool push(T const &val);
    void push_final();
    bool pop(T &val) const;

    template<class Functor>
    int consume_batch(const Functor &f) const;

    template<class Functor>
    int consume_all(const Functor &f) const;
};


template<class T>
bool circ_buffer<T>::push(T const &val)
{
    buffer[i] = val;
    i++;
    if (i == buffer_size) {
        while (i != 0)
            i -= queue->push(buffer, i);
    }

    // if (i == limit) {
    //     while ((limit = queue->push(buffer, buffer_size)) == 0)
    //         ;
    //     i = 0;
    // }

    return true;
}

template<class T>
void circ_buffer<T>::push_final()
{
    while (i != 0)
        i -= queue->push(buffer, i);
}


template<class T>
bool circ_buffer<T>::pop(T &val) const
{
    return queue->pop(val);
}

template<class T> template<class Functor>
int circ_buffer<T>::consume_batch(const Functor &f) const
{
    if (queue->read_available() >= batch)
        return queue->consume_all(f);
    return 0;
}


template<class T> template<class Functor>
int circ_buffer<T>::consume_all(const Functor &f) const
{
    queue->consume_all(f);
}
