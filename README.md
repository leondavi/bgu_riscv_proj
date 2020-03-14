# bgu_riscv_proj
riscv architecture for gem5

## Requirements 
1. Clone this repository (Prefered path "<user_home_dir>/workspace")..
2. gem5 required dependencies: 
```sudo apt install build-essential git m4 scons zlib1g zlib1g-dev libprotobuf-dev protobuf-compiler libprotoc-dev libgoogle-perftools-dev libboost-all-dev python-dev python libmlpack-dev```
3. Install scons: ```sudo apt-get install scons```
4. Goto gem5 directory inside the repository and run ```scons build/RISCV/gem5.opt -j4```
5. Build [riscv-gnu-toolchain](https://github.com/riscv/riscv-gnu-toolchain).
   Choose Installation (Newlib), in order to support the embedded riscv gem5 machine. 
   Configure the toolchain with ```--with-arch=rv64g --disable-multilib``` will remove support of compressed instructions.
6. Cross compile using riscv gcc or g++ using the following commands: ```/opt/riscv/bin/riscv64-unknown-elf-gcc-8.2.0```
   or ```/opt/riscv/bin/riscv64-unknown-elf-g++```
7. Install six module of python using pip: ```sudo pip install six```
   or upgrade to the latest version: ```sudo pip install six --upgrade```
8. Optionally - update g5gui submodule: ```git submodule foreach git pull origin master```
   Read more about g5gui in its [repository](https://github.com/leondavi/g5gui). 
   
## IDE Eclipse 
1. Install eclipse throguh Ubuntu Software or apt-get. 
2. Open eclipse and go to help->install new software
3. Choose in work with tab one of eclipse distribution and search for c++.
4. Install Eclipse CDT (C/C++ Development Tooling), install any tool relates to C/C++
5. After C/C++ installation add to work with the following source: 
   Name: SConsolidator Location: http://www.sconsolidator.com/update
   Install Sconsolidator packages. 
6. Create new SCons project, go to File->New->New SCons project from existing code
7. Browse to gem5 folder in Existing Code Location. 
8. Add to SCons Option build/RISCV/gem5.opt -j4
### Eclipse Debug Configurations
1. Go to debug configurations
2. Under Main tab, within C/C++ application bar browse to ```build/RISCV/gem5.opt```.
3. You might choose to add more arguments e.g ```--debug-flag=BGUTrace $GEM5/config/bgu/some_script.py```
### Eclipse - Fix std libs unresolved warnning
1. Go to Project -> Properties -> C/C++ General -> Preprocessor Includes -> Providers and select both: 
<br /> "CDT GCC Built-in Compiler Settings"
<br /> "CDT CROSS GCC Built-in Compiler Settings"
<br /> For each one of those also select the sub-entry: "Use global provider shared between projects".
2. Find the correct include location of C++ by using find for example: ```sudo find / -name vector -print```
3. ```Project->Properties->C/C++ General->Paths and Symbols->GNU C++->(Add)->"/usr/include/c++/<N>" ```

## IDE - QT Creator (optional)
1. sudo apt-get update
2. sudo apt install qtcreator 
3. Open qtcreator->Import Existing Project. 
  <br> Name it (i.e gem5) and give the path to gem5 directory ("<user_home_dir>/workspace/bgu_riscv_proj/gem5"). 
4. Add *.py to "Show files matching" list. (The ability watching py files from QT). 
5. Exclude build folder. Hit finish. 
6. On left side pane goto Projects->Build Settings. 
7. Build directory should be: ```/home/<USER>/workspace/bgu_riscv_proj/gem5```
8. Under build steps and clean steps sections remove make all. (put the cursor over it and choose delete). 
9. Under build steps choose click on "add build step" and select "custom process step". 
10. Fill the following in the command bar: ```scons```
11. Fill the following in the Arguments bar: ```build/RISCV/gem5.opt -j3```
12. In Clean steps section click on Add Clean Step and fill: Command: ```scons``` Arguments: ```-c```
13. Go to Run Settings (also located in Projects left side pane) 
14. choose /gem5/build/RISCV/gem5.opt as executable 
15. TODO complete those instructions (command line arguments should be added) 

## IDE - Visual Studio Code

1. Add gem5 dir as workspace directory. 
2. Open command search window ```CTRL+SHIFT+P```
3. Select the Tasks:Conifgure Task command and click on Create tasks.json from templates. 
4. Select others to create a task that runs an external command. 
5. Paste the following json instead: 
```
{
    
    "version": "2.0.0",
    "tasks": [
        {
            "label": "build hello world",
            "type": "shell",
            "command": "scons build/RISCV/gem5.opt -j4",
            "options": {
                "cwd": "${workspaceFolder}"
            },
            "args": [],
            "group": {
                "kind": "build",
                "isDefault": true
            }
        }
    ]

}
```

## Bash Recommended Aliases 
It's recommended to make a workspace folder as in those alias configuration. 
Add the following to ~/.bashrc:
```
GEM5=$(echo "$HOME/workspace/bgu_riscv_proj/gem5" | tr -d '\r')
alias cdws='cd ~/workspace'
alias cdgem='cd ~/workspace/bgu_riscv_proj/gem5'
alias g5gui='cd $GEM5/g5gui/src ; python3 main.py'
```
