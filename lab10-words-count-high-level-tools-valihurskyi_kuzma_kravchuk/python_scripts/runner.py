#! /usr/bin/env python3

import sys
import subprocess
import glob
import numpy as np

EXECUTABLE_NAME = glob.glob("./build/countwords_par_tbb", recursive=True)[-1]
EPSILON = 0.1

# Taken from lab1 : https://github.com/ucu-cs/lab1-productivity-kravchuk_tarnavskiy_slabliuk
def clear_disk_cache():
    try:
        subprocess.run(["sudo", "sh", "-c", "echo 3 > /proc/sys/vm/drop_caches"], check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error clearing disk cache: {e}")
        sys.exit(1)

def main(call_times: int, flush: bool):
    outputs = []
    total_times = []
    finding_times = []
    reading_times = []
    writing_times = []


    for i in range(call_times):
        if flush:
            clear_disk_cache()

        result_raw = subprocess.run(
            [EXECUTABLE_NAME, "./configuration/config_linux"],
            stdout=subprocess.PIPE,
        ).stdout.decode("utf-8").split()

        timing_dict = {k: float(v) for k, v in (entry.split("=") for entry in result_raw)}
        total_times.append(timing_dict.get("Total", 0.0))
        finding_times.append(timing_dict.get("Finding", 0.0))
        reading_times.append(timing_dict.get("Reading", 0.0))
        writing_times.append(timing_dict.get("Writing", 0.0))


        current_result = {}
        with open("./data/res_a.txt", 'r', encoding='utf-8') as f:
            for line in f:
                if ":" not in line:
                    continue
                splitted = line.strip().split(":")
                if len(splitted) > 2:
                    word = ":".join(splitted[:-1])
                    count = splitted[-1]
                else:
                    word, count = splitted
                current_result[word.strip()] = int(count.strip())

        outputs.append(current_result)

    first_out = outputs[0]
    for i, other_out in enumerate(outputs[1:], start=2):
        if set(other_out.items()) != set(first_out.items()):
            print(f"Результати запуску #{i} не збігаються з першим запуском")
            raise Exception("Results are not equal")

    def stats(arr):
        return min(arr), sum(arr) / len(arr), np.std(arr, ddof=1)

    return stats(total_times), stats(finding_times),stats(reading_times), stats(writing_times)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        raise Exception("Usage: script.py <call_times> <flush (0|1)>")

    call_times = sys.argv[1]
    flush = sys.argv[2]

    if not call_times.isdigit() or int(call_times) <= 0:
        raise Exception("Number of calls must be a natural number")

    if flush not in ["0", "1"]:
        raise Exception("Flush must be 0 or 1")

    call_times = int(call_times)
    flush = bool(int(flush))

    total_stats, finding_stats, reading_stats, write_stats = main(call_times, flush)

    for value in total_stats:
        print(value)

    print()

    for value in finding_stats:
        print(value)

    print()

    for value in reading_stats:
        print(value)

    print()

    for value in write_stats:
        print(value)
