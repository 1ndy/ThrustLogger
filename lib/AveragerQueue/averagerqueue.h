#ifndef AVERAGERQUEUE_H
#define AVERAGERQUEUE_H

template <class T>
class AveragerQueue
{
public:
    AveragerQueue(int size, T zero);
    ~AveragerQueue();

    void add(T val);
    T average();

private:
    int size;
    T *data;
    int lastPos;

    T sum();
};

#include "averagerqueue.h"

template <class T>
AveragerQueue<T>::AveragerQueue(int size, T zero)
{
    this->size = size;
    this->data = new T[size];
    for(int i = 0; i < size; i++) {
        data[i] = zero;
    }
    this->lastPos = 0;
}

template <class T>
AveragerQueue<T>::~AveragerQueue() {
    delete[] data;
}

template <class T>
void AveragerQueue<T>::add(T val) {
    data[lastPos] = val;
    lastPos += 1;
    lastPos %= this->size;
}

template <class T>
T AveragerQueue<T>::sum() {
    T sum = 0;
    for(int i = 0; i < size; i++) {
        sum += data[i];
    }
    return sum;
}

template <class T>
T AveragerQueue<T>::average() {
    return (T)(sum() / size);
}

#endif // AVERAGERQUEUE_H
