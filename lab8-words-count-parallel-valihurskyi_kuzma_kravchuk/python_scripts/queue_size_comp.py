import os
import subprocess
import numpy as np
import matplotlib.pyplot as plt

config_template = """
indir="/mnt/iso/7/7"
out_by_a="./data/res_a.txt"
out_by_n="./data/res_n.txt"
indexing_extensions = .txt .md
archives_extensions = .zip
max_file_size = 10000000
indexing_threads = 2
merging_threads = 2
filenames_queue_size = {size}
raw_files_queue_size = {size}
dictionaries_queue_size = {size}
"""

executable = "./build/countwords_par"
config_path = "./configuration/config_generated"


# queue_sizes = [1, 10, 100, 1000, 10000, 100000, 1000000, 10000000]
queue_sizes = [10**i for i in range(1,10)]
timings = []

def run_with_config(size):
    config_text = config_template.format(size=size)
    with open(config_path, 'w') as f:
        f.write(config_text)

    result_raw = subprocess.run(
        [executable, config_path],
        stdout=subprocess.PIPE
    ).stdout.decode("utf-8").split()

    timing_dict = {k: float(v) for k, v in (entry.split("=") for entry in result_raw)}
    print(f"Queue size {size}, time: {timing_dict.get('Total', 0.0)}")
    return timing_dict.get("Total", 0.0)

for size in queue_sizes:
    times = []
    for _ in range(2):
        total_time = run_with_config(size)
        times.append(total_time)
    avg_time = np.mean(times)
    timings.append(avg_time)

plt.figure(figsize=(10, 6))
plt.plot(queue_sizes, timings, marker='o')
plt.xscale("log")
plt.xlabel("Queue Size (log scale)")
plt.ylabel("Average Total Time (s)")
plt.title("Average Execution Time vs Queue Size")
plt.grid(True)
plt.tight_layout()
plt.show()
