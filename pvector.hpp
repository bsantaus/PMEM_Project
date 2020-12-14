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
class PVector {
    public:

        PVector();
        PVector(size_t init_cap);
        ~PVector();
        
        size_t cap() { return capacity; }
        void resize(size_t size);
        T& get(int index) { return arr[index]; }
        void set(int index, T value) { arr[index] = value; }
        T* begin() { return &arr[0]; }
    
    private:
        size_t capacity;
        persistent_ptr<T[]> arr;
};


template <class T>
PVector<T>::PVector() {
    auto pool = pool_by_vptr(this);
    transaction::run(pool, [this] {
        arr = make_persistent<T[]>(10);
    });
    capacity = 10;
}

template <class T>
PVector<T>::PVector(size_t init_cap) {
    auto pool = pool_by_vptr(this);
    transaction::run(pool, [this, init_cap] {
        arr = make_persistent<T[]>(init_cap);
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
        for (int i = 0; i < capacity; i++) new_arr[i] = arr[i];
        delete_persistent<T[]>(arr, capacity);
        arr = new_arr;
    });
    capacity = new_size;
}

