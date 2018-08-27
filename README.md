## bgu_riscv_proj
riscv architecture for gem5

#Requirements 
1. Clone this repository.
2. gem5 required dependencies: 
```sudo apt install build-essential git m4 scons zlib1g zlib1g-dev libprotobuf-dev protobuf-compiler libprotoc-dev libgoogle-perftools-dev python-dev python```
3. Install scons: ```sudo apt-get install scons```
4. Goto gem5 directory inside the repository and run ```scons build/RISCV/gem5.opt -j4```
5. Build [riscv-gnu-toolchain](https://github.com/riscv/riscv-gnu-toolchain).

#IDE - QT Creator (optional)
1. sudo apt-get update
2. sudo apt install qtcreator 
3. Open qtcreator->Import Existing Project
