all: binarytrees_um_cpp binarytrees_um_go binarytrees_um_gopmem binarytrees_pmem_cpp binarytrees_pmem_cleanup_gopmem binarytrees_pmem_reuse_gopmem binarytrees_um_cpp fannkuchredux_um_cpp fannkuchredux_um_go fannkuchredux_um_gopmem fannkuchredux_pmem_cpp fannkuchredux_pmem_gopmem nbody_um_cpp nbody_um_go nbody_um_gopmem nbody_pmem_cpp nbody_pmem_gopmem

binarytrees_um_cpp: binarytrees_um.cpp
	g++ -o binarytrees_um_cpp binarytrees_um.cpp -pthread

binarytrees_um_go: binarytrees_um.go
	go build -o binarytrees_um_go binarytrees_um.go

binarytrees_um_gopmem: binarytrees_um.go
	~/workspace/go-pmem/bin/go build -o binarytrees_um_gopmem binarytrees_um.go

binarytrees_pmem_cpp: binarytrees_pmem.cpp
	g++ -o binarytrees_pmem_cpp binarytrees_pmem.cpp -lpmemobj -pthread


binarytrees_pmem_cleanup_gopmem: binarytrees_pmem_cleanup.go
	~/workspace/go-pmem/bin/go build -txn -o binarytrees_pmem_cleanup_gopmem binarytrees_pmem_cleanup.go


binarytrees_pmem_reuse_gopmem: binarytrees_pmem_reuse.go
	~/workspace/go-pmem/bin/go build -txn -o binarytrees_pmem_reuse_gopmem binarytrees_pmem_reuse.go


fannkuchredux_um_cpp: fannkuchredux_um.cpp
	g++ -o fannkuchredux_um_cpp fannkuchredux_um.cpp


fannkuchredux_um_go: fannkuchredux_um.go
	go build -o fannkuchredux_um_go fannkuchredux_um.go


fannkuchredux_um_gopmem: fannkuchredux_um.go
	~/workspace/go-pmem/bin/go build -txn -o fannkuchredux_um_gopmem fannkuchredux_um.go


fannkuchredux_pmem_cpp: fannkuchredux_pmem.cpp pvector.hpp
	g++ -o fannkuchredux_pmem_cpp fannkuchredux_pmem.cpp pvector.hpp -lpmemobj


fannkuchredux_pmem_gopmem: fannkuchredux_pmem.go
	~/workspace/go-pmem/bin/go build -txn -o fannkuchredux_pmem_gopmem fannkuchredux_pmem.go


nbody_um_cpp: nbody_um.cpp
	g++ -o nbody_um_cpp nbody_um.cpp

nbody_um_go: nbody_um.go
	go build -o nbody_um_go nbody_um.go


nbody_um_gopmem: nbody_um.go
	~/workspace/go-pmem/bin/go build -txn -o nbody_um_gopmem nbody_um.go


nbody_pmem_cpp: nbody_pmem.cpp
	g++ -o nbody_pmem_cpp nbody_pmem.cpp -lpmemobj


nbody_pmem_gopmem: nbody_pmem.go
	~/workspace/go-pmem/bin/go build -txn -o nbody_pmem_gopmem nbody_pmem.go

clean:
	rm binarytrees_um_cpp binarytrees_um_go binarytrees_um_gopmem binarytrees_pmem_cpp binarytrees_pmem_cleanup_gopmem binarytrees_pmem_reuse_gopmem fannkuchredux_um_cpp fannkuchredux_um_go fannkuchredux_um_gopmem fannkuchredux_pmem_cpp fannkuchredux_pmem_gopmem nbody_um_cpp nbody_um_go nbody_um_gopmem nbody_pmem_cpp nbody_pmem_gopmem