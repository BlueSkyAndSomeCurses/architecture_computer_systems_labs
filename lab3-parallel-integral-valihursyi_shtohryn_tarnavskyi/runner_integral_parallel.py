#! /usr/bin/env python3

from os import write
import sys
import subprocess
import glob
import numpy as np

EXECUTABLE_NAME = glob.glob("./integrate_parallel")[-1]
EPSILON = 1e-5


def main(call_times: int, threads_num: int):
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


def write_results_for_each_numb_of_threads(filename: str):
    csv_to_write = "threads/func,1,2,3\n"
    for thread_num in (*range(1, 17), 100, 1000):
        csv_to_write += str(thread_num)

        results = main(30, thread_num)

        for func in results:
            csv_to_write += f",{func[4]}"

        csv_to_write += "\n"

    with open(filename, "w", encoding="utf-8") as file:
        file.write(csv_to_write)


if __name__ == "__main__":
    call_times = sys.argv[1]
    threads_num = sys.argv[2]

    if call_times.isalnum() == 0 or threads_num.isalnum() == 0:
        raise Exception(
            "Number of calls of each method and threads number must be natural number"
        )

    call_times, threads_num = int(call_times), int(threads_num)

    results = main(call_times, threads_num)
    for func in results:
        for res in func:
            print(res)
        print()

    # write_results_for_each_numb_of_threads(
    # "./integral_parallel_measurements/final_measurements.csv"
    # )
