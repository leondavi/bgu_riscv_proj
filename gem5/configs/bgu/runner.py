#!/usr/bin/python

import os
import Tkinter
from Tkinter import *
import tkFileDialog # open dir/file with gui
import ast # load file as variable
import subprocess as sub

#Const
GUI_VERSION = "1.0"
BUILD_CMD = "scons"
GEM5_OPT = "build/RISCV/gem5.opt"
DEBUG_FLAG = "--debug-flags="
PARAM_STORE = os.path.dirname(__file__)+"/params.txt"
#Parameters
param_dict = dict()

# TODO
# files to create
# 1. gem5exe:
#    - buildGem5(g5dir,build_flag)
#    - runGem5(g5opt,g5opt_flags,script,scipt_flags,mode,frame)    
#    - runParallelGem5(g5opt,g5opt_flags,rgr,coreNum,workdir,frame)    
# 2. common
#    - globalVars/Const
#    - genPermutation(rgr)
#    - parseResult(rgr)
#    - genCSV()
#    - runFromCSV()
#    - cleanWorkdir(workdir)


# exeBuild
#==============================================================================
def exeBuild(top_dict):
    # update all parameters
    getParamDict(top_dict) 

    # check if gem5_dir exists and valid path
    if param_dict["gem5_dir"] == '':
        print "ERROR: enter gem5_dir path"
        return
    if not os.path.isdir(param_dict["gem5_dir"]):
        print "ERROR: invalid gem5_dir path"
        return
    build_cmd = [BUILD_CMD,GEM5_OPT]     
    if not param_dict["gem5_build_flag"] == '':
        build_cmd.append(param_dict["gem5_build_flag"])
    # Run command
    p = sub.Popen(build_cmd,cwd=param_dict["gem5_dir"],
        stdout=sub.PIPE,stderr=sub.PIPE)
    output, errors = p.communicate()
    top_dict["text"].delete('1.0', END)
    top_dict["text"].insert('end',output)
    top_dict["text"].insert('end'," ".join(["done execute:"]+build_cmd))
    top_dict["text"].see(END)
    print errors

# exeRun
#==============================================================================
def exeRun(top_dict):
    # update all parameters
    getParamDict(top_dict) 

    # check if gem5_dir exists and valid path
    if param_dict["gem5_dir"] == '':
        print "ERROR: enter gem5_dir path"
        return
    if not os.path.isdir(param_dict["gem5_dir"]):
        print "ERROR: invalid gem5_dir path"
        return
    build_cmd = [GEM5_OPT]
    if not param_dict["gem5_opt_flag"] == '':
        build_cmd.append(DEBUG_FLAG+param_dict["gem5_opt_flag"])
    if param_dict["config_file"] == '':
        print "ERROR: enter config_file"
        return
    if not os.path.isfile(param_dict["config_file"]):
        print "ERROR: invalid config_file path"
        return
    build_cmd.append(param_dict["config_file"]) 
    if not param_dict["config_flags"] == '':
        build_cmd +=param_dict["config_flags"].split(',')
   
    # Run command
    p = sub.Popen(build_cmd,cwd=param_dict["gem5_dir"],
        stdout=sub.PIPE,stderr=sub.PIPE)
    output, errors = p.communicate()
    top_dict["text"].delete('1.0', END)
    top_dict["text"].insert('end',output)
    top_dict["text"].insert('end'," ".join(["done execute:"]+build_cmd))
    top_dict["text"].see(END)
    print errors

# exeRegression
#==============================================================================
def exeRegression(top_dict):
    getParamDict(top_dict) # update all parameters
    print "exeRegression pressed"

# exeRerun
#==============================================================================
def exeRerun(top_dict):
    getParamDict(top_dict) # update all parameters
    print "exeRerun pressed"

# exeParse
#==============================================================================
def exeParse(top_dict):
    getParamDict(top_dict) # update all parameters
    print "exeParse pressed"

# exeClean
#==============================================================================
def exeClean(top_dict):
    getParamDict(top_dict) # update all parameters
    print "exeClean pressed"

# browseFunc
#==============================================================================
def browseFunc(entry_set,name):
    if name == "gem5_dir":
        name = tkFileDialog.askdirectory() # TODO-change to support dir
    else:
        name = tkFileDialog.askopenfilename() # TODO-change to support dir
    entry_set.delete(0, 'end')
    entry_set.insert(0,name)

# getParamDict
#==============================================================================
# Update all GUI parameters
def getParamDict(top_dict):
    key_list = top_dict["main"][1].keys()
    for key in key_list:
        if "_button" in key:
            continue
        param_dict[key] = top_dict["main"][1][key][1]["entry"].get()
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
                  ["regression_file",["label","entry","button"] ],
                  ["workdir"        ,["label","entry","button"] ],
                  ["post_parsing"   ,["label","entry"]          ]]
    top_dict = create_frame(top,top_dict,"main",main_frame)
   
    button_list = ["build","run", "regression","rerun","parse","clean"]
    for b in button_list:
        p_button = Button(top_dict["main"][0],text = b,
            command=lambda x = eval("exe"+b.capitalize()):x(top_dict))
        p_button.pack(side=LEFT)
        top_dict["main"][1][b+"_button"] = p_button
    setParamDict(top_dict)

    p_text = Text(top)
    p_text.pack(side=LEFT,expand = True)
    p_scrollbar = Scrollbar(top)
    p_scrollbar.pack(side=RIGHT, fill=Y)
    p_text.config(yscrollcommand=p_scrollbar.set)
    p_scrollbar.config(command=p_text.yview)
    top_dict["text"] = p_text

    top.mainloop()
   
   

#==============================================================
# Must be used to set main
if __name__ == '__main__':
    main()

