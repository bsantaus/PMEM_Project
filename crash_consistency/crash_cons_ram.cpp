#include <iostream>
#include <math.h>
#include <stdint.h>
#include "../rvector.hpp"

using namespace std;

const int64_t iterations = 50000;
const int64_t MAX_CAP = 50000;
const int64_t default_cap = 2500;

int main(int argc, char **argv)
{
    RVector<int64_t> rv(default_cap);

    for (int64_t j = 0; j < iterations; j++) {
        int64_t sum = 0;
        for (int64_t i = 0; i < rv.cap(); i++) {
            int64_t val = (int64_t)(ceil(sqrt(rv.get(i)) + (((i + 1) * rv.cap()) % MAX_CAP)));
            rv.set(i, val);
            sum = sum + val;
        }
        int64_t newcap = (sum % MAX_CAP) + 1;
        if (newcap <= 0) {
            newcap = default_cap;
        }
        rv.resize(newcap);
        if (j == iterations - 1) cout << sum << endl;
    }

    return 0;
}
