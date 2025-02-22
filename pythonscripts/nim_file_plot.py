import matplotlib.pyplot as plt
import numpy as np
from nim_file_reader import NimFileReader

CATNIM_FILE_PATH = 'data/reduce3newfileformat/nim_file_0.catnim'

if __name__ == '__main__':
    with NimFileReader(CATNIM_FILE_PATH) as reader:
        headers = reader.get_header()
        nimbers = [nimber for nimber in reader]

    fig, axs = plt.subplots()
    n_0 = headers['n0']
    axs.plot(range(n_0, n_0 + len(nimbers)), nimbers)

    axs.set_xlabel('n')
    axs.set_ylabel(f'C(n, {headers['x_class']}) eq nimber')

    axs.xaxis.set_major_locator(plt.MaxNLocator(integer=True))
    axs.xaxis.set_minor_locator(plt.MaxNLocator(integer=True))
    axs.yaxis.set_major_locator(plt.MaxNLocator(integer=True))
    plt.show()
