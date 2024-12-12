import matplotlib.pyplot as plt
import numpy as np
import os
from math import log2

DIR_PATH = 'data'
INPUT_FILE_PREFIX = 'caterpillar_nim_'
N = 1000

if __name__ == '__main__':
    files = [f for f in os.listdir(DIR_PATH) if f.startswith(INPUT_FILE_PREFIX)]
    nimbers = []

    for file_name in files:
        with open(DIR_PATH + '/' + file_name, 'rb') as file:
            nimbers.append(np.fromfile(file, dtype=np.uint32, count=N))

    num_files = len(nimbers)
    fig, axs = plt.subplots(num_files, 1, figsize=(10, 5 * num_files))

    if num_files == 1:
        axs = [axs]

    for i, nimber in enumerate(nimbers):
        n_0 = 0 if i == 0 else 3 + int(log2(i))
        axs[i].plot(range(n_0, n_0 + len(nimber)), nimber)
        axs[i].set_xlabel('n')
        axs[i].set_ylabel(f'C(n, {i}) eq nimber')
        axs[i].yaxis.set_minor_locator(plt.MultipleLocator(1))
        axs[i].yaxis.set_major_locator(plt.MultipleLocator(1))
        axs[i].xaxis.set_minor_locator(plt.MultipleLocator(1))

    plt.tight_layout()
    plt.show()
