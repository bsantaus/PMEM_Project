/*
* The Computer Language Benchmarks Game
* https://salsa.debian.org/benchmarksgame-team/benchmarksgame/
*
* contributed by Oleg Mazurov, June 2010
* flag.Arg hack by Isaac Gouy
*
*/

package main

import (
	"fmt"
	"runtime"
	"flag"
	"strconv"
	"github.com/vmware/go-pmem-transaction/pmem"
	"github.com/vmware/go-pmem-transaction/transaction"
)

type Result struct {
	maxFlips int
	checkSum int
}

var (
	NCHUNKS = 720
	CHUNKSZ = 0
	NTASKS  = 0
	pmemFile = flag.String("file", "fkr_pmem", "fkr_pmem file name")
)
var n = 12
var Fact []int
var p_g [][]int
var pp_g [][]int
var count_g [][]int

func fannkuch( idxMin int, routine int, ch chan Result ) {

	idxMax := idxMin + CHUNKSZ
	if idxMax < Fact[n] {
		go fannkuch( idxMax, routine + 1, ch )
	} else {
		idxMax = Fact[n]
	}

	p     := p_g[routine]
    pp    := pp_g[routine]
	count := count_g[routine]

	// first permutation
	for i := 0; i<n; i++ {
		p[i] = i
	}
	for i, idx := n-1, idxMin; i>0; i-- {
		d := idx / Fact[i]
		count[i] = d
		idx = idx % Fact[i]

		copy( pp, p )
		for j := 0; j <= i; j++ {
		if j+d <= i {
				p[j] = pp[j+d]
		} else {
				p[j] = pp[j+d-i-1]
		}
		}
	}

	maxFlips := 1
	checkSum := 0

	for idx, sign := idxMin, true; ; sign = !sign {

		// count flips
		first := p[0]
	if first != 0 {
		flips := 1
		if p[first] != 0 {
		copy( pp, p )
		p0 := first
			for {
			flips++
			for i, j := 1, p0-1; i < j; i, j = i+1, j-1 {
				pp[i], pp[j] = pp[j], pp[i]
			}
			t := pp[p0]
			pp[p0] = p0
			p0 = t
			if pp[p0] == 0 {
				break
			}
			}
		}
		if maxFlips < flips {
		maxFlips = flips
		}
		if sign {
		checkSum += flips
		} else {
		checkSum -= flips
		}
	}

	if idx++; idx == idxMax {
		break
	}

	// next permutation
	if sign {
		p[0], p[1] = p[1], first
	} else {
		p[1], p[2] = p[2], p[1]
		for k := 2;; k++ {
			if count[k]++; count[k] <= k {
			break
		}
			count[k] = 0
		for j:=0; j<=k; j++ {
			p[j] = p[j+1]
		}
		p[k+1] = first
		first = p[0]
		}
	}
	}

	ch <- Result{ maxFlips, checkSum }
}

func printResult( n int, res int, chk int ) {
	fmt.Printf("%d\nPfannkuchen(%d) = %d\n", chk, n, res)
}

func validate_or_create(slice [][]int, name string) [][]int {
	arr := slice
	if arr == nil {
		println("array didnt exist")
		arr = pmem.Make(name, arr, NTASKS).([][]int)
	}

	if len(arr) != NTASKS {
		println("incorrect length")
		for i := range arr {
			pmem.Delete(name + strconv.Itoa(i))
		}
		pmem.Delete(name)
		arr = pmem.Make(name, arr, NTASKS).([][]int)
	}

	if arr[0] == nil {
		println("creating inner arrays")
		for i := range arr {
			arr[i] = pmem.Make(name + strconv.Itoa(i), arr[i], n).([]int)
		} 
	}

	if len(arr[0]) != n {
		for i := range arr {
			pmem.Delete(name + strconv.Itoa(i))
			arr[i] = pmem.Make(name + strconv.Itoa(i), arr[i], n).([]int)
		}
	}

	return arr
}

func main() {
	flag.Parse()

	if flag.NArg() > 0 { n,_ = strconv.Atoi( flag.Arg(0) ) }
	runtime.GOMAXPROCS(4)

	firstInit := pmem.Init(*pmemFile)
	if firstInit {
		Fact = pmem.Make("fact", Fact, n+1).([]int)
		txn("undo") {
			Fact[0] = 1
			for i := 1; i<len(Fact); i++ {
				Fact[i] = Fact[i-1] * i
			}
		}

		CHUNKSZ = (Fact[n] + NCHUNKS - 1) / NCHUNKS
		CHUNKSZ += CHUNKSZ%2
		NTASKS = (Fact[n] + CHUNKSZ - 1) / CHUNKSZ

		p_g = pmem.Make("p_g", p_g, NTASKS).([][]int)
		pp_g = pmem.Make("pp_g", pp_g, NTASKS).([][]int)
		count_g = pmem.Make("count_g", count_g, NTASKS).([][]int)

		txn("undo") {
			for i := range p_g {
				p_g[i] = pmem.Make("p_g" + strconv.Itoa(i), p_g[i], n).([]int)
				pp_g[i] = pmem.Make("pp_g" + strconv.Itoa(i), pp_g[i], n).([]int)
				count_g[i] = pmem.Make("count_g" + strconv.Itoa(i), count_g[i], n).([]int)
			}
		}
	} else {
		Fact = pmem.GetSlice("fact", Fact).([]int)

		if Fact == nil {
			Fact = pmem.Make("fact", Fact, n+1).([]int)
		}

		if len(Fact) != n+1 {
			pmem.Delete("fact")
			Fact = pmem.Make("fact", Fact, n+1).([]int)
		}

		if Fact[n] == 0 {
			txn("undo") {
				Fact[0] = 1
				for i := 1; i<len(Fact); i++ {
					Fact[i] = Fact[i-1] * i
				}
			}
		}

		CHUNKSZ = (Fact[n] + NCHUNKS - 1) / NCHUNKS
		CHUNKSZ += CHUNKSZ%2
		NTASKS = (Fact[n] + CHUNKSZ - 1) / CHUNKSZ
		
		p_g = pmem.GetSlice("p_g", p_g).([][]int)
		pp_g = pmem.GetSlice("pp_g", pp_g).([][]int)
		count_g = pmem.GetSlice("count_g", count_g).([][]int)

		txn("undo") {
			p_g = validate_or_create(p_g, "p_g")
			pp_g = validate_or_create(pp_g, "pp_g")
			count_g = validate_or_create(count_g, "count_g")
		}
	}
	ch := make(chan Result, NTASKS)

	go fannkuch(0, 0, ch)
	
	res := 0
	chk := 0
	for i := 0; i<NTASKS; i++ {
		r := <-ch
		if res < r.maxFlips {
			res = r.maxFlips
		}
		chk += r.checkSum
	}

	printResult( n, res, chk )
}
