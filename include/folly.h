#pragma once

#include <ProducerConsumerQueue.h>
#include <stdio.h>
#include <thread>         // std::this_thread::sleep_for
#include <chrono>         // std::chrono::seconds

template<class T>
class circ_buffer {
private:
    folly::ProducerConsumerQueue<T> *queue;
    uint32_t piece_size, size;
    uint32_t sleep;
public:
    int pushes = 0, pops = 0;
    double sleep_time = 0;
    circ_buffer(uint32_t size, uint32_t _piece_size)
    {
        sleep = 100;
        piece_size = _piece_size;
        queue = new folly::ProducerConsumerQueue<T>(size);
    };

    ~circ_buffer() {};
    bool push(const T &val) const;
    bool pop(T &val) const;
    T *top() const;
    void pop() const;
    // int is_ready(T *&val);

};


template<class T>
bool circ_buffer<T>::push(const T &val) const
{
    return queue->write(val);
    // while (!queue->write(val))
    //     std::this_thread::sleep_for(std::chrono::nanoseconds(sleep));
}


template<class T>
bool circ_buffer<T>::pop(T &val) const
{
    return queue->read(val);
}

template<class T>
T *circ_buffer<T>::top() const
{
    return queue->frontPtr();
}

template<class T>
void circ_buffer<T>::pop() const
{
    queue->popFront();
    // pops++;
}


// it is not working because frontPtr needs to be updated regularly
// template<class T>
// int circ_buffer<T>::is_ready(T *&val)
// {
//     const int size = queue->sizeGuess();
//     if (size >= piece_size) {
//         val = queue->frontPtr();
//         return size;
//     } else {
//         return 0;
//     }
//     // val = queue->frontPtr();
//     // if (val) {
//     //     queue->popFront();
//     //     pops++;
//     //     return true;
//     // } else {
//     //     return false;
//     // }

// }
