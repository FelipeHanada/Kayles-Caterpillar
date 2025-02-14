import matplotlib.pyplot as plt
import numpy as np
from math import log2

DIR_PATH = 'data/reduce3'
INPUT_FILE_PREFIX = 'nim_file_'
FILES = (2, 5, 6, 7, 8, 11)
N = 1000

if __name__ == '__main__':
    nimbers = []
    for file_i in FILES:
        with open(DIR_PATH + '/' + INPUT_FILE_PREFIX + str(file_i), 'rb') as file:
            nimbers.append(np.fromfile(file, dtype=np.uint32, count=N))

    num_files = len(nimbers)
    fig, axs = plt.subplots(num_files, 1, figsize=(10, 5 * num_files))

    if num_files == 1:
        axs = [axs]

    for i in range(len(FILES)):
        x = FILES[i]
        nimber = nimbers[i]

        axs[i].plot(range(len(nimber)), nimber)
        axs[i].set_xlabel('index')
        axs[i].set_ylabel(f'C(n, {x}) eq nimber')
                
        axs[i].xaxis.set_major_locator(plt.MaxNLocator(integer=True))
        axs[i].yaxis.set_major_locator(plt.MaxNLocator(integer=True))

    plt.tight_layout()
    plt.show()
