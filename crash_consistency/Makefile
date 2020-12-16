all: cc_pmem_cpp cc_pmem_go cc_ram_cpp cc_ram_go

cc_ram_cpp: crash_cons_ram.cpp
	g++ -o cc_ram_cpp crash_cons_ram.cpp

cc_pmem_cpp: crash_cons_pmem.cpp
	g++ -o cc_pmem_cpp crash_cons_pmem.cpp -lpmemobj

cc_ram_go: crash_cons_ram.go
	~/workspace/go-pmem/bin/go build -o cc_ram_go crash_cons_ram.go

cc_pmem_go: crash_cons_pmem.go
	~/workspace/go-pmem/bin/go build -txn -o cc_pmem_go crash_cons_pmem.go

clean: 
	rm cc_pmem_cpp cc_pmem_go cc_ram_go cc_ram_cpp