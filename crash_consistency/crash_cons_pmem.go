package main

import (
   "flag"
   "math"
   "github.com/vmware/go-pmem-transaction/pmem"
   "github.com/vmware/go-pmem-transaction/transaction"
)
var pmemFile = flag.String("file", "pmemtest", "pmemtest file name")

type PVector struct {
	capacity int32
	arr []int32
}

func Init(cap int32) *PVector {
    var pv *PVector
    pv = (*PVector)(pmem.Get("pvec", pv))

    var c int = 2500

    if pv == nil {
        pv = (*PVector)(pmem.New("pvec", pv))
        pv.capacity = cap
        pv.arr = pmem.Make("pvec_slice", pv.arr, c).([]int32)
        return pv
    }

    return pv
} 

func (pv *PVector) cap() int32 {
    return pv.capacity
}

func (pv *PVector) get(i int32) int32 {
    return pv.arr[i]
}

func (pv *PVector) set(i int32, v int32) {
    pv.arr[i] = v
}

func (pv *PVector) resize(sz int32) {
    var temp []int32 = make([]int32, pv.capacity)
    for i := int32(0); i < pv.capacity; i++ {
        temp[i] = pv.arr[i]
    }

    pmem.Delete("pvec_slice")
    pv.arr = pmem.Make("pvec_slice", pv.arr, sz).([]int32)

    var ec int32
    if sz > pv.capacity {
        ec = pv.capacity
    } else {
        ec = sz
    }

    for i := int32(0); i < ec; i++ {
        pv.arr[i] = temp[i]
    }
    pv.capacity = sz
}

func (pv *PVector) reset(sz int32) {
    pmem.Delete("pvec_slice")
    pv.arr = pmem.Make("pvec_slice", pv.arr, sz).([]int32)
}


var iterations int32 = 50000
var MAX_CAP int32 = 50000
var default_cap int32 = 2500

var pv *PVector
var iter *int32

func main() {
    flag.Parse()
    firstInit := pmem.Init(*pmemFile)
	if firstInit {
        println("new")
        pv = Init(default_cap)
        iter = (*int32)(pmem.New("iter", iter))
        *iter = 0
	} else {
        pv = (*PVector)(pmem.Get("pvec", pv))
        iter = (*int32)(pmem.Get("iter", iter))

        if pv == nil {
            println("new")
            pv = Init(default_cap)
        }
        if iter == nil {
            iter = (*int32)(pmem.New("iter", iter))
            *iter = 0
        }
        if *iter == 50000 {
            *iter = 0
            pv.reset(default_cap)
        }
    }
    
	for ; *iter < iterations; {
        txn("undo") {
            var sum int32 = 0
            for i := int32(0); i < pv.cap(); i++ {
                var val int32
                val = int32(math.Ceil(float64(math.Sqrt(float64(pv.get(i))) + float64(((i + 1) * pv.cap()) % MAX_CAP))))
                pv.set(i, val)
                sum += val
            }
            var newcap int32 = sum % MAX_CAP + 1
            if (newcap <= 0) {
                newcap = default_cap
            }
            pv.resize(newcap)
            (*iter)++
        }
	}
}