import matplotlib.pyplot as plt
import numpy as np
from math import log2

DIR_PATH = 'data'
INPUT_FILE_PREFIX = 'caterpillar_nim_'
X = 100
N = 1000

if __name__ == '__main__':
    with open(DIR_PATH + '/' + INPUT_FILE_PREFIX + str(X), 'rb') as file:
        nimbers = np.fromfile(file, dtype=np.uint32, count=N)

    fig, axs = plt.subplots()
    n_0 = 0 if X == 0 else 3 + int(log2(X))
    axs.plot(range(n_0, n_0 + len(nimbers)), nimbers)

    axs.set_xlabel('n')
    axs.set_ylabel(f'C(n, {X}) eq nimber')

    axs.xaxis.set_major_locator(plt.MaxNLocator(integer=True))
    axs.yaxis.set_major_locator(plt.MaxNLocator(integer=True))
    plt.show()
