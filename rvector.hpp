#include <libpmemobj/atomic_base.h>
#include <boost/pool/object_pool.hpp>
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/make_persistent_array.hpp>
#include <libpmemobj++/make_persistent_atomic.hpp>
#include <libpmemobj++/transaction.hpp>
#include <iostream>
#include <libpmemobj++/utils.hpp>
#include <iterator>


using namespace std;
using namespace pmem::obj;


template <class T>
class RVector {
    public:

        RVector();
        RVector(size_t init_cap);
        ~RVector();
        
        size_t cap() { return capacity; }
        void resize(size_t size);
        T get(int index) { return arr[index]; }
        void set(int index, T value) { arr[index] = value; }
        T* begin() { return &arr[0]; }
    
    private:
        size_t capacity;
        T *arr;
};


template <class T>
RVector<T>::RVector() {
    arr = new T[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = 0;
    }
    capacity = 10;
}

template <class T>
RVector<T>::RVector(size_t init_cap) {
    arr = new T[init_cap];
    for (int i = 0; i < init_cap; i++) {
        arr[i] = 0;
    }
    capacity = init_cap;
}

template <class T>
RVector<T>::~RVector() {
    delete[] arr;
}

template <class T>
void RVector<T>::resize(size_t new_size) {
    T *newarr = new T[new_size];
    size_t elems_copied = new_size > capacity ? capacity : new_size;
    for (size_t i = 0; i < elems_copied; i++) newarr[i] = arr[i];
    delete[] arr;
    arr = newarr;
    capacity = new_size;
}

