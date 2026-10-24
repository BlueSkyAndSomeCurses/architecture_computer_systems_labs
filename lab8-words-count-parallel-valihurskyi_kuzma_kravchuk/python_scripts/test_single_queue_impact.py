import os
import subprocess
import numpy as np
import matplotlib.pyplot as plt
import time

executable = "./build/countwords_par"
config_path = "./configuration/config_generated"
os.makedirs(os.path.dirname(config_path), exist_ok=True)

test_sizes = [10, 100, 1000, 10_000, 100_000, 1_000_000, 10_000_000]
fixed_size = 1000

target_queue = "dictionaries_queue_size"

config_template = """
indir="/mnt/iso/7/7"
out_by_a="./data/res_a.txt"
out_by_n="./data/res_n.txt"
indexing_extensions = .txt .md
archives_extensions = .zip
max_file_size = 10000000
indexing_threads = 4
merging_threads = 2
filenames_queue_size = {filenames}
raw_files_queue_size = {raw}
dictionaries_queue_size = {dicts}
"""

def run_with_config(filenames, raw, dicts):
    config_text = config_template.format(filenames=filenames, raw=raw, dicts=dicts)
    with open(config_path, 'w') as f:
        f.write(config_text)

    result = subprocess.run([executable, config_path], stdout=subprocess.PIPE)
    output = result.stdout.decode("utf-8").split()

    for entry in output:
        if "Total=" in entry:
            return float(entry.split("=")[1])
    return None

results = []

for size in test_sizes:
    if target_queue == "filenames_queue_size":
        params = (size, fixed_size, fixed_size)
    elif target_queue == "raw_files_queue_size":
        params = (fixed_size, size, fixed_size)
    elif target_queue == "dictionaries_queue_size":
        params = (fixed_size, fixed_size, size)
    else:
        raise ValueError("Unknown queue name")

    print(f"[RUNNING] {target_queue} = {size}")
    times = [run_with_config(*params) for _ in range(2)]
    avg_time = np.mean(times)
    print(f"[DONE] Time: {avg_time:.2f} s")
    results.append((size, avg_time))

x_vals, y_vals = zip(*results)

plt.figure(figsize=(10, 6))
plt.plot(x_vals, y_vals, marker='o')
plt.xscale("log")
plt.xlabel(f"{target_queue} (log scale)")
plt.ylabel("Average Total Time (s)")
plt.title(f"Impact of {target_queue} on Execution Time")
plt.grid(True)
plt.tight_layout()
plt.show()
