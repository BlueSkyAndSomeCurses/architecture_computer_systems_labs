import csv
import matplotlib.pyplot as plt

def read_csv(filename):
    with open(filename, 'r') as file:
        reader = csv.reader(file)
        header = next(reader)
        data = {header[i]: [] for i in range(len(header))}

        for row in reader:
            for i in range(len(header)):
                data[header[i]].append(float(row[i]) if i > 0 else int(row[i]))

    return data

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

        plt.savefig(f'./integral_parallel_measurements/function_{func}_histogram.png')

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
        plt.title(f'Parallelization efficiency for Function {func} by Number of Threads')
        plt.tight_layout()
        plt.legend()

        plt.xticks(threads_str, rotation=30)

        plt.savefig(f'./integral_parallel_measurements/function_{func}_histogram_effectiveness.png')

def compare_histogram(data1, data2, label1='12-thread system', label2='20-thread system'):
    threads = data1['threads/func']
    threads_str = [str(t) for t in threads]

    for func in range(1, 4):
        plt.figure()
        plt.bar(threads_str, data1[str(func)], label=f'{label1}', alpha=0.5, color='blue')
        plt.bar(threads_str, data2[str(func)], label=f'{label2}', alpha=0.5, color = 'green')
        plt.xlabel('Number of Threads')
        plt.ylabel('Execution Time (ms)')
        plt.title(f'Execution Time for Function {func} by Number of Threads')
        plt.tight_layout()
        plt.legend()

        plt.xticks(threads_str, rotation=30)

        plt.savefig(f'./integral_parallel_measurements/function_{func}_histogram_comparison.png')

def compare_histogram_effectiveness(data1, data2, label1='12-thread system', label2='20-thread system'):
    threads = data1['threads/func']
    threads_str = [str(t) for t in threads]

    eff_data1 = [
        [data1[str(func)][0] / (threads[i] * cur_time) for i, cur_time in enumerate(data1[str(func)])]
        for func in range(1, 4)
    ]

    eff_data2 = [
        [data2[str(func)][0] / (threads[i] * cur_time) for i, cur_time in enumerate(data2[str(func)])]
        for func in range(1, 4)
    ]

    for func in range(1, 4):
        plt.figure()
        plt.bar(threads_str, eff_data1[func-1], label=f'{label1}', alpha=0.5, color='blue')
        plt.bar(threads_str, eff_data2[func-1], label=f'{label2}', alpha=0.5, color='green')
        plt.xlabel('Number of Threads')
        plt.ylabel('Effectiveness')
        plt.title(f'Parallelization efficiency for Function {func} by Number of Threads')
        plt.tight_layout()
        plt.legend()

        plt.xticks(threads_str, rotation=30)

        plt.savefig(f'./integral_parallel_measurements/function_{func}_histogram_effectiveness_comparison.png')

if __name__ == "__main__":
    # filename = './integral_parallel_measurements/final_measurements.csv'
    # data = read_csv(filename)
    # plot_histogram(data)
    # plot_histogram_effectiveness(data)

    data1 = read_csv('./integral_parallel_measurements/final_measurements.csv')
    data2 = read_csv('./integral_parallel_measurements/measurements_oleg.csv')
    compare_histogram(data1, data2)
    compare_histogram_effectiveness(data1, data2)

