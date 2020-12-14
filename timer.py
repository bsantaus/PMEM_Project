from subprocess import call
import os
import sys
import resource

def run_pgm(benchmark, cfg, exe_name):
    usage_st = resource.getrusage(resource.RUSAGE_CHILDREN)
    call(["{}_bench/{}".format(benchmark, exe_name), "9"])
    usage_en = resource.getrusage(resource.RUSAGE_CHILDREN)
    user_time = usage_en.ru_utime - usage_st.ru_utime
    sys_time = usage_en.ru_stime - usage_st.ru_stime
    print("{} user time: {}\n".format(exe_name, user_time))
    print("{} sys time: {}\n".format(exe_name, sys_time))

def bench_go(benchmark, configs):
    for cfg in configs:
        if os.path.exists("{}/{}_{}.go".format(benchmark, benchmark, cfg)):
            if cfg == "um":
                call(["go", "build", "-o", "{}_bench/{}_{}_um".format(benchmark, benchmark, cfg), "{}/{}_{}.go".format(benchmark, benchmark, cfg)])
                run_pgm(benchmark, cfg, "{}_{}_um".format(benchmark, cfg))
                call(["../go-pmem/bin/go", "build", "-o", "{}_bench/{}_{}_mod".format(benchmark, benchmark, cfg), "{}/{}_{}.go".format(benchmark, benchmark, cfg)])
                run_pgm(benchmark, cfg, "{}_{}_mod".format(benchmark, cfg))
            else:
                call(["../go-pmem/bin/go", "build", "-txn", "-o", "{}_bench/{}_{}".format(benchmark, benchmark, cfg), "{}/{}_{}.go".format(benchmark, benchmark, cfg)])
                run_pgm(benchmark, cfg, "{}_{}".format(benchmark, cfg))


def bench_cpp(benchmark, configs):
    for cfg in configs:
        call(["g++", "-pthread", "-I/usr/local/include", "-L/usr/local/lib", "-lpmemobj", "-std=c++11", "-o", "{}_bench/{}_{}".format(benchmark, benchmark, cfg), "{}/{}_{}.cpp".format(benchmark, benchmark, cfg)])
        run_pgm(benchmark, cfg, "{}_{}".format(benchmark, cfg))

def main(argv, argc):
    benchmarks = []
    langs = []
    cfgs = []
    curr_list = ""
    if argc < 7:
        print("Too few arguments\nCorrect Syntax: python3 timer.py -b [bench1, <bench2, ...>] -l [lang1, <lang2>] -c [cfg1, <cfg2, ...>]")
        exit()
    for arg in argv:
        if (arg == argv[0]):
            continue

        if arg == "-b" or arg == "-l" or arg == "-c":
            curr_list = arg
        else:
            if curr_list == "-b":
                benchmarks.append(arg)
            elif curr_list == "-l":
                langs.append(arg)
            elif curr_list == "-c":
                cfgs.append(arg)
            else:
                print("Incorrect syntax (no flag specified)")
                print("Correct Syntax: python3 timer.py -b [bench1, <bench2, ...>] -l [lang1, <lang2>] -c [cfg1, <cfg2, ...>]")

    for bmk in benchmarks:
        if "cpp" in langs:
            bench_cpp(bmk, cfgs)
        if "go" in langs:
            bench_go(bmk, cfgs)


if __name__ == "__main__":
    main(sys.argv, len(sys.argv))