import random
import sys
import subprocess
import glob
import numpy as np
import re
from PIL import Image
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib.animation import FFMpegWriter

import os
import re
from PIL import Image
import matplotlib.pyplot as plt
import matplotlib.animation as animation

def create_heatmap_animation(folder_path, output_path='animation.mp4', fps=5):
    def get_index(filename):
        match = re.search(r'snapshot_(\d+)\.png', filename)
        return int(match.group(1)) if match else float('inf')

    files = sorted(
        [f for f in os.listdir(folder_path) if f.startswith("snapshot_") and f.endswith(".png")],
        key=get_index
    )

    images = [Image.open(os.path.join(folder_path, f)) for f in files]

    Vmin, Vmax = 0.0, 100.0
    fig, ax = plt.subplots()
    im = ax.imshow(images[0], animated=True, cmap='jet', vmin=Vmin, vmax=Vmax)
    cbar = fig.colorbar(im, ax=ax)
    cbar.set_label('Temperature (°C)')

    def update(frame):
        im.set_data(images[frame])
        return [im]

    ani = animation.FuncAnimation(fig, update, frames=len(images), blit=True)

    writer = FFMpegWriter(fps=fps)
    ani.save(output_path, writer=writer)
    print(f"Animation saved to {output_path}")

def generate_empty_field(n: int, m: int, out_file: str):
    with open(out_file, 'w') as f:
        f.write(','.join([str(random.randint(0, 100)) for _ in range(m)]) + '\n')
        for i in range(n-2):
            f.write(','.join([str(random.randint(0, 100))] + ['0'] * (m - 2) + [str(random.randint(0, 100))]) + '\n')
        f.write(','.join([str(random.randint(0, 100)) for _ in range(m)]) + '\n')

def generate_field(n: int, m: int, out_file: str):
    with open(out_file, 'w') as f:
        for i in range(n):
            f.write(','.join([str(random.randint(0, 100)) for _ in range(m)]) + '\n')

def generate_simple_field(n: int, m: int, out_file: str, left: int = 0, right: int = 0, up: int = 0, down: int = 0):
    with open(out_file, 'w') as f:
        f.write(','.join([str(up)] * m)  + '\n')

        for i in range(n-2):
            f.write(','.join([str(left)] + ['0'] * (m - 2) + [str(right)]) + '\n')

        f.write(','.join([str(down)] * m) + '\n')


EXECUTABLE_NAME = glob.glob("**/temperature_simulation", recursive=True)[-1]

def main(call_times: int, node_num: int, config_file: str):
    results = []

    for i in range(call_times):
        print(f"Running {i + 1} times")
        result = subprocess.run(
                        [
                            'mpirun',
                            '-np', str(node_num),
                            EXECUTABLE_NAME,
                            config_file
                        ],
                        stdout=subprocess.PIPE,
                    ).stdout.decode("utf-8")

        result = int(result.split('=')[1].strip())
        results.append(result)

    min_time = min(results)
    avg_time = sum(results) / len(results)
    std_time = np.std(results)

    print(f"Min time: {min_time}")
    print(f"Avg time: {avg_time}")
    print(f"Std of time: {std_time}")

    return min_time, avg_time, std_time




if __name__ == '__main__':
    n = 25
    m = 25
    out_file = '../data/data1/start_dist.csv'
    generate_simple_field(n, m, out_file, left=100, right=100, up=100, down=100)
    main(5, 3, '../data/data1.cfg')
    create_heatmap_animation('../data/pictures', 'animation.mp4', fps=2)