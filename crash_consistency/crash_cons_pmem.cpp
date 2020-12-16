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
const int64_t iterations = 50000;
const int64_t MAX_CAP = 50000;
const int64_t default_cap = 2500;
const uint64_t PMEM_FILE_SZ = (uint64_t)3*1024*1024*1024;

struct root {
    persistent_ptr<PVector<int64_t>> arr;
    int64_t iter;
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
        std::cout << "opening existing pmem file\n";
        pop = pmem::obj::pool<root>::open(path, LAYOUT);
        if (pop.root()->iter == iterations) {
            transaction::run(pop, [&] {
                pop.root()->iter = 0;
                pop.root()->arr->reset(default_cap);
            });
        }
    } else {
        std::cout << "opening new pmem file\n";
        pop = pmem::obj::pool<root>::create(path, LAYOUT, PMEM_FILE_SZ, CREATE_MODE_RW);
        transaction::run(pop, [&] {
            pop.root()->iter = 0;
            pop.root()->arr = make_persistent<PVector<int64_t>>(default_cap);
        });
    }

    auto root = pop.root();

    while (root->iter < iterations) {
        transaction::run(pop, [&root] {
            int64_t sum = 0;
            for (int64_t i = 0; i < root->arr->cap(); i++) {
                int64_t val = (int64_t)(ceil(sqrt(root->arr->get(i)) + (((i + 1) * root->arr->cap()) % MAX_CAP)));
                root->arr->set(i, val);
                sum = sum + val;
            }
            int64_t newcap = (sum % MAX_CAP) + 1;
            if (newcap <= 0) {
                newcap = default_cap;
            }
            root->arr->resize(newcap);
            root->iter += 1;
            if (root->iter == iterations) cout << sum << endl;
        });
    }
    pop.close();

    return 0;
}
