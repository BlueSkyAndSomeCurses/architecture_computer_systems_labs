# Lab work <mark>1</mark>: <mark>C++ methods' performance comparison</mark>
Authors (team): 
[Valihurskyi Anton](https://github.com/BlueSkyAndSomeCurses),
[Volodymyr Kuzma](https://github.com/Kuzmarg),
[Oleh Shtohryn](https://github.com/lalkaman1981)
## Prerequisites

cmake, make, gcc, clang, bash/zsh/fish, python

Python packages. Install them with:
```
$ python -m venv .env
$ source .env/bin/activate
$ pip install -r requirements.txt
```

C++ libraries:
- Boost
- Qt6
- Abseil

### Compilation

```
$ mkdir cmake-build
$ cd cmake-build
$ cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=".." ..
$ make
```

### Installation

Install binaries with make:
```
$ make install
```

### Usage

If you want to run separate program, run:
```
cxx_perf_<io/conv> <path/to/file/file_name> <method number [1-5]>
```

If you want to use automatic python runner:
```
python cxx_perf_<io/conv>_runner.py <path/to/file/file_name> <how many times run each method> <clear buffer 0 or 1> <file to save p-values>
```

For python script from additional task 1:
```
python cxx_bonus_one_runner.py <path/to/file/file_name> <how many times run each method> <file to save p-values for split methods> <file to save p-values for conversion methods>
```

### Results

Results on different methods comparison with detailed explanation can be found in the `program1_results` and `program2_results` folders' python notebooks' respectively.

# Additional tasks
- Additional task 1

    Abseil library was used as 3rd alternative for comparison. Split methods and conversion method were tested separately to estimate the best combinations without running all pairs. See folder `bonus1_results`, [notebook](./bonus1_results/bonus1_comparison.ipynb) for explanations.

- Additional task 2

    Analysis is present in this [notebook](./bonus2_distributions.ipynb).

- Additional task 3
    
    See folder `program3_results`, [notebook](./program3_results/program3_comparison.ipynb) for explanations.
