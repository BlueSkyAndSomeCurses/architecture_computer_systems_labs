#! /usr/bin/env python3

import sys
import subprocess
import glob
import numpy as np


EXECUTABLE_NAME = glob.glob("**/integrate_parallel_queue", recursive=True)[-1]
EPSILON = 1e-5


def main(call_times: int, threads_num: int, partition_num: int):
    results = {k: [] for k in range(1, 4)}

    for _ in range(call_times):
        for func_type in range(1, 4):
            result = list(
                map(
                    float,
                    (
                        subprocess.run(
                            [
                                EXECUTABLE_NAME,
                                str(func_type),
                                f"./data/func{func_type}.cfg",
                                str(threads_num),
                                str(partition_num),
                            ],
                            stdout=subprocess.PIPE,
                        )
                        .stdout.decode("utf-8")
                        .split()
                    ),
                )
            )
            if len(result) == 0:
                results[func_type].append([0, 0, 0, 0])
            else:
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


def write_results_for_each_numb_of_threads(filename: str, order_split):
    csv_to_write = "threads/func,1,2,3\n"
    for thread_num in range(1, 115, 3):

        csv_to_write += str(thread_num)
        print(thread_num)
        results = main(3, thread_num, thread_num * order_split)

        for func in results:
            csv_to_write += f",{func[4]}"

        csv_to_write += "\n"

    with open(
        filename + f"_{order_split}_partitions.csv", "w", encoding="utf-8"
    ) as file:
        file.write(csv_to_write)


if __name__ == "__main__":
    for order_split in [8]:
        write_results_for_each_numb_of_threads(
            "./bonus_one_results/data/bonus_3_measurements",
            order_split,
        )
