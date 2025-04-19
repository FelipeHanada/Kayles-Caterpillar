import matplotlib.pyplot as plt
from nim_file_reader import NimFileReader

CATNIM_FILE_PATH = 'data/19-03/nim_file_1.catnim'

if __name__ == '__main__':
    with NimFileReader(CATNIM_FILE_PATH) as reader:
        headers = reader.get_header()
        nimbers = [nimber for nimber in reader]

    print(headers)
    print(nimbers)

    fig, axs = plt.subplots()
    n_0 = headers['n0']
    axs.plot(range(n_0, n_0 + len(nimbers)), nimbers)

    axs.set_xlabel('n')
    axs.set_ylabel('nímero')

    axs.xaxis.set_major_locator(plt.MaxNLocator(integer=True))
    axs.xaxis.set_minor_locator(plt.MaxNLocator(integer=True))
    axs.yaxis.set_major_locator(plt.MaxNLocator(integer=True))

    fig.set_size_inches(8, 4.5)
    fig.savefig('nim_plot.png', dpi=300)
    plt.show()
