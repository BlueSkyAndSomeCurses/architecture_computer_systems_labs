# Lab work 11: MPI temperature simulatrion
Authors (team): [Valihurskyi Anton](https://github.com/BlueSkyAndSomeCurses), [Shtohryn Oleh](https://github.com/lalkaman1981), [Tarnavskyi Maksym-Vasyl](https://github.com/TarnMaVas)

## Prerequisites

cmake make ninja clang gcc libopenmpi-dev openmpi-bin openmpi-common openmpi-doc libboost-dev libboost-mpi-dev 

### Compilation

```shell
mkdir build && cd build 
cmake .. 
make temperature_simulation
```


### Usage

To run code, firstly create starting distribution grid as show in `data/data1/start_dist.csv` and create config simillar to `data/data1.cfg`
then run following command
```shell
mpirun -np <number of procs '>3'> build/temperature_simulation <pass to config>
```


### Results

[report](https://docs.google.com/document/d/17gwLHBN_o_lrzs6mTskkVSbqLfO0j1uNaCGXco_NkzE/edit?tab=t.0)

