#include <libpmemobj/atomic_base.h>
#include <boost/pool/object_pool.hpp>
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/make_persistent_array.hpp>
#include <libpmemobj++/make_persistent_atomic.hpp>
#include <libpmemobj++/transaction.hpp>
#include <iostream>
#include <libpmemobj++/utils.hpp>
#include <iterator>
#include <stdint.h>


using namespace std;
using namespace pmem::obj;


template <class T>
class PVector {
    public:

        PVector();
        PVector(size_t init_cap);
        ~PVector();
        
        size_t cap() { return capacity; }
        void resize(size_t size);
        T get(int index) { return arr[index]; }
        void set(int index, T value) { arr[index] = value; }
        T* begin() { return &arr[0]; }
        void reset(size_t sz);
    
    private:
        size_t capacity;
        persistent_ptr<T[]> arr;
};


template <class T>
PVector<T>::PVector() {
    auto pool = pool_by_vptr(this);
    transaction::run(pool, [this] {
        arr = make_persistent<T[]>(10);
        for (int i = 0; i < 10; i++) {
            arr[i] = 0;
        }
    });
    capacity = 10;
}

template <class T>
PVector<T>::PVector(size_t init_cap) {
    auto pool = pool_by_vptr(this);
    transaction::run(pool, [this, init_cap] {
        arr = make_persistent<T[]>(init_cap);
        for (int i = 0; i < init_cap; i++) {
            arr[i] = 0;
        }
    });
    capacity = init_cap;
}

template <class T>
PVector<T>::~PVector() {
    auto pool = pool_by_vptr(this);
    transaction::run(pool, [this] {
        delete_persistent<T[]>(arr, capacity);
    });
}

template <class T>
void PVector<T>::resize(size_t new_size) {
    auto pool = pool_by_vptr(this);
    transaction::run(pool, [this, new_size] {
        persistent_ptr<T[]> new_arr = make_persistent<T[]>(new_size);

        size_t elems_copied = new_size > capacity ? capacity : new_size;
        for (int i = 0; i < elems_copied; i++) new_arr[i] = arr[i];
        delete_persistent<T[]>(arr, capacity);
        arr = new_arr;
    });
    capacity = new_size;
}

template <class T>
void PVector<T>::reset(size_t new_size) {
    auto pool = pool_by_vptr(this);
    transaction::run(pool, [this, new_size] {
        delete_persistent<T[]>(arr, capacity);
        arr = make_persistent<T[]>(new_size);
        for (int i = 0; i < new_size; i++) arr[i] = 0;
    });
    capacity = new_size;
}

