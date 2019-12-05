import os
import csv
from collections import OrderedDict


def fetchParam(my_str, start_str,end_str):
    start   = my_str.find(start_str)    + len(start_str)
    end     = my_str.find(end_str)       
    return my_str[start: end]


def getRunStats(statDir):
    paramList = []
    for folder in os.listdir(statDir):
        if not os.path.isdir(os.path.join(statDir,folder)):
            continue
        keyVal = folder.split(":")
        dict = {}
        for elm in keyVal:
            kv = elm.split("-")
            dict[kv[0]] = kv[1]
        stats_file = os.path.join(statDir,folder,"stats.txt")
        if not os.path.isfile(stats_file):
            continue
        size,xx = convert_bytes(os.stat(stats_file).st_size)
        dict["done"] = size
        paramList.append(dict)
    return paramList


def updateCSVs(statDir, csv_in, csv_out):
    workloads = []
    paramList = getRunStats(statDir)
    finalHeaders = ["App_workload", "cores", "L1_size", "associativity", "l1_Config", "L0_Width", "banks", "bankBit", "clusters", "Done"]
    dictHeaders = ["binary", "num_proc", "size", "assoc", "l1Config", "L0lineWidth", "banks", "bankBits", "clusters", "done"]
    return readWrite_CSV(csv_in, csv_out, paramList, finalHeaders, dictHeaders)


def readWrite_CSV(infile, outfile, paramList, headers, dHeaders):
    count_jobs = 0
    count_empties = 0
    with open(infile) as inf:
        reader = csv.DictReader(inf)
        cpus = outfile.split("/")[2][0]
        with open(outfile, 'w') as ouf:
            writer = csv.DictWriter(ouf, fieldnames = headers)
            writer.writeheader()
            for row in reader:
                for ii in range(len(paramList)):
                    if( paramList[ii][dHeaders[0]] == row["App"] and
                        paramList[ii][dHeaders[1]] == row["num_cores"] and
                        paramList[ii][dHeaders[2]] == row["L1_size"] and
                        paramList[ii][dHeaders[3]] == row["associativity"]  and
                        paramList[ii][dHeaders[4]] == row["L1_conf"]       and
                        paramList[ii][dHeaders[5]] == row["L0_width"]    and
                        paramList[ii][dHeaders[6]] == row["Banks"]         and
                        paramList[ii][dHeaders[7]] == row["Bank_bits"]  and
                        paramList[ii][dHeaders[8]] == row["clusters"]
                        ):
                        writer.writerow({"App_workload": row["App"], "cores": row["num_cores"],\
                                        "L1_size": row["L1_size"], "associativity": row["associativity"],\
                                        "l1_Config": row["L1_conf"], "L0_Width": row["L0_width"],\
                                        "banks": row["Banks"], "bankBit": row["Bank_bits"],\
                                        "clusters": row["clusters"], "Done": paramList[ii]["done"]})
                        count_jobs += 1
                        if paramList[ii]["done"] == 0:
                            count_empties +=1
    return count_jobs, count_empties




def convert_bytes(num):
    """
    this function will convert bytes to MB.... GB... etc
    """
    for x in ['bytes', 'KB', 'MB', 'GB', 'TB']:
        if num < 1024.0:
            return (num, x)
        num /= 1024.0

def cvs_update(stat_path = "/home/shlomo5/workspace/bgu_riscv_proj/gem5/statistics/"):
    jj = {}
    ee = {}
    for file in os.listdir(CSV_DIR) :
        if file[-4:] == ".csv":
            outfile = file[:-4] + "_tmp.csv"
            jj[file[0]], ee[file[0]] = updateCSVs(stat_path,os.path.join(CSV_DIR,file),os.path.join(CSV_OUTDIR,outfile))
    return jj, ee

STAT_FILE = "stats.txt"
JOBFILE = "job_tracker.txt"
CSV_DIR = "csvs"
CSV_OUTDIR = "csvs/out"

if __name__ == '__main__':
    jj,ee = cvs_update()
    print("Jobs passed: {0}\n Jobs empty/failed: {1}".format(jj, ee))

