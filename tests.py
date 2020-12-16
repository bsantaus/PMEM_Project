from subprocess import call
import os
import sys
import resource

header = "Prog Name,PMEM file exist"
time_hdr = ",User{},Sys{}"
prog_info = "{},{}"
trial_vals = ",{},{}"

repetitions_con = 20
repetitions_asc = 10
pmem_file_name = "pmemtest"

rpt_clas = { "binarytrees": "8", "fannkuchredux": "10", "nbody": "10000"}
asc_clas = { "binarytrees": ["6", "7", "8", "9"], "fannkuchredux": ["6", "7", "8", "9", "10"], "nbody": ["10", "100", "1000", "10000", "25000"]}

um_progs = ["./binarytrees_um_go", "./binarytrees_um_gopmem", "./binarytrees_um_cpp", "./fannkuchredux_um_go", "./fannkuchredux_um_gopmem", "./fannkuchredux_um_cpp", "./nbody_um_go", "./nbody_um_gopmem", "./nbody_um_cpp"]
pmem_progs = ["./binarytrees_pmem_reuse_gopmem", "./binarytrees_pmem_cleanup_gopmem", "./binarytrees_pmem_cpp", "./fannkuchredux_pmem_gopmem", "./fannkuchredux_pmem_cpp", "./nbody_pmem_gopmem", "./nbody_pmem_cpp"]

def run_pgm(exe_name, param):
    usage_st = resource.getrusage(resource.RUSAGE_CHILDREN)
    call([exe_name, param])
    usage_en = resource.getrusage(resource.RUSAGE_CHILDREN)
    user_time = usage_en.ru_utime - usage_st.ru_utime
    sys_time = usage_en.ru_stime - usage_st.ru_stime
    return (user_time, sys_time)

def get_rept_param(pgm):
    if "binarytrees" in pgm:
        return rpt_clas["binarytrees"]
    elif "fannkuchredux" in pgm:
        return rpt_clas["fannkuchredux"]
    elif "nbody" in pgm:
        return rpt_clas["nbody"]
    return None

def run_repetitive_tests(outputfile):
    for pgm in um_progs:
        outputfile.write(prog_info.format(pgm, "n/a"))
        for i in range(0, repetitions_con):
            (usr, sys) = run_pgm(pgm, get_rept_param(pgm))
            outputfile.write(trial_vals.format(usr,sys))
        outputfile.write("\n")

    for pgm in pmem_progs:
        outputfile.write(prog_info.format(pgm, "no"))
        for i in range(0, repetitions_con):
            (usr, sys) = run_pgm(pgm, get_rept_param(pgm))
            outputfile.write(trial_vals.format(usr,sys))
            call(["rm", pmem_file_name])
        outputfile.write("\n")
        outputfile.write(prog_info.format(pgm, "yes"))
        for i in range(0, repetitions_con):
            (usr, sys) = run_pgm(pgm, get_rept_param(pgm))
            outputfile.write(trial_vals.format(usr,sys))
        call(["rm", pmem_file_name])
        outputfile.write("\n")

def get_asc_param(pgm):
    if "binarytrees" in pgm:
        return asc_clas["binarytrees"]
    elif "fannkuchredux" in pgm:
        return asc_clas["fannkuchredux"]
    elif "nbody" in pgm:
        return asc_clas["nbody"]
    return None

def run_ascending_tests(outputfile):
    for pgm in um_progs:
        outputfile.write(prog_info.format(pgm, "n/a"))
        for i in range(0, repetitions_con):
            total_usr = 0
            total_sys = 0
            for param in get_asc_param(pgm):
                (usr, sys) = run_pgm(pgm, param)
                total_usr = usr + total_usr
                total_sys = sys + total_sys
            outputfile.write(trial_vals.format(total_usr,total_sys))
        outputfile.write("\n")

    for pgm in pmem_progs:
        outputfile.write(prog_info.format(pgm, "no"))
        for i in range(0, repetitions_asc):
            total_usr = 0
            total_sys = 0
            for param in get_asc_param(pgm):
                (usr, sys) = run_pgm(pgm, param)
                total_usr = usr + total_usr
                total_sys = sys + total_sys
            outputfile.write(trial_vals.format(total_usr,total_sys))
            if (repetitions_asc - 1 > i):
                call(["rm", pmem_file_name])
        outputfile.write("\n")
        outputfile.write(prog_info.format(pgm, "yes"))
        for i in range(0, repetitions_asc):
            total_usr = 0
            total_sys = 0
            for param in get_asc_param(pgm):
                (usr, sys) = run_pgm(pgm, param)
                total_usr = usr + total_usr
                total_sys = sys + total_sys
            outputfile.write(trial_vals.format(total_usr,total_sys))
        call(["rm", pmem_file_name])
        outputfile.write("\n")



# repeat_file = open("test_output/consistent.csv", "w+")
# ascend_file = open("test_output/ascend.csv", "w+")

repeat_file.write(header)
ascend_file.write(header)

for i in range(0, repetitions_con):
    repeat_file.write(time_hdr.format(i,i))

for i in range(0, repetitions_asc):
    ascend_file.write(time_hdr.format(i,i))

repeat_file.write("\n")
ascend_file.write("\n")

call(["make"])

run_repetitive_tests(repeat_file)
run_ascending_tests(ascend_file)

repeat_file.close()
ascend_file.close()

print('runs')



