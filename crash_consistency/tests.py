import subprocess
import os
import sys
import resource
import time
import math

header = "Prog Name,PMEM file exist"
time_hdr = ",User{},Sys{}"
prog_info = "{},{}"
trial_vals = ",{},{}"

avg_reps = 5
crash_reps = 10
pmem_file_name = "pmemtest"

cpp_avg = 0
go_avg = 0

um_progs = ["./cc_ram_cpp", "./cc_ram_go"]
pmem_progs = ["./cc_pmem_cpp", "./cc_pmem_go"]

f = open("cc_results_file.txt", "w+")


def get_averages(um, pmem):
    avg_list = []
    for i in range(0, avg_reps):
        start = time.time()
        subprocess.call([um])
        end = time.time()
        avg_list.append(end-start)
    u = sum(avg_list) / len(avg_list)          
    avg_list = []  
    for i in range(0, avg_reps):
        start = time.time()
        subprocess.call([pmem])
        avg_list.append(time.time() - start)
        subprocess.call(["rm", pmem_file_name])
    p = sum(avg_list) / len(avg_list)       
    return (u,p)   

def kill_proc_test(prog, killint, kills):
    total = 0
    times_k = 0
    finished = False
    print("launching {}".format(prog))
    start = time.time()
    proc = subprocess.Popen([prog], stdout=subprocess.PIPE)
    while not finished:
        while times_k < kills:
            try:
                proc.wait(timeout=int(math.floor(killint)))
                end = time.time()
                total = total + (end - start)
                finished = True
                break
            except:
                proc.kill()
                k = time.time()
                total = total + (k - start)
                times_k = times_k + 1
                print("killed {}, relaunching".format(prog))
                start = time.time()
                proc = subprocess.Popen([prog], stdout=subprocess.PIPE)
        if not finished:
            print("waiting on prog completion")
            proc.wait()
            end = time.time()
            total = total + end - start
            finished = True

    return (total, times_k)



def run_crash_test(um, pmem, u, p):
    print("starting crash test")
    
    for kills in range (1, 31):
        ku = u / 2
        kp = p / (kills + 1)
        (ram_time, rk) = kill_proc_test(um, ku, kills)
        subprocess.call(["rm", pmem_file_name])
        (pmem_time, pk) = kill_proc_test(pmem, kp, kills)
        subprocess.call(["rm", pmem_file_name])
        f.write("{} {} Results:\n".format(pmem, kills))
        f.write("{} Realtime: {}, killed {} times\n".format(um, ram_time, rk))
        f.write("{} Realtime: {}, killed {} times\n".format(pmem, pmem_time, pk))

subprocess.call(["make"])

(ug,pg) = get_averages("./cc_ram_go", "./cc_pmem_go")
f.write("./cc_ram_go Average Time: {}\n./cc_pmem_go Average Time: {}\n".format(ug,pg))
(uc,pc) = get_averages("./cc_ram_cpp", "./cc_pmem_cpp")
f.write("./cc_ram_cpp Average Time: {}\n./cc_pmem_cpp Average Time: {}\n".format(uc,pc))
run_crash_test("./cc_ram_go", "./cc_pmem_go", ug, pg)
run_crash_test("./cc_ram_cpp", "./cc_pmem_cpp", uc, pc)

