# Lab work 6: 1brc

Authors (team): [Valihurskyi Anton](https://github.com/BlueSkyAndSomeCurses)
Variant:

## Prerequisites

```shell
make ninja gcc clang cmake boost python3
```

### Compilation

<mark>HOW TO COMPILE YOUR PROGRAM? (RECOMMENDED: ./compile.sh -- use `./compile.sh -h` for info)</mark>

### Installation

<mark>DESCRIBE THE INSTALLATION PROCESS (USE ./dependencies FOLDER)</mark>

<mark>Note: For Python scripts, You must add `requirements.txt`
file and add your env to the `.gitignore` file!</mark>

### Usage

<mark>PROVIDE AN EXAMPLE OF HOW TO RUN YOUR PROGRAM (IT CAN BE A_flag COMMAND LINE WITH INPUT AND EXPECTED
OUTPUT)</mark>

<mark>Note: if your project needs or generates any data, media and so on -- put them
into the data folder</mark>

### Important!

<mark>WHAT ELSE SHOULD WE KNOW ABOUT YOUR WORK? (E.G. KNOWN ISSUES, BUGS, SPECIAL BEHAVIOR ETC)</mark>

### Results

After obvious rewrite of provided in lab python 1brc code to c++ the execution was actually slower. Here is short timing
table for 10 million lines executions' times (in ms) table.

| try | python time | c++ time |
|-----|-------------|----------|
| 1   | 2688        | 3256     |
| 2   | 2643        | 3239     |
| 3   | 2695        | 3253     |
| 4   | 2675        | 3231     |
| 5   | 2742        | 3266     |

We see, that c++ version is around 1/4 slower. Here is results of profiling with `KDAB/hotspot`

![hotspot_27_03](./profiler_runs/run_27_03.jpg)
We already see, that boost's methods take up a lot of our cycles!
We have replaced `boost::split` method to split each line by `;` to `std::substr` together with `std::find` methods. We
also went from `double` datatype to store data to `float` as we require precision up to 1 sign after dot. Consequently,
we decided to switch from `boost::lexical_cast` to `atof`. In the very end, it drastically improved performance (almost
4 times!).

| try | python time | c++ time |
|-----|-------------|----------|
| 1   | 2642        | 908      |
| 2   | 2601        | 905      |
| 3   | 2635        | 907      |
| 4   | 2614        | 905      |
| 5   | 2635        | 910      |

We are also considering to switch from `getline` to `get` function, as get should be searching through buffer which is
expected to be faster than RAM (`getline` reads into string and that we do operations on this string, that lives on
RAM).

![hotspot_02_04](./profiler_runs/run_02_04.png)
![hotspot_details_02_04](./profiler_runs/additional_info_run_02_04.png)

Here we see, that writing and retrieving data in hashtable is very time consuming.
Thus we will try to implement heterogeneous lookup. Also, floating point conversation and rounding are heavy,
we will transition it to `stoi`.
We will store part before comma and after. And in the very end do only one floating point conversation.

Paralelization

To best paralelize we need to split a task into the smaller parts. These parts can be assigned to each thread to be
computed independently,
as we only need the info we get from the processing in the end.
We estimate adequate number of lines to evaluate per thread together with maximum number of tasks in a queue,
so that we do not store more than 100MB data at once.

We use separate thread safe queues, to pass bunch of lines to processs to a thread.
And a queue for resulting hash maps that will be merged further by merging thread.

Running profiler on parallel code is indeed problematic. Here is the flamegraph:
![parallel_hotspot](./profiler_runs/hotspot_02_04_run_parallel.png)
![topdown_load](./profiler_runs/top-down-load_parallel.png)

We see, that profiler was not able to trace "hot spots".

# Additional tasks

<mark>IF APPLICABLE, LIST ALL THE EXTRA FEATURES YOU ADDED. PROVIDE DETAILS<mark>



