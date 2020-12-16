package main

import (
   "math"
)

type RVector struct {
	capacity int32
	arr []int32
}

func Init(cap int32) *RVector {
	rv := new(RVector)
	rv.capacity = cap
	rv.arr = make([]int32, cap)
    return rv
} 

func (rv *RVector) cap() int32 {
    return rv.capacity
}

func (rv *RVector) get(i int32) int32 {
    return rv.arr[i]
}

func (rv *RVector) set(i int32, v int32) {
    rv.arr[i] = v
}

func (rv *RVector) resize(sz int32) {
    var newarr []int32 = make([]int32, sz)

    var ec int32
    if sz > rv.capacity {
        ec = rv.capacity
    } else {
        ec = sz
    }

    for i := int32(0); i < ec; i++ {
        newarr[i] = rv.arr[i]
	}
	
	rv.arr = nil
	rv.arr = newarr
	rv.capacity = sz
}   

var iterations int32 = 4000
var MAX_CAP int32 = 50000
var default_cap int32 = 2500

func main() {
	rv := Init(default_cap)

	for j := int32(0); j < iterations; j++ {
		var sum int32 = 0
		for i := int32(0); i < rv.cap(); i++ {
			var val int32
			val = int32(math.Ceil(float64(math.Sqrt(float64(rv.get(i))) + float64(((i + 1) * rv.cap()) % MAX_CAP))))
			rv.set(i, val)
			sum += val
		}
		var newcap int32 = sum % MAX_CAP + 1
		if (newcap <= 0) {
			newcap = default_cap
		}
		rv.resize(newcap)
	}

	println("done")
}