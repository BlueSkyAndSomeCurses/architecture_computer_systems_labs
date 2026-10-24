import os
import subprocess
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from itertools import product
import time

# Конфігураційний шаблон
config_template = """
indir="/mnt/iso/7/7"
out_by_a="./data/res_a.txt"
out_by_n="./data/res_n.txt"
indexing_extensions = .txt .md
archives_extensions = .zip
max_file_size = 10000000
indexing_threads = {indexing_threads}
merging_threads = {merging_threads}
filenames_queue_size = 1000
raw_files_queue_size = 1000
dictionaries_queue_size = 1000
"""

executable = "./build/countwords_par"
config_path = "./configuration/config_generated"
os.makedirs(os.path.dirname(config_path), exist_ok=True)

indexing_threads_list = [1, 2, 4, 6, 8, 12, 16]
merging_threads_list = [1, 2, 4]

results = []
base_time = None

def run_with_config(indexing_threads, merging_threads):
    print(f"[RUNNING] Indexing={indexing_threads}, Merging={merging_threads}...")
    start = time.time()

    config_text = config_template.format(indexing_threads=indexing_threads, merging_threads=merging_threads)
    with open(config_path, 'w') as f:
        f.write(config_text)

    result_raw = subprocess.run(
        [executable, config_path],
        stdout=subprocess.PIPE
    ).stdout.decode("utf-8").split()

    end = time.time()
    duration = end - start
    print(f"[FINISHED] in {duration:.2f} sec")

    # Парсинг результату
    timing_dict = {}
    for entry in result_raw:
        if "=" in entry:
            key, val = entry.split("=")
            timing_dict[key.strip()] = float(val.strip())
    return timing_dict.get("Total", 0.0)

for indexing_threads, merging_threads in product(indexing_threads_list, merging_threads_list):
    times = []
    for _ in range(2):
        total_time = run_with_config(indexing_threads, merging_threads)
        times.append(total_time)
    avg_time = np.mean(times)

    if indexing_threads == 1 and merging_threads == 1:
        base_time = avg_time

    if base_time and avg_time > 0:
        speedup = base_time / avg_time
        efficiency = speedup / indexing_threads
    else:
        speedup = 0.0
        efficiency = 0.0

    results.append({
        "indexing_threads": indexing_threads,
        "merging_threads": merging_threads,
        "avg_time": avg_time,
        "speedup": speedup,
        "efficiency": efficiency
    })

    print(f"Indexing={indexing_threads}, Merging={merging_threads} -> Time={avg_time:.2f}s, Speedup={speedup:.2f}, Efficiency={efficiency:.4f}")

df = pd.DataFrame(results)
df.to_csv("parallelization_results.csv", index=False)

plt.figure(figsize=(10, 6))
for merging_threads in sorted(df["merging_threads"].unique()):
    subset = df[df["merging_threads"] == merging_threads]
    plt.plot(
        subset["indexing_threads"],
        subset["efficiency"],
        marker='o',
        label=f"Merging Threads = {merging_threads}"
    )

plt.xlabel("Кількість потоків індексування (n)")
plt.ylabel("Ефективність E(n)")
plt.title("Коефіцієнт ефективності паралелізації")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("efficiency_plot.png")
plt.show()
