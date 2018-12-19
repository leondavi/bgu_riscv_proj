# bgu_riscv_proj
riscv architecture for gem5

## Requirements 
1. Clone this repository (Prefered path "<user_home_dir>/workspace").
2. gem5 required dependencies: 
```sudo apt install build-essential git m4 scons zlib1g zlib1g-dev libprotobuf-dev protobuf-compiler libprotoc-dev libgoogle-perftools-dev python-dev python```
3. Install scons: ```sudo apt-get install scons```
4. Goto gem5 directory inside the repository and run ```scons build/RISCV/gem5.opt -j4```
5. Build [riscv-gnu-toolchain](https://github.com/riscv/riscv-gnu-toolchain).
   Choose Installation (Newlib), in order to support the embedded riscv gem5 machine. 

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



