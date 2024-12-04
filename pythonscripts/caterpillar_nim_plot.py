import matplotlib.pyplot as plt
import numpy as np


INPUT_FILE = 'data/caterpillar_nim_1'
START = 0
N = 10000

if __name__ == '__main__':
    fig, ax = plt.subplots()

    with open(INPUT_FILE, mode='rb') as file:
        file.seek(START * 4)
        nimbers = np.fromfile(file, dtype=np.uint32, count=N)

    ax.plot(range(START, START + len(nimbers)), nimbers)

    ax.set_xlabel('n')
    ax.set_ylabel('C(n, 1) eq nimber')
    ax.yaxis.set_minor_locator(plt.MultipleLocator(1))
    ax.yaxis.set_major_locator(plt.MultipleLocator(1))
    ax.xaxis.set_minor_locator(plt.MultipleLocator(1))

    plt.show()
 