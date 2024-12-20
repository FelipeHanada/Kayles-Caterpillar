import matplotlib.pyplot as plt
import numpy as np
from math import log2

DIR_PATH = 'data'
INPUT_FILE_PREFIX = 'caterpillar_nim_'
FILES = (4, 5, 6)
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
        n_0 = 0 if i == 0 else 3 + int(log2(x))

        axs[i].plot(range(n_0, n_0 + len(nimber)), nimber)
        axs[i].set_xlabel('n')
        axs[i].set_ylabel(f'C(n, {x}) eq nimber')
                
        axs[i].xaxis.set_major_locator(plt.MaxNLocator(integer=True))
        axs[i].yaxis.set_major_locator(plt.MaxNLocator(integer=True))

    plt.tight_layout()
    plt.show()
