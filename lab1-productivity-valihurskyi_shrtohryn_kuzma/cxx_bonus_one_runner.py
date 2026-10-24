#! /usr/bin/env python3

import subprocess
import numpy as np
import scipy.stats
import os
import math
import argparse

EXECUTABLE_NAME = os.path.join(os.path.dirname(__file__), "bin", "cxx_bonus_one")
MAX_NEW_PROCESSES = 2

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("test_data", type=str, help="Path to the test data")
    parser.add_argument("call_times", type=int, help="Number of calls of each method")
    parser.add_argument("split_output_file", type=str, help="Output file for split methods")
    parser.add_argument("convert_output_file", type=str, help="Output file for convert methods")
    parser.add_argument("--split_results", type=str, help="Raw time results for split methods")
    parser.add_argument("--convert_results", type=str, help="Raw time results for convert methods")

    args = parser.parse_args()
    if (os.path.dirname(args.split_output_file) != ""):
        os.makedirs(os.path.dirname(args.split_output_file), exist_ok=True)
    if (os.path.dirname(args.convert_output_file) != ""):
        os.makedirs(os.path.dirname(args.convert_output_file), exist_ok=True)
    if args.split_results is None:
        args.split_results = os.path.join(os.path.dirname(args.split_output_file), "split_results.csv")
    if args.convert_results is None:
        args.convert_results = os.path.join(os.path.dirname(args.convert_output_file), "convert_results.csv")


    if args.call_times < 1:
        raise ValueError("Number of calls of each method must be a natural number")
    
    if not os.path.exists(args.test_data):
        raise FileNotFoundError("Test data file not found")
    

    # --------------Test split methods----------------
    split_results = np.ndarray((args.call_times, 3), dtype=int)
    for method_id in range (3):
        times = []
        results = []
        # Split the processes into groups of MAX_NEW_PROCESSES and run consequently
        for idx in range(math.ceil(args.call_times/MAX_NEW_PROCESSES)):
            
            processes = []
            for _ in range(min(MAX_NEW_PROCESSES, args.call_times - idx*MAX_NEW_PROCESSES)):
                proc = subprocess.Popen([EXECUTABLE_NAME, args.test_data, str(method_id + 1)],
                                        stdout=subprocess.PIPE)
                processes.append(proc)

            for proc in processes:
                proc.wait()
                time, res = map(int, proc.stdout.read().decode("utf-8").split())
                times.append(time)
                results.append(res)
        
        split_results[:, method_id] = times
        print(np.min(times))
        print(np.mean(times))
        print(np.std(times, ddof=1))
        print()

    if len(set(results)) != 1:
        raise ValueError("Different methods have read different number of symbols")

    with open(args.split_results, "w", encoding="utf-8") as file:
        for line in split_results:
            file.write(",".join(map(str, line)) + "\n")

    wilcoxon_test_matrix = [["", 1, 2, 3]] + [
            [i+1] + [0 for _ in range(3)] for i in range(3)
        ]
    
    for i in range(1, 4):
        for j in range(1, 4):
            if i == j:
                wilcoxon_test_matrix[i][j] = 1.0
                continue
            wilcoxon_test_matrix[i][j] = scipy.stats.wilcoxon(
                split_results[:, i-1], split_results[:, j-1]
            )[1]

    with open(args.split_output_file, "w", encoding="utf-8") as file:
        for line in wilcoxon_test_matrix:
            file.write(",".join(map(str, line)) + "\n")

    # --------------Test conversion methods----------------
    convert_results = np.ndarray((args.call_times, 3), dtype=int)
    for method_id in range(3, 6):
        times = []
        results = []
        processes = []
        for idx in range(math.ceil(args.call_times/MAX_NEW_PROCESSES)):

            processes = []
            for _ in range(min(MAX_NEW_PROCESSES, args.call_times - idx*MAX_NEW_PROCESSES)):
                proc = subprocess.Popen([EXECUTABLE_NAME, args.test_data, str(method_id + 1)],
                                        stdout=subprocess.PIPE)
                processes.append(proc)
            
            for proc in processes:
                proc.wait()
                time, res = map(int, proc.stdout.read().decode("utf-8").split())
                times.append(time)
                results.append(res)

        convert_results[:, method_id - 3] = times
        print(np.min(times))
        print(np.mean(times))
        print(np.std(times, ddof=1))
        if method_id != 5: print()

    if len(set(results)) != 1:
        raise ValueError("Different methods have different sum of symbols")
        
    with open(args.convert_results, "w", encoding="utf-8") as file:
        for line in convert_results:
            file.write(",".join(map(str, line)) + "\n")

    wilcoxon_test_matrix = [["", 1, 2, 3]] + [
            [i+1] + [0 for _ in range(3)] for i in range(3)
        ]
    
    for i in range(1, 4):
        for j in range(1, 4):
            if i == j: 
                wilcoxon_test_matrix[i][j] = 1.0
                continue
            wilcoxon_test_matrix[i][j] = scipy.stats.wilcoxon(
                convert_results[:, i-1], convert_results[:, j-1]
            )[1]
    
    with open(args.convert_output_file, "w", encoding="utf-8") as file:
        for line in wilcoxon_test_matrix:
            file.write(",".join(map(str, line)) + "\n")

if __name__ == "__main__":
    main()
