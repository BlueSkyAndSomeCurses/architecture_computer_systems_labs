#! /usr/bin/env python3

import sys
import subprocess
import glob
import numpy as np
import csv

EXECUTABLE_NAME = glob.glob("*/integrate_serial")[-1]
EPSILON = 1e-3


def main():
    call_times = sys.argv[1]

    if call_times.isalnum() == 0:
        raise Exception(
            "Number of calls of each method and buffer clearing flag must be natural number"
        )

    call_times = int(call_times)

    results = []

    for _ in range(call_times):

        result = list(
            map(
                float,
                (
                    subprocess.run(
                        [
                            EXECUTABLE_NAME,
                            str(4),
                            f"./data/func{4}.cfg",
                        ],
                        stdout=subprocess.PIPE,
                    )
                    .stdout.decode("utf-8")
                    .split()
                ),
            )
        )

        results.append(result)

    flag = True

    for i in range(len(results) - 1):
        if abs(results[i][0] - results[i + 1][0]) > EPSILON:
            flag = False
            break

    # with open('results.csv', mode='w', newline='') as file:
    #     writer = csv.writer(file)
    #     writer.writerow(['Function', 'Min Time', 'Avg Time', 'Std Deviation'])

    if flag:
        print(results[-1][0])
        print(min(results, key=lambda ress: ress[1])[1])
        print(min(results, key=lambda ress: ress[2])[2])
        print(min(results, key=lambda ress: ress[3])[3])

        calc_times_arr = [ress[3] for ress in results]
        print(sum(calc_times_arr) / len(results))
        print(np.std(calc_times_arr, ddof=1))

        print()

            # writer.writerow([4, min(results, key=lambda ress: ress[3])[3], sum(calc_times_arr) / len(results), np.std(calc_times_arr, ddof=1)])

    else:
        print("Absolute error is too big")


if __name__ == "__main__":
    main()