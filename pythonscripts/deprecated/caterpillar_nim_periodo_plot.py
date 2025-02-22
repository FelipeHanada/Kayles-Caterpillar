import matplotlib.pyplot as plt
import numpy as np

DIR_PATH = 'data/reduce3'

def read_nim_file(file_name):
    with open(file_name, 'rb') as file:
        return np.fromfile(file, dtype=np.uint32)

def get_periodicity(nimbers):
    # there is an interval at the start of the array that is not periodic
    # we need to find the first period that repeats
    # and return where it starts and its length
    # to do this, we will find the period size from the back of the array
    # and then check if the period repeats from the start of the array
    n = len(nimbers)
    period = 0
    for s in range(n):
        for p in range(1, (n - s) // 2 + 1):
            for i in range(s, n - p):
                if nimbers[i] != nimbers[i + p]:
                    break
            else:
                return s, p
    
    if period == 0:
        return 0, 0

N_FILES = 100
def main():
    start_data = []
    period_data = []
    for i in range(N_FILES):
        nimbers = read_nim_file(f'{DIR_PATH}/nim_file_{i}')
        start, period = get_periodicity(nimbers)
        start_data.append(start)
        period_data.append(period)
    
    # plot two subplots for start and period as line graphs
    fig, axs = plt.subplots(2, 1, figsize=(10, 10))
    axs[0].plot(range(N_FILES), start_data, marker='o')
    axs[0].set_xlabel('n')
    axs[0].set_ylabel('start of periodicity')
    axs[0].set_title('Start of periodicity for C(n, x)')
    axs[0].xaxis.get_major_locator().set_params(integer=True)
    axs[1].plot(range(N_FILES), period_data, marker='o')
    axs[1].set_xlabel('n')
    axs[1].set_ylabel('period of periodicity')
    axs[1].set_title('Period of periodicity for C(n, x)')
    axs[1].xaxis.get_major_locator().set_params(integer=True)
    plt.tight_layout()
    plt.show()


if __name__ == '__main__':
    main()
