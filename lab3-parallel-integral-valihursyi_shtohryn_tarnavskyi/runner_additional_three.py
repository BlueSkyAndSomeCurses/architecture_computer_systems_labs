#! /usr/bin/env python3

from os import write
import sys
import subprocess
import glob
import numpy as np

EXECUTABLE_NAME1 = glob.glob("*/integrate_parallel_mutex_bad")[-1]
EXECUTABLE_NAME2 = glob.glob("*/integrate_parallel_atomic_bad")[-1]
EXE_LIST = [EXECUTABLE_NAME1, EXECUTABLE_NAME2]
EPSILON = 1e-5


def main(call_times: int, threads_num: int, executable_name: str):
    results = {k: [] for k in range(1, 4)}

    for _ in range(call_times):
        for func_type in range(1, 4):
            result = list(
                map(
                    float,
                    (
                        subprocess.run(
                            [
                                executable_name,
                                str(func_type),
                                f"./data/func{func_type}.cfg",
                                str(threads_num),
                            ],
                            stdout=subprocess.PIPE,
                        )
                        .stdout.decode("utf-8")
                        .split()
                    ),
                )
            )

            results[func_type].append(result)

    flag = True

    for func_num in range(1, 4):
        for res_left in results[func_num]:
            for res_right in results[func_num]:
                if abs(res_left[0] - res_right[0]) > EPSILON:
                    flag = False
                    break
            if not flag:
                break
        if not flag:
            break

    if flag:
        return [
            [
                results[func_num][-1][0],
                min(results[func_num], key=lambda ress: ress[1])[1],
                min(results[func_num], key=lambda ress: ress[2])[2],
                min(results[func_num], key=lambda ress: ress[3])[3],
                sum(calc_times_arr := [ress[3] for ress in results[func_num]])
                / len(results[func_num]),
                np.std(calc_times_arr, ddof=1),
            ]
            for func_num in range(1, 4)
        ]
    else:
        raise Exception("Absolute error is too big")


def write_results_for_each_numb_of_threads(filename: str, executable_name: str):
    csv_to_write = "threads/func,1,2,3\n"
    threads_lst = [1, 2, 4, 8, 16, 20, 100, 1000]

    for thread_num in threads_lst:
        csv_to_write += str(thread_num)

        print(thread_num)
        results = main(3, thread_num, executable_name)

        for func in results:
            csv_to_write += f",{func[4]}"

        csv_to_write += "\n"

    with open(filename, "w", encoding="utf-8") as file:
        file.write(csv_to_write)


if __name__ == "__main__":
    file_list = [
        "./additional_three_comparison/data/measurements_mutex_bad.csv",
        "./additional_three_comparison/data/measurements_atomic_bad.csv",
    ]

    for i in range(2):
        write_results_for_each_numb_of_threads(file_list[i], EXE_LIST[i])
