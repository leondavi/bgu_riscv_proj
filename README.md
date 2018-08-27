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
7. Under build steps and clean steps sections remove make all. (put the cursor over it and choose delete). 
8. Under build steps choose click on "add build step" and select "custom process step". 
9. Fill the following: 
