import os
import subprocess
import numpy as np
import matplotlib.pyplot as plt
import time

executable = "./build/countwords_par"
config_path = "./configuration/config_generated"
os.makedirs(os.path.dirname(config_path), exist_ok=True)

queue_sizes = [1, 4, 8, 10, 20, 30, 40, 50, 60, 70, 80]
target_queue = "raw_files_queue_size"

fixed_value = 1000
indexing_threads = 4
merging_threads = 2

config_template = """
indir="/mnt/iso/7/7"
out_by_a="./data/res_a.txt"
out_by_n="./data/res_n.txt"
indexing_extensions = .txt .md
archives_extensions = .zip
max_file_size = 10000000
indexing_threads = {indexing_threads}
merging_threads = {merging_threads}
filenames_queue_size = {filenames}
raw_files_queue_size = {raw}
dictionaries_queue_size = {dicts}
measure_indexing_time = true
"""

def run_with_config(queue_size):
    if target_queue == "raw_files_queue_size":
        filenames = fixed_value
        raw = queue_size
        dicts = fixed_value
    elif target_queue == "filenames_queue_size":
        filenames = queue_size
        raw = fixed_value
        dicts = fixed_value
    elif target_queue == "dictionaries_queue_size":
        filenames = fixed_value
        raw = fixed_value
        dicts = queue_size
    else:
        raise ValueError("Unknown queue")

    config_text = config_template.format(
        indexing_threads=indexing_threads,
        merging_threads=merging_threads,
        filenames=filenames,
        raw=raw,
        dicts=dicts
    )

    with open(config_path, "w") as f:
        f.write(config_text)

    result = subprocess.run([executable, config_path], stdout=subprocess.PIPE)
    output = result.stdout.decode("utf-8").splitlines()

    indexing_time = None
    for line in output:
        if line.startswith("Reading="):
            continue
        if "Indexing=" in line or "indexing=" in line:
            indexing_time = float(line.split("=")[1])
        if "Total=" in line:
            total = float(line.split("=")[1])
    return indexing_time if indexing_time else total

results = []
for size in queue_sizes:
    print(f"[RUNNING] {target_queue} = {size}")
    times = [run_with_config(size) for _ in range(2)]
    avg_time = np.mean(times)
    print(f"[DONE] Avg Indexing Time: {avg_time:.2f} s")
    results.append((size, avg_time))

x_vals, y_vals = zip(*results)

plt.figure(figsize=(10, 6))
plt.plot(x_vals, y_vals, marker="o")
plt.xscale("log")
plt.xlabel(f"{target_queue} (log scale)")
plt.ylabel("Average Indexing Time (s)")
plt.title(f"Impact of {target_queue} on Indexing Time")
plt.grid(True)
plt.tight_layout()
plt.show()
