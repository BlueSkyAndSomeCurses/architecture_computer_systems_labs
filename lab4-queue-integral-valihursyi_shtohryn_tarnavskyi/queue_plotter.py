import csv
import matplotlib.pyplot as plt
import numpy as np
import scipy.stats as stats

def read_csv(filename):
    with open(filename, 'r') as file:
        reader = csv.reader(file)
        header = next(reader)
        data = {header[i]: [] for i in range(len(header))}

        for row in reader:
            for i in range(len(header)):
                data[header[i]].append(float(row[i]) if i > 0 else int(row[i]))

    return data
def read_csv_splits(filename):
    data = {}
    with open(filename, 'r') as file:
        reader = csv.reader(file)
        headers = next(reader)
        for header in headers[1:]:
            data[header] = {'order_splits': [], 'time': []}
        for row in reader:
            for i, header in enumerate(headers[1:], start=1):
                data[header]['order_splits'].append(float(row[0]))
                data[header]['time'].append(float(row[i]))
    return data


data_12_threads = read_csv_splits('integral_parallel_measurements/comparative_measurements_12_threads.csv')

def plot_order_splits_data(data):
    for func, values in data.items():
        plt.figure(figsize=(10, 6))
        plt.bar(values['order_splits'], values['time'], alpha=0.7, label=f'Function {func}')
        plt.title(f'Histogram of Time for 12 Threads - Function {func}')
        plt.xlabel('Splits')
        plt.ylabel('Time (ms)')
        plt.legend()
        plt.savefig(f'./integral_parallel_measurements/result_12_threads_func_{func}.png')


def plot_compare_histograms(data1, data2, label1="queue", label2="no queue"):
    threads = data1['threads/func']
    threads_str = [str(t) for t in threads]
    x = np.arange(len(threads))

    width = 0.35

    for func in range(1, 4):
        plt.figure(figsize=(8, 5))

        plt.bar(x - width/2, data1[str(func)], width, label=f'{label1}', color='green', alpha=0.7)
        plt.bar(x + width/2, data2[str(func)], width, label=f'{label2}', color='purple', alpha=0.7)

        plt.xlabel('Number of Threads')
        plt.ylabel('Execution Time (ms)')
        plt.title(f'Average execution Time for Function {func} by Number of Threads')
        plt.xticks(x, threads_str, rotation=30)
        plt.legend()
        plt.tight_layout()

        plt.savefig(f'./integral_parallel_measurements/function_{func}_comparison_histogram.png')

def plot_histogram(data):
    threads = data['threads/func']
    threads_str = [str(t) for t in threads]

    for func in range(1, 4):
        plt.figure()
        plt.bar(threads_str, data[str(func)], label=f'Function {func}')
        plt.xlabel('Number of Threads')
        plt.ylabel('Execution Time (ms)')
        plt.title(f'Execution Time for Function {func} by Number of Threads')
        plt.tight_layout()
        plt.legend()

        plt.xticks(threads_str, rotation=30)

        plt.savefig(f'./integral_parallel_measurements/function_{func}_comparison_histogram.png')

def plot_histogram_effectiveness(data):
    threads = data['threads/func']
    threads_str = [str(t) for t in threads]

    eff_data = [
        [data[str(func)][0] / (threads[i] * cur_time) for i, cur_time in enumerate(data[str(func)])]
        for func in range(1, 4)
    ]

    for func in range(1, 4):
        plt.figure()
        plt.bar(threads_str, eff_data[func-1], label=f'Function {func}')
        plt.xlabel('Number of Threads')
        plt.ylabel('Effectiveness')
        plt.title(f'Queue parallelization efficiency for Function {func} by Number of Threads')
        plt.tight_layout()
        plt.legend()

        plt.xticks(threads_str, rotation=30)

        plt.savefig(f'./integral_parallel_measurements/function_{func}_histogram_effectiveness.png')

def compare_with_wilcoxon(data1, data2):
    threads = data1['threads/func']
    threads_str = [str(t) for t in threads]
    results = {}

    for func in range(1, 4):
        values1 = data1[str(func)]
        values2 = data2[str(func)]

        stat, p = stats.wilcoxon(values1, values2)

        results[func] = (stats, p)
        print(f"Function {func}: Wilcoxon test statistic = {stat}, p-value = {p:.5f}")
        if p < 0.05:
            print(f"Function {func}: Samples are significantly different")

        else:
            print(f"Function {func}: Samples are mostly identical")

    return results


data1 = read_csv('integral_parallel_measurements/final_measurements.csv')
data2 = read_csv('integral_parallel_measurements/final_measurements_parallel.csv')
data_min = read_csv('integral_parallel_measurements/final_measurements_min.csv')

# plot_compare_histograms(data1, data2)
# compare_with_wilcoxon(data1, data2)
plot_histogram_effectiveness(data_min)