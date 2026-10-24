# Lab work 4: Parallel integral with thread safe queue
Authors (team): [Valihurskyi Anton](https://github.com/BlueSkyAndSomeCurses) [Oleh Shtohryn](https://github.com/lalkaman1981) [Vasyl Tarnavskyi](https://github.com/TarnMaVas)

## Prerequisites

gcc clang cmake make ninja python3 boost gsl

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

Binary of integral runner wil be named `integrate_parallel_queue`

```sh
$ ./build/integrate_parallel_serial <func_number> <path/to/config/file> <number_of_threads> <number_of_jobs_per_thread>
```

If you want to run python runner script, create config files for each function called `funcX.cfg` where `X` is function's number under `data` folder.

```sh
$ ./runner_integral.py <number_of_calls> <number_of_threads> <number_of_jobs_per_thread>
```

# Additional tasks
All additional tasks:

- Determined number of threads for which paralellization produces speedup
- alternative approach: each thread calculates separate square - the respective executable is alternative_approach; is described in the report and initially implemented, but has some bugs
- main thread calculates different splits at the same done
- Estimamted portion of parallelized code with Amdal's law and linear regression.
