import matplotlib.pyplot as plt
import networkx as nx


FILE_NAME = "data/mex_set_clean/mex_set_0.txt"
INPUT_BY_FILE = True
MEX_CODE = True
LAST_N = 34
if __name__ == "__main__":
    graph = nx.DiGraph()

    last_vertices = []
    with open(FILE_NAME, "r") as file:
        first_line = file.readline().strip()
        if MEX_CODE:
            previous_vertex = 0
            for num in map(int, first_line.split()):
                previous_vertex += 2 ** num
        else:
            previous_vertex = first_line

        for line in file:
            current_line = line.strip()
            if MEX_CODE:
                current_vertex = 0
                for num in map(int, current_line.split()):
                    current_vertex += 2 ** num
            else:
                current_vertex = current_line

            last_vertices.append(current_vertex)
            if (len(last_vertices) > LAST_N):
                last_vertices.pop(0)

            graph.add_edge(previous_vertex, current_vertex)
            previous_vertex = current_vertex

    plt.figure(figsize=(10, 10))
    nx.draw(
        graph,
        nx.circular_layout(graph),
        with_labels=True,
        node_color=["red" if node in last_vertices else "lightgreen" for node in graph.nodes()],
        edge_color="blue",
        node_size=500,
        font_size=12,
        font_color="darkred",
        arrowsize=15,
        arrowstyle="->",
    )
    plt.show()
