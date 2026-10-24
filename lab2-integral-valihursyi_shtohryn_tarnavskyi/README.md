# Lab work <mark>NUMBER</mark>: <mark>SHORT TOPIC</mark>

Authors (
team): [Valihurskyi Anton](https://github.com/BlueSkyAndSomeCurses) [Oleh Shtohryn](https://github.com/lalkaman1981) [Vasyl Tarnavskyi]()

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

Binary of integral runner wil be named `integrate_serial`

```sh
$ ./build/integrate_serial <func_number> <path/to/config/file>
```

If you want to run python runner script, create config files for each function called `funcX.cfg` where `X` is
function's number under `data` folder.

```sh
$ ./runner_integral.py <number_of_calls>
```

For checking the results of additional tasks 4 and 5, use the same command with ./runner_additional_functional.py and
./runner_additional_precision.py respectively.

# Additional tasks

1,2,4,5

