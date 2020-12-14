// The Computer Language Benchmarks Game
// https://salsa.debian.org/benchmarksgame-team/benchmarksgame/
//
// Contributed by Dave Compton
// Based on "fannkuch-redux C gcc #5", contributed by Jeremy Zerfas
// which in turn was based on the Ada program by Jonathan Parker and 
// Georg Bauhaus which in turn was based on code by Dave Fladebo, 
// Eckehard Berns, Heiner Marxen, Hongwei Xi, and The Anh Tran and 
// also the Java program by Oleg Mazurov.

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
#include "pvector.hpp"
#define CREATE_MODE_RW (S_IWUSR | S_IRUSR)


using namespace std;
using namespace pmem::obj;

const char *LAYOUT = "fkrcplus";


class Permutation 
{
  public:
    // Permutation(int n, int64_t start);
    ~Permutation();
    void initialize(int n, int64_t start);
    void advance();
    int64_t countFlips() const;

  private:
     persistent_ptr<PVector <int>> count;
     persistent_ptr<PVector <int8_t>> current;
     size_t len;

};

struct root {
   persistent_ptr<PVector<Permutation>> perms;
   persistent_ptr<int64_t[]> fact;
};

pmem::obj::pool<root> pop;
persistent_ptr<int64_t[]> fact;

void initializeFact(int n)
{
    fact[0] = 1;
    for (auto i = 1; i <= n; ++i)
        fact[i] = i * fact[i - 1];
}

Permutation::~Permutation() {
    auto pool = pool_by_vptr(this);
    transaction::run(pool, [this] {
        delete_persistent<PVector<int>>(count);
        delete_persistent<PVector<int8_t>>(current);
    });
}

// 
// Initialize the current value of a permutation
// and the cycle count values used to advance .
// 
void Permutation::initialize(int n, int64_t start)
{
    if (count == NULL) {
        auto pool = pool_by_vptr(this);
        transaction::run(pool, [this, n] {
            count = make_persistent<PVector<int>>(n);
            current = make_persistent<PVector<int8_t>>(n);
        });
    } else {
        count->resize(n);
        current->resize(n);
    }

    len = n;

    // Initialize count 
    for (auto i = n - 1; i >= 0; --i) 
    {
        auto d = start / fact[i];
        start = start % fact[i];
        count->get(i) = d;
    }

    // Initialize current.
    for (auto i = 0; i < n; ++i)
        current->get(i) = i;

    for (auto i = n - 1; i >= 0; --i) 
    {
        auto d = count->get(i);
        auto b = current->begin();
        rotate(b, b + d, b + i + 1);
    }
}

//
// Advance the current permutation to the next in sequence.
// 
void Permutation::advance()
{
    for (auto i = 1; ;++i) 
    {
        // Tried using std::rotate here but that was slower.
        auto first = current->get(0);
        for (auto j = 0; j < i; ++j)
            current->get(j) = current->get(j + 1);
        current->get(i) = first;

        ++(count->get(i));
        if (count->get(i) <= i)
            break;
        count->get(i) = 0;
    }
}

//
// Count the flips required to flip 0 to the front of the vector.
//
// Other than minor cosmetic changes, the following routine is
// basically lifted from "fannkuch-redux C gcc #5"
//
inline int64_t Permutation::countFlips() const
{
    const auto n = current->cap();
    auto flips = 0;
    auto first = current->get(0);
    if (first > 0) 
    {
        flips = 1;

        int8_t temp[n];
        // Make a copy of current to work on. 
        for (size_t i = 0; i < n; ++i)
            temp[i] = current->get(i);


        // Flip temp until the element at the first index is 0
        for (; temp[first] > 0; ++flips) 
        {
            // Record the newFirst and restore the old
            // first at its new flipped position.
            const int8_t newFirst = temp[first];
            temp[first] = first;

            if (first > 2) 
            {
                int64_t low = 1, high = first - 1;
                do 
                {
                    swap(temp[low], temp[high]);
                    if (!(low + 3 <= high && low < 16))
                        break;
                    ++low;
                    --high;
                } while (1);
            }
            // Update first to newFirst that we recorded earlier.
            first = newFirst;
        }
    }
    return flips;
}

bool file_exists(const std::string &path) {
   std::ifstream f(path.c_str());
   return f.good();
}

int main(int argc, char **argv)
{

    std::string path = "fkr_cpp";
    const auto n = atoi(argv[1]);


    if (file_exists(path)) {
        std::cout << "opening existing pmem file\n";
        pop = pmem::obj::pool<root>::open(path, LAYOUT);
    } else {
        std::cout << "opening new pmem file\n";
        pop = pmem::obj::pool<root>::create(path, LAYOUT, 1024*1024*64, CREATE_MODE_RW);
    }

    auto root = pop.root();

    transaction::run(pop, [&root, n] {
        root->fact = make_persistent<int64_t[]>(32);
        fact = root->fact;
        initializeFact(n);
        root->perms = make_persistent<PVector<Permutation>>();
    });

    // Compute some factorials for later use.

    // blockCount works best if it is set to a multiple of the number
    // of CPUs so that the same number of blocks gets distributed to
    // each cpu.  The computer used for development (Intel i7-4700MQ)
    // had 8 "CPU"s (4 cores with hyperthreading) so 8, 16 and 24 
    // all worked well.


    auto blockCount = 24;
    if (blockCount > fact[n])
        blockCount = 1;
    const int64_t blockLength = fact[n] / blockCount;
    cout << "waddup1" << endl;

    root->perms->resize(blockCount);

    cout << "waddup" << endl;


    int64_t maxFlips = 0, checksum = 0;

    // Iterate over each block.
    #pragma omp parallel for \
        reduction(max:maxFlips) \
        reduction(+:checksum)

    for (int64_t blockStart = 0;
         blockStart < fact[n]; 
         blockStart += blockLength) 
    {
        auto perm = ceil((float) blockStart / blockLength);
        // first permutation for this block.
        root->perms->get(perm).initialize(n, blockStart);

        // Iterate over each permutation in the block.
        auto index = blockStart;
        while (1) 
        {
            const auto flips = root->perms->get(perm).countFlips();

            if (flips) 
            {
                if (index % 2 == 0)
                    checksum += flips;
                else
                    checksum -= flips;

                if (flips > maxFlips)
                    maxFlips = flips;
            }

            if (++index == blockStart + blockLength)
                break;

            // next permutation for this block.
            root->perms->get(perm).advance();
        }
    }

    // Output the results to stdout.
    cout << checksum << endl;
    cout << "Pfannkuchen(" << n << ") = " << maxFlips << endl;

    transaction::run(pop, [&root] {
        delete_persistent<int64_t[]>(root->fact, 32);
        delete_persistent<PVector<Permutation>>(root->perms);
    });
    pop.close();

    return 0;
}
