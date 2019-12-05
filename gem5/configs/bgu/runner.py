#!/usr/bin/python

import os,time,re
import Tkinter
from Tkinter import *
import tkFileDialog # open dir/file with gui
import ast # load file as variable
import subprocess as sub
from multiprocessing import Pool, Manager,Value
import threading
import itertools
#Const
DEBUG_MODE = 0
GUI_VERSION = "1.0"
PARAM_STORE = os.path.dirname(os.path.abspath(__file__))+"/params.txt"
BUILD_CMD = "scons"
GEM5_OPT = "build/RISCV/gem5.opt"
DEBUG_FLAG = "--debug-flags="
RGR_WILDCARD = "BINARY_DIR"
RGR_DIR_FLAG = " --outdir="   
RGR_TRACKER = "job_tracker.txt"
STATS_FILE = "stats.txt"
#Parameters
param_dict = dict()
runStatus=dict()

###############################################################################
###############################################################################
###############################################################################
# Common function
#==============================================================================

# checkPointer
#==============================================================================
def checkPointer(pointer,name,isdir = False,verbose=True):
    if pointer =='':
        if verbose:
            print "Error: enter {0} pointer".format(name)
        return 0

    if (    isdir and (not os.path.isdir(pointer))) or\
       (not isdir and (not os.path.isfile(pointer))):
       if verbose:
            print "Error: invalid {0} pointer({1})".format(name,pointer)
       return 0
    else:
        return 1

# genPermutation
#==============================================================================
# each list inside list is in length of 2, first is "key", second "Value",
# the value can be a single or a list.
def genPermutation(list_of_lists):
    perm_list_of_lists = list()
    key_list = list()
    # check the rgr is in correct format
    if type(list_of_lists) !=list:
        print "Error: RGR format not supported"
        return None
    
    for lists in list_of_lists:
        # check that each list is in correct format
        if(type(lists)!=list ) or (len(lists) != 2):
            print "Error: not supported format {0}".format(lists)
            return None
        key,values = lists
        # convers to list the values
        if type(values) != list:
            values = [values]
        
        if key == RGR_WILDCARD:
            key = "binary"
            p = values[0]
            values =[p+"/"+s for s in os.listdir(values[0])]

        key_list.append(key)
        # base is empty        
        if(not len(perm_list_of_lists)):
            for val in values:
                perm_list_of_lists.append(["--{0}={1}".format(key,val)])
        else:
            tmp_list = list()
            for perm in perm_list_of_lists:
                for val in values:
                    val = "--{0}={1}".format(key,val)
                    tmp_list.append(perm+[val])
            perm_list_of_lists = tmp_list
    return key_list,perm_list_of_lists

# genDirName
#==============================================================================
def genRgrDirName(param_list):
    lcl = list()
    for i in range(len(param_list)):
        if "binary" in param_list[i]:
            flag,name = param_list[i].split("=")
            name = name.split("/")[-1].split(".")[0]
            lcl.append("{0}={1}".format(flag,name))
        else:
            lcl.append(param_list[i])
    return ("".join(lcl)).replace("--",":").replace("=","-")[1:]

# genRgrCmd
#==============================================================================
def genRgrCmd(cmd,wd,config_file,rgr_file,rgr_wd):
    # read file and create all rgr permutation
    p = open(rgr_file)
    _,permutation = genPermutation(ast.literal_eval(p.read()))
    p.close()
    if(permutation == None):
        return None

    cmd_list=list()
    # 2. gen local cmd, modify output dir and append all parameters
    for l in permutation:  
        outdir_flag = RGR_DIR_FLAG+rgr_wd+"/"+genRgrDirName(l)
        new_cmd = cmd + [outdir_flag,config_file]+l
        outdir= outdir_flag.split("=")[1]
        cmd_list.append({"cmd":new_cmd,"wd":wd,"rgr":"1","outdir":outdir})
    return cmd_list

# runSingleCmd
#==============================================================================
def runSingleCmd(param):
    cmd = param["cmd"]
    wd = param["wd"]
    skip = 0
    output=""
    errors=""

    if "rgr" in param:
        if checkPointer(param["outdir"],"",isdir=True,verbose=False): 
            skip = 1
            runStatus["skip"].acquire(block=True)
            runStatus["skip"].value +=1
            runStatus["skip"].release()
            print "Skip execute: {0} \n".format(" ".join(cmd))

    if skip == 0:    
        print "Start execute: {0} \n".format(" ".join(cmd))
        # TODO - depemdes on the version
        p = sub.Popen(" ".join(cmd),cwd=wd,stdout=sub.PIPE,stderr=sub.PIPE,shell=True)
        output, errors = p.communicate()

    if "rgr" in param:
        runStatus["run"].acquire(block=True)
        runStatus["run"].value +=1
        runStatus["run"].release()
        print "Total: {0}, Run: {1}, Skip: {2}".format(
            runStatus["total"],runStatus["run"].value,runStatus["skip"].value)
    
    return output,errors
    
# runCmd
#==============================================================================
def runCmd(cmd,wd,frame,clear=True):
    if clear:
        frame.delete('1.0', END)
    if DEBUG_MODE == 0:
        output,errors = runSingleCmd({"cmd":cmd,"wd":wd})
        frame.insert('end',output)
        print errors # TODO - maybe 2 windows?
    else:
        time.sleep(2)
    frame.insert('end'," ".join(["done execute:"]+cmd)+"\n\n")
    frame.see(END)

# runRgrCmd
#==============================================================================
def runRgrCmd(cmd,wd,config_file,rgr_file,rgr_wd,cpus,frame):
    frame.delete('1.0', END) # clean window

    cmd_list = genRgrCmd(cmd = cmd,wd = wd,config_file = config_file,
        rgr_file=rgr_file,rgr_wd=rgr_wd)
    if cmd_list == None:
        return

    runStatus["total"] = len(cmd_list)
    runStatus["run"] = Value('i', 0)
    runStatus["skip"] = Value('i',0)

    pool = Pool(processes=int(cpus), maxtasksperchild=1)
    results = pool.map(runSingleCmd,cmd_list)
    runPostProcessing(rgr_file,rgr_wd,[])

# runPostProcessing()
#==============================================================================
def runPostProcessing(rgr_file,rgr_wd,parse_list):
    p = open(rgr_file)
    key_list,permutation = genPermutation(ast.literal_eval(p.read()))
    p.close()
    if(permutation == None):
        return None

    result_list=list()
    # 2. gen local cmd, modify output dir and append all parameters
    for perm in permutation:  
        outdir = rgr_wd+"/"+genRgrDirName(perm)
        result = parseResult(outdir,parse_list)
        new_perm = list()
        for p in perm:
            new_perm.append(p.split("=")[1])
        result_list.append(new_perm+result)

    key_list = key_list + ["status"]+parse_list
    printCSV(rgr_wd,header=key_list,data=result_list)

    return 

# parseResult
#==============================================================================
def parseResult(outdir,parse_list):
    # chec that directory exists
    if not checkPointer(outdir,"",isdir=True,verbose=False):
        return ["UNKNOWN"]
        
    p_file = outdir+"/"+STATS_FILE
    if os.stat(p_file).st_size ==0:
        return ["FAIL"]

    parse_result = [None]*len(parse_list)
    p_read = open(p_file)
    
    for line in p_read:
        # merege all space to 1 and split by one
        data=(" ".join(line.split())).split (" ") 
        if(len(data)==1):
            continue
        if data[0] in parse_list:
            parse_result[parse_list.index(data[0])]=data[1]
    p_read.close
       
    result = ["PASS"] + parse_result
    return result 

# printCSV
#==============================================================================
def printCSV(rgr_wd,header,data):
    result_file = rgr_wd+"/result.csv"
    p_result_file = open(result_file,"w")
    p_result_file.write("{0}\n".format(",".join(header)).replace("-",""))
    for line in data:                    
        print line
        p_result_file.write("{0}\n".format(",".join(line)))
    p_result_file.close()

    


###############################################################################
###############################################################################
###############################################################################
# buttun activation
#==============================================================================

# exeBuild
#==============================================================================
def exeBuild(top_dict):
    build_cmd = [BUILD_CMD,GEM5_OPT]     

    # update all parameters
    getParamDict(top_dict) 
    
    # check if params exists and are ok
    if(not checkPointer(param_dict["gem5_dir"],"gem5_dir",True)):
        return
    if not param_dict["gem5_build_flag"] == '':
        build_cmd.append(param_dict["gem5_build_flag"])

    # Run command
    runCmd(build_cmd,param_dict["gem5_dir"],top_dict["text"])
    print "exeBuild pressed"


# exeRun
#==============================================================================
def exeRun(top_dict):
    build_cmd = [GEM5_OPT]

    # update all parameters
    getParamDict(top_dict)

    # check if params exists and are ok  
    if(not checkPointer(param_dict["gem5_dir"],"gem5_dir",True)):
        return    
    if not param_dict["gem5_opt_flag"] == '':
        build_cmd.append(DEBUG_FLAG+param_dict["gem5_opt_flag"])
    if(not checkPointer(param_dict["config_file"],"config_file",False)):
        return    
    build_cmd.append(param_dict["config_file"]) 
    if not param_dict["config_flags"] == '':
        build_cmd +=param_dict["config_flags"].split(',')
   
    # Run command
    runCmd(build_cmd,param_dict["gem5_dir"],top_dict["text"])
    print "exeRun pressed"

# exeRegression
#==============================================================================
def exeRegression(top_dict):
    build_cmd = [GEM5_OPT]

    # update all parameters
    getParamDict(top_dict)

    # check if params exists and are ok  
    if(not checkPointer(param_dict["gem5_dir"],"gem5_dir",True)):
        return    
    if not param_dict["gem5_opt_flag"] == '':
        build_cmd.append(DEBUG_FLAG+param_dict["gem5_opt_flag"])
    if(not checkPointer(param_dict["rgr_wd"],"rgr_wd",True)):
        return
    if(not checkPointer(param_dict["config_file"],"config_file",False)):
        return    
    if(not checkPointer(param_dict["rgr_file"],"rgr_file",False)):
        return
    if (param_dict["rgr_cpus"] == '') or\
        (not param_dict["rgr_cpus"].isdigit()) :
        print "Error: invalid cpus entry(must be int) ({0})".\
            format(param_dict["rgr_cpus"] )
        return

    # Run command
    runRgrCmd(cmd = build_cmd,wd = param_dict["gem5_dir"],
        config_file=param_dict["config_file"], rgr_file=param_dict["rgr_file"],
        rgr_wd=param_dict["rgr_wd"],cpus=param_dict["rgr_cpus"],
        frame=top_dict["text"])
    print "exeRegression pressed"

# exeRerun
#==============================================================================
def exeRerun(top_dict):
    getParamDict(top_dict) # update all parameters
    if(not checkPointer(param_dict["rgr_wd"],"rgr_wd",True)):
        return
    result_file = param_dict["rgr_wd"]+"/result.csv"
    if(not checkPointer(result_file,"rgr_wd",False)):
        return 
    # TODO - read CSV, key and valus, in key search t he location of status
    # for any status that not PASS generate the full command for regresion
    print "exeRerun pressed"
# exeParse
#==============================================================================
def exeParse(top_dict):
    getParamDict(top_dict) # update all parameters
    if(not checkPointer(param_dict["rgr_wd"],"rgr_wd",True)):
        return
    if(not checkPointer(param_dict["rgr_file"],"rgr_file",False)):
        return
    parse_list = param_dict["post_parsing"].split(",")        
    runPostProcessing(rgr_file=param_dict["rgr_file"],
        rgr_wd=param_dict["rgr_wd"],parse_list=parse_list)
    print "exeParse pressed"

# exeClean
#==============================================================================
def exeClean(top_dict):
    getParamDict(top_dict) # update all parameters
    print "exeClean pressed"

# exeHelp
#==============================================================================
def exeHelp(top_dict):
    frame = top_dict["text"]
    s = "Help Menu:                                                         \n"\
        "1.build                                                            \n"\
        "  Requires: gem5_dir\n  Optional: gem5_build_flag                  \n"\
        "  Exe: \"scons build/RISCV/gem5.opt <flags>\"                      \n"\
        "2.run(single)                                                      \n"\
        "  Requires: gem5_dir,config_file                                   \n"\
        "  Optional: gem5_opt_flag,config_flags                             \n"\
        "  Exe: \"build/RISCV/gem5.opt <opt_flags> config_file <config_flags>\"\n"\
        "3.regression                                                       \n"\
        "  Requires: gem5_dir,config_file,rgr_file,rgr_wd                   \n"\
        "  Optional: gem5_opt_flag                                          \n"\
        "  Exe: Create all rgr permutation and run them on config file.     \n"\
        "       Each test result generated into rgr_wd directory, in case   \n"\
        "       rgr permutation exists, the test will be skipped.           \n"\
        "       The execution generate two file:                            \n"\
        "         1. tracker.txt - all command that run/skip-TBD            \n"\
        "         2. result.csv - report of test each test status           \n"\
        "                         (optional: auto parse run[TBD])           \n"\
        "4.rerun\n"\
        "5.parse\n"\
        "  Requires: rgr_file,rgr_wd                                        \n"\
        "  Optional: post_parsing                                           \n"\
        "  Exe: Based om rgr file check in rgr_wd which test run and what is\n"\
        "       there status:[UNKNOW,FAIL,PASS]                             \n"\
        "       In additionin, extract post_parsing statisitcs from result. \n"\
        "6.clean-TBD                                                        \n"\
        "7.stop -TBD                                                      \n\n"\
        "*sendMail-once execution done, send a mail-TBD                     \n"\
        "*config_flags,post_parsing are lists, can insert multiplale        \n"\
        " arguments using comma(,) as seperatore                            \n"
    frame.delete('1.0', END)
    frame.insert('end', s)
    frame.see(END)

# exeStop
#==============================================================================
def exeStop(top_dict):
    pass

# browseFunc
#==============================================================================
def browseFunc(entry_set,name):
    if name == "gem5_dir" or name =="rgr_wd":
        name = tkFileDialog.askdirectory() 
    else:
        name = tkFileDialog.askopenfilename() 
    entry_set.delete(0, 'end')
    entry_set.insert(0,name)


###############################################################################
###############################################################################
###############################################################################
# GUI functions
#==============================================================================

# getParamDict
#==============================================================================
# Update all GUI parameters
def getParamDict(top_dict):
    key_list = top_dict["main"][1].keys()
    for key in key_list:
        if "_button" in key:
            continue
        param_dict[key] = top_dict["main"][1][key][1]["entry"].get()
    
#    print param_dict["sendMail"]
    
    p = open(PARAM_STORE,"w")
    p.write(str(param_dict))
    p.close()

# setParamDict()
#==============================================================================
# ReadParamDict
def setParamDict(top_dict):
    if not os.path.isfile(PARAM_STORE):
        return
    p = open(PARAM_STORE)
    data = ast.literal_eval(p.read())
    p.close()
    key_list = top_dict["main"][1].keys()
    for key in key_list:
        if "_button" in key:
            continue
        if key in data:
            param_dict[key] = top_dict["main"][1][key][1]["entry"].insert(0,data[key])

# create frame
#==============================================================================
def create_frame(top,top_dict,frame_name,frame_format):
    frame_dict = dict()
    # Frame
    p_label_frame= LabelFrame(top,text=frame_name)
    p_label_frame.pack(side = TOP,fill = BOTH,expand = True)

    for iteam in frame_format:
        key,val_list = iteam
        row_dict = dict()
        p_frame = Frame(p_label_frame)
        p_frame.pack(side = TOP,fill = BOTH, expand = True)
        for val in val_list:
            if val == "label":
                p_label = Label(p_frame,text="{}:".format(key))
                p_label.pack(side = LEFT,fill = BOTH,expand = False)
                row_dict["label"] = p_label
            elif val == "entry":
                p_entry = Entry(p_frame)
                p_entry.pack(side = LEFT,fill = X, expand = True)
                row_dict["entry"] = p_entry
            elif val == "button":
                p_button = Button(p_frame, text="Browse",
                    command = lambda x = row_dict["entry"],y=key :browseFunc(x,y))
                p_button.pack(side= LEFT, expand = False)
                row_dict["button"] = p_button
        frame_dict[key] =[p_frame,row_dict]
    top_dict[frame_name] = [p_label_frame,frame_dict]    
    return top_dict

###############################################################################    
###############################################################################    
###############################################################################    
def main():
    top_dict = dict()
    top = Tkinter.Tk()
   
    top.title("gem5 GUI app ver-"+str(GUI_VERSION))
    top.geometry('730x620')
    top.resizable(TRUE,TRUE)
     
    # Build Frames
    main_frame = [["gem5_dir"       ,["label","entry","button"] ],
                  ["gem5_build_flag",["label","entry"]          ],
                  ["gem5_opt_flag"  ,["label","entry"]          ],
                  ["config_file"    ,["label","entry","button"] ],
                  ["config_flags"   ,["label","entry"]          ],
                  ["rgr_file"       ,["label","entry","button"] ],
                  ["rgr_wd"         ,["label","entry","button"] ],
                  ["rgr_cpus"       ,["label","entry"]          ],
                  ["post_parsing"   ,["label","entry"]          ]]
    top_dict = create_frame(top,top_dict,"main",main_frame)
   
    button_list = ["help","build","run", "regression","rerun","parse","clean","stop"]
    for b in button_list:
        p_button = Button(top_dict["main"][0],text = b,
            command=lambda x = eval("exe"+b.capitalize()):x(top_dict))
        p_button.pack(side=LEFT)
        top_dict["main"][1][b+"_button"] = p_button
    
    sendMail = IntVar()
    p_check = Checkbutton(top_dict["main"][0], text="sendMail",
        var = sendMail,  onvalue = 1, offvalue = 0,  )
    p_check.pack(side=RIGHT)
    top_dict["main"][1]["check_button"] = p_check
#    param_dict["sendMail"]  = sendMail.get() TODO

    setParamDict(top_dict)

    p_text = Text(top)
    p_text.pack(side=LEFT,fill = BOTH,expand = True)
    p_scrollbar = Scrollbar(top)
    p_scrollbar.pack(side=RIGHT, fill=Y)
    p_text.config(yscrollcommand=p_scrollbar.set)
    p_scrollbar.config(command=p_text.yview)
    top_dict["text"] = p_text

    #print top.winfo_children() 
    top.mainloop()

#==============================================================
# Must be used to set main
if __name__ == '__main__':
    main()
