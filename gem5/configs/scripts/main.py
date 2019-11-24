#!/usr/bin/python
import ast
import os
import itertools

# TODO - need to be parameteric
RGR_FILE="/home/yossi/workspace/g5gui/run.pgp"
RGR_RESULT="/home/yossi/workspace/bgu_riscv_proj/gem5/statistics"
GEM5_DIR = "/home/yossi/workspace/bgu_riscv_proj/gem5"
STS_LIST = ["sim_insts","sim_ticks"]

# wild card
# script name 
CONFIG_FILE_ATTR = "config"
# binnary directory
BINARY_DIR_ATTR = "binary_dir"
# debug flag 
DEBUG_ATTR = "--debug-flag"

# parse_reg_file
#==============================================================================
def parse_reg_file():
    p_rgr_file =  open(RGR_FILE)
    rgr_file_list = ast.literal_eval(p_rgr_file.read())
    
    for rgr_file_data in rgr_file_list:
        script_name,rgr_name,rgr = parse_reg_data(rgr_file_data)
        rgr_name,rgr = find_stats(script_name,rgr_name,rgr,STS_LIST)
        print_result(rgr_name,rgr)

# parse_reg_data
#==============================================================================
def parse_reg_data(rgr_file_data):
    script_name = ""
    binary_files = []
    debug_flag = ""
    # extract script name
    script_name = parse_reg_data_wild(rgr_file_data,CONFIG_FILE_ATTR,1)
    binary_dir = parse_reg_data_wild(rgr_file_data,BINARY_DIR_ATTR,0)
    if binary_dir != None:
        p_path = "{0}/{1}".format(GEM5_DIR,binary_dir)
        rgr_file_data["--binary"] = [binary_dir+"/" + s for s in os.listdir(p_path)]
    debug_flag = parse_reg_data_wild(rgr_file_data,DEBUG_ATTR,0)

    for key,val in rgr_file_data.items():
        print "key: {0} data: {1}".format(key,val)
    rgr_name,rgr = createPermutions(rgr_file_data)
    return script_name,rgr_name,rgr
# parse_reg_data_wild
#==============================================================================
def parse_reg_data_wild(rgr_file_data,field,error=0):
    # extract script name
    if field in  rgr_file_data:
        name = rgr_file_data[field]
        del(rgr_file_data[field])
    elif error:
        print "Error couldnt find  field in pgp (wild card:{0})".format(field)
    return name
    
#createPermutions
#==============================================================================
def createPermutions(rgr_params_list):
    rgr = list(list())
    rgr_name = list()
    for key,val in rgr_params_list.items():
        rgr_name.append(key)
        tmp_rgr = []
        # check if list
        if type(val) != list:
            val = [val]
    
        # in case list is empty
        if len(rgr) == 0:
            tmp_rgr = [val]
        else:
            for old_list in rgr:
                for new_val in val:
                    tmp_rgr.append(old_list+[new_val])
        rgr = tmp_rgr

    return rgr_name,rgr


# find_stats
#==============================================================================
def find_stats(script_name,rgr_name,rgr,sts_list):
    all_dirs = os.listdir(RGR_RESULT)
    new_rgr_name = rgr_name +["STS"]+sts_list
    new_rgr = []
    for line in rgr:
        # look on all dirs
        all_dir_copy= all_dirs
        # check all fields exists
        for i in range(len(rgr_name)):
            if rgr_name[i]  == "--binary":
                pttrn = line[i].split("/")[-1]
            else:
                pttrn = "{0}_{1}".format(rgr_name[i][2:],line[i])
            all_dir_found = []   

            # search for the pattern
            for dir_name in all_dir_copy:
                if (dir_name.find(pttrn)!=-1):
                    all_dir_found.append(dir_name)
            # Store the dirs where the pattern is found
            all_dir_copy = all_dir_found 
        
        if len(all_dir_copy)==1:
            p_path = RGR_RESULT+ "/"+all_dir_copy[0]
            sts = get_stats(p_path,sts_list)
        else:
            sts = ["NOT_RUN"]
        new_rgr.append(line+sts)
    return new_rgr_name,new_rgr

# get_stats
#==============================================================================
def get_stats(p_dir,sts):
    p_file = p_dir+"/"+"stats.txt"
    sts_dic = {}
    if not os.path.exists(p_file):
        return ["FAILED"]
    
    p_file_read = open(p_file)
    for line in p_file_read:
        fields = line.split()
        if(len(fields)==0):
            continue
        if(fields[0] in sts):
            sts_dic[fields[0]] = fields[1]

    sts_list =["DONE"]
    for s in sts:
        if s in sts_dic:
            sts_list.append(sts_dic[s])
        else:
            sts_list.append("x")
            sts_list[0] = "Error"
    return sts_list        

# print_result
#==============================================================================
def print_result(rgr_name,rgr):
    result_file = RGR_RESULT+"/result.csv"
    print result_file
    p_result_file = open(result_file,"w")
    p_result_file.write("{0}\n".format(",".join(rgr_name)).replace("-",""))
    for line in rgr:
        p_result_file.write("{0}\n".format(",".join(line)))
    p_result_file.close()


# Main
#==============================================================================
def main():
    parse_reg_file()

#
#==============================================================================
if __name__=="__main__":
    main()
