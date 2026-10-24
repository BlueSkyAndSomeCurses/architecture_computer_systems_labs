#! /usr/bin/env python3

import sys
import subprocess
import glob
import numpy as np

EXECUTABLE_NAME = glob.glob("*/additional_precision")[-1]
EPSILON = 1e-3


def main():
    call_times = sys.argv[1]

    if call_times.isalnum() == 0:
        raise Exception(
            "Number of calls of each method and buffer clearing flag must be natural number"
        )

    call_times = int(call_times)

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
        for i in range(len(results[func_num]) - 1):
            if abs(results[func_num][i][0] - results[func_num][i + 1][0]) > EPSILON:
                flag = False
                break

        if not flag:
            break

    if flag:
        for func_num in range(1, 4):
            print(results[func_num][-1][0])
            print(min(results[func_num], key=lambda ress: ress[1])[1])
            print(min(results[func_num], key=lambda ress: ress[2])[2])
            print(min(results[func_num], key=lambda ress: ress[3])[3])

            calc_times_arr = [ress[3] for ress in results[func_num]]
            print(sum(calc_times_arr) / len(results[func_num]))
            print(np.std(calc_times_arr, ddof=1))

            print()
    else:
        print("Absolute error is too big")


if __name__ == "__main__":
    main()
