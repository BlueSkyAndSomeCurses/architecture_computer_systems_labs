import os
import subprocess
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from itertools import product
import time

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

executable = "./build/countwords_par_tbb"
config_path = "./configuration/config_generated"
os.makedirs(os.path.dirname(config_path), exist_ok=True)

indexing_threads_list = [1, 2, 4, 8, 12, 16]
merging_threads_list = [1, 2, 4]

results = []

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

    timing_dict = {}
    for entry in result_raw:
        if "=" in entry:
            key, val = entry.split("=")
            timing_dict[key.strip()] = float(val.strip())
    return timing_dict.get("Total", 0.0)

for indexing_threads, merging_threads in product(indexing_threads_list, merging_threads_list):
    times = []
    for _ in range(2):
        t = run_with_config(indexing_threads, merging_threads)
        times.append(t)
    avg_time = np.mean(times)
    results.append({
        "indexing_threads": indexing_threads,
        "merging_threads": merging_threads,
        "avg_time": avg_time
    })
    print(f"[RESULT] Indexing={indexing_threads}, Merging={merging_threads} -> Avg Time = {avg_time:.2f}s")

df = pd.DataFrame(results)

best_row = df.loc[df["avg_time"].idxmin()]
best_i = int(best_row["indexing_threads"])
best_m = int(best_row["merging_threads"])
best_t = float(best_row["avg_time"])

plt.figure(figsize=(10, 6))
for merging_threads in sorted(df["merging_threads"].unique()):
    subset = df[df["merging_threads"] == merging_threads]
    plt.plot(
        subset["indexing_threads"],
        subset["avg_time"],
        marker='o',
        label=f"Merging Threads = {merging_threads}"
    )

plt.axvline(best_i, color='red', linestyle='--', alpha=0.5, label=f"Оптимум: {best_i} / {best_m} → {best_t:.2f}s")
plt.xlabel("Кількість потоків індексування")
plt.ylabel("Середній час виконання (сек)")
plt.title("Час виконання для повної сітки потоків")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.show()

df.to_csv("results_grid_search.csv", index=False)

print(f"\n✅ Найшвидша конфігурація: Indexing={best_i}, Merging={best_m}, Time={best_t:.2f}s")
