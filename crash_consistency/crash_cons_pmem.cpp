#include <iostream>
#include <fstream>
#include <algorithm>
#include <math.h>
#include <pthread.h>
#include <libpmemobj/atomic_base.h>
#include <libpmemobj++/make_persistent.hpp>
#include <libpmemobj++/make_persistent_atomic.hpp>
#include <libpmemobj++/transaction.hpp>
#include <libpmemobj++/utils.hpp>
#include <stdint.h>
#include "../pvector.hpp"
#define CREATE_MODE_RW (S_IWUSR | S_IRUSR)


using namespace std;
using namespace pmem::obj;

const char *LAYOUT = "fkrcplus";
const int64_t iterations = 2000;
const int64_t MAX_CAP = 50000;
const int64_t default_cap = 2500;
const uint64_t PMEM_FILE_SZ = (uint64_t)3*1024*1024*1024;

struct root {
    PVector<int64_t> arr;
    int64_t iter;
    int init = 0;
};

pmem::obj::pool<root> pop;

bool file_exists(const std::string &path) {
   std::ifstream f(path.c_str());
   return f.good();
}

int main(int argc, char **argv)
{

    std::string path = "pmemtest";

    if (file_exists(path)) {
        cout << "attempting to open file" << endl;
        pop = pmem::obj::pool<root>::open(path, LAYOUT);
        cout << "opened file successfully" << endl;
        if (pop.root()->init != 72) {
            cout << "making new root objs" << endl;
            transaction::run(pop, [&] {
                pop.root()->iter = 0;
                pop.root()->arr.reset(default_cap);
            });
        }
        if (pop.root()->iter == iterations) {
            cout << "resetting root objs" << endl;
            transaction::run(pop, [&] {
                pop.root()->iter = 0;
                pop.root()->arr.reset(default_cap);
            });
        }
    } else {
        cout << "attempting to create new file" << endl;
        pop = pmem::obj::pool<root>::create(path, LAYOUT, PMEM_FILE_SZ, CREATE_MODE_RW);
        transaction::run(pop, [&] {
            pop.root()->iter = 0;
            pop.root()->arr.reset(default_cap);
            pop.root()->init = 72;
        });
    }

    cout << "calling cap" << endl;
    cout << pop.root()->arr.cap() << endl;
    cout << "called cap" << endl; 
    auto root = pop.root();

    while (root->iter < iterations) {
        transaction::run(pop, [&root] {
            int64_t sum = 0;
            for (int64_t i = 0; i < root->arr.cap(); i++) {
                int64_t val = (int64_t)(ceil(sqrt(root->arr.get(i)) + (((i + 1) * root->arr.cap()) % MAX_CAP)));
                root->arr.set(i, val);
                sum = sum + val;
            }
            int64_t newcap = (sum % MAX_CAP) + 1;
            if (newcap <= 0) {
                newcap = default_cap;
            }
            root->arr.resize(newcap);
            root->iter += 1;
        });
    }
    pop.close();

    cout << "done" << endl;
    return 0;
}
