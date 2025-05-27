import matplotlib.pyplot as plt
from nim_file_reader import NimFileReader

def plot_nim_file_graph(catnim_file_path, output_path='nim_plot.png'):
    with NimFileReader(catnim_file_path) as reader:
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
    fig.savefig(output_path, dpi=300)
    plt.show()

def plot_text_file_graph(text_file_path, output_path='text_plot.png'):
    with open(text_file_path, 'r') as file:
        lines = file.readlines()

    nimbers = []
    for line in lines:
        nimbers += list(map(int, line.strip().split()))

    fig, axs = plt.subplots()
    axs.plot(range(len(nimbers)), nimbers)

    axs.set_xlabel('n')
    axs.set_ylabel('nímero')

    axs.xaxis.set_major_locator(plt.MaxNLocator(integer=True))
    axs.xaxis.set_minor_locator(plt.MaxNLocator(integer=True))
    axs.yaxis.set_major_locator(plt.MaxNLocator(integer=True))

    fig.set_size_inches(8, 4.5)
    fig.savefig(output_path, dpi=300)
    plt.show()

if __name__ == '__main__':
    # plot_nim_file_graph('data/19-03/nim_file_1.catnim')
    plot_text_file_graph('data/nimbers caterpillar 01 0 10.txt')
