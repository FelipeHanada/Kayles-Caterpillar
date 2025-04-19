import csv
import nim_file_periodicity as nfp


p0 = nfp.run("data/19-03/nim_file_0.catnim")
A0 = p0["periodic_sequence_0"]
S0 = list(zip(p0["non_periodic_indices"], p0["non_periodic_nimbers"]))
S0.insert(0, (0, 0))

p3 = nfp.run("data/19-03/nim_file_3.catnim")
A3 = p3["periodic_sequence_0"]
S3 = list(zip(p3["non_periodic_indices"], p3["non_periodic_nimbers"]))
print(A3)
print(S3)

def generate_table(label, values, nlines=34):
    table = [label]

    for n in range(nlines):
        row = []
        for value in values:
            if callable(value):
                row.append(value(n))
            else:
                row.append(value)
        table.append(row)
    
    return table

def save_table_to_csv(table, filename):
    with open(filename, "w", newline="", encoding="utf-8") as file:
        writer = csv.writer(file, quoting=csv.QUOTE_NONE, escapechar="\\")

        for row in table:
            writer.writerow(row)

    print(f"CSV table saved to {filename}")

def save_table_to_latex(table, filename):
    with open(filename, "w", encoding="utf-8") as file:
        file.write("\\begin{tabular}{|" + "c|" * len(table[0]) + "}\n")
        file.write("    " + "\\hline\n")
        file.write("    " + " & ".join(map(str, table[0])) + "\\\\\n")
        file.write("    " + "\\hline\n")
        for row in table[1:]:
            formatted_row = " & ".join(map(str, row)) + "\\\\\n"
            file.write("    " + formatted_row)
        file.write("    " + "\\hline\n")

        file.write("\\end{tabular}\n")

    print(f"LaTeX table saved to {filename}")

def split_table(table, max_columns):
    split_tables = []
    for i in range(1, len(table[0]), max_columns):
        split_tables.append([[row[0]] + row[i:i + max_columns] for row in table])
    return split_tables

def break_table(table, max_rows):
    rtable = [[] for _ in range(max_rows)]
    for i, row in enumerate(table[1:]):
        rtable[i % max_rows] += row
    
    rtable.insert(0, [])
    while len(rtable[0]) < len(rtable[1]):
        rtable[0] += table[0]
    
    for i in range(len(rtable)):
        rtable[i] += ["" for _ in range(len(rtable[0]) - len(rtable[i]))]

    return rtable

def create_n_3_tables():
    table0 = generate_table(
        [
            "$n$",
            "$P_{n-1}$",
            "$P_{n-3} + K_1$",
            "$P_{n-4} + K_1$",
            "$P_{n-3} + P_2$",
            "$C_{n-2}$",
            "$C_{n-3}$",
            "$P_{n-5} + P_2 + K_1$",
            "$P_{n-6} + P_4$"
        ],
        [
            lambda n: n,
            lambda n: f"$*{A0[(n-1+34) % 34]}$",
            lambda n: f"$*{A0[(n-3+34) % 34] ^ 1}$",
            lambda n: f"$*{A0[(n-4+34) % 34] ^ 1}$",
            lambda n: f"$*{A0[(n-3+34) % 34] ^ 1}$",
            lambda n: f"$*{A3[(n-2+34) % 34]}$",
            lambda n: f"$*{A3[(n-3+34) % 34]}$",
            lambda n: f"$*{A0[(n-5+34) % 34]}$",
            lambda n: f"$*{A0[(n-6+34) % 34]}$",
        ]
    )
    save_table_to_latex(table0, "tabela0.tex")

    table1 = generate_table(
        ["$n/p$"] + [str(p[0]) for p in S3],
        [lambda n: str(n)] + [lambda n, p=p: f"$*{(p[1] ^ A0[(n-3-(p[0] % 34)+34) % 34])}$" for p in S3]
    )
    save_table_to_latex(table1, "tabela1.tex")

    table2 = generate_table(
        ["$n/q$"] + [str(q[0]) for q in S0],
        [lambda n: str(n)] + [lambda n, q=q: f"$*{(A3[(n-3-(q[0] % 34) + 34) % 34] ^ q[1])}$" for q in S0]
    )
    save_table_to_latex(table2, "tabela2.tex")

    table3 = generate_table(
        ["$n/p$"] + [str(i) for i in range(34)],
        [lambda n: str(n)] + [lambda n, i=i: f"$*{(A3[i] ^ A0[(n-3-i+34) % 34])}$" for i in range(34)]
    )
    for i, table in enumerate(split_table(table3, 17)):
        save_table_to_latex(table, f"tabela3_parte_{i+1}.tex")

if __name__ == "__main__":
    transient_table = generate_table(
        ["$k$", "$n_0$"],
        [lambda n: str(n), lambda n: nfp.run(f"data/19-03/nim_file_{n}.catnim")["x0"] - 1],
        nlines=200
    )

    save_table_to_latex(break_table(transient_table, 25), "tabela_transiente.tex")
