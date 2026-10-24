#! /usr/bin/env python3

import sys
import subprocess
import glob
import numpy
import scipy.stats

EXECUTABLE_NAME = glob.glob("*/cxx_perf_io")[-1]


def main():
    cl_args = sys.argv[1:]
    test_data, call_times, cache_flag, output_file = cl_args

    if call_times.isalnum() == 0 or cache_flag.isalnum() == 0:
        raise Exception(
            "Number of calls of each method and buffer clearing flag must be natural number"
        )

    call_times, cache_flag = int(call_times), int(cache_flag)

    results = {
        k: [[0] * call_times, float("+inf"), [0] * call_times] for k in range(1, 6)
    }

    for attempt in range(call_times):
        for method_type in range(1, 6):
            if cache_flag:
                subprocess.run(["sudo", "sync"])
                subprocess.run(["sudo", "purge"])

            result = list(
                map(
                    float,
                    (
                        subprocess.run(
                            [EXECUTABLE_NAME, test_data, str(method_type)],
                            stdout=subprocess.PIPE,
                        )
                        .stdout.decode("utf-8")
                        .split()
                    ),
                )
            )

            results[method_type][0][attempt] = result[1]
            results[method_type][1] = min(results[method_type][1], result[0])
            results[method_type][2][attempt] = result[0]

    if (
        results[1][0]
        == results[2][0]
        == results[3][0]
        == results[4][0]
        == results[5][0]
    ):
        for method, result in results.items():
            print(result[1])
            mean = sum(results[method][2]) / call_times
            print(mean)
            if call_times != 1:
                print(
                    numpy.sqrt(
                        sum(map(lambda x: (x - mean) ** 2, result[2]))
                        / (call_times - 1)
                    )
                )
            else:
                print(0)
            print()

        wilcoxon_test_matrix = [["", 1, 2, 3, 4, 5]] + [
            [i] + [0 for _ in range(5)] for i in range(1, 6)
        ]

        for i in range(1, 6):
            for j in range(1, 6):
                wilcoxon_test_matrix[i][j] = scipy.stats.wilcoxon(
                    results[i][2], results[j][2]
                )[1]

        with open(output_file, "w", encoding="utf-8") as file:
            for line in wilcoxon_test_matrix:
                file.write(",".join(map(str, line)) + "\n")
    else:
        print(results)
        raise Exception("Different methods have read different number of symbols")

    return results


def write_data_to_csv(data: dict, filename: str) -> None:
    with open(filename, "w", encoding="utf-8") as file:
        file.write("1,2,3,4,5,\n")
        for i in range(len(data[1][2])):
            for method in range(1, 6):
                file.write(f"{data[method][2][i]},")

            file.write("\n")


if __name__ == "__main__":
    data = main()
    write_data_to_csv(data, "results_50m_with_clearing.csv")
