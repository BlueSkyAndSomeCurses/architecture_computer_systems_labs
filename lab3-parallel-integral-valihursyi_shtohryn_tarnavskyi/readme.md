# Lab work 3: Parallelizing the calculation of the integral of a given function
Authors (team): [Valihurskyi Anton](https://github.com/BlueSkyAndSomeCurses) [Oleh Shtohryn](https://github.com/lalkaman1981) [Vasyl Tarnavskyi]()

## Prerequisites

gcc clang cmake make ninja python3 boost

### Compilation

Firstly, install all the dependencies and clone `tomlplusplus` repository.

```sh
$ mkdir build && cd build
$ cmake ..
$ make all || ninja
```

### Installation


```shell
$ git clone https://github.com/marzer/tomlplusplus.git --depth=1
$ python -m venv .env
$ source .env/bin/activate
$ pip install -r requirements.txt
```
### Usage

Binary of integral runner wil be named `integrate_parallel`

```sh
$ ./build/integrate_parallel <func_number> <path/to/config/file> <thread_number>
```

If you want to run python runner script, create config files for each function called `funcX.cfg` where `X` is function's number under `data` folder.

```sh
$ ./runner_integral_parallel.py <number_of_calls> <thread_number>
```

# Additional tasks
Were made additional tasks 1, 2 and 3.

Additional 1:

Details and analysis in the report.

Additional 2:
```sh
$ ./runner_additional_two.py <number_of_calls> <thread_number>
```
It will perform the same action as ./runner_integral_parallel.py and then write data in
additional_two_comparison/data. To perform comparison in ./additional_two_comparison there is
.ipynb file.

Additional 3:
```sh
$ ./runner_additional_three.py
```
It will search for two files: integrate_parallel_mutex_bad and integrate_parallel_atomic_bad.
Then there will be data in ./additional_three_comparison/data. To perform comparison in ./additional_three_comparison there is
.ipynb file.
# ATTENTION!
  
Additional tasks not listed in the previous paragraph would not be graded.

Be sure to provide a complete list of authors.

