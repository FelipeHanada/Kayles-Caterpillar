INPUTFILE = "data/caterpillar_nim_1"
OUTPUTFILE = "data/caterpillar_nim_1.txt"

with open(INPUTFILE, "rb") as input_file, open(OUTPUTFILE, "w") as output_file:
    while True:
        data = input_file.read(4)  # 4 bytes for an unsigned int
        if not data:
            break
        n = int.from_bytes(data, byteorder='little')
        output_file.write(f"{n} ")
    output_file.write("\n")

    print(f"Converted {INPUTFILE} to {OUTPUTFILE}")
    print(f"Number of bytes in file: {input_file.tell()}")
    print(f"Number of bytes read: {input_file.tell()}")
    print(f"Number of elements read: {input_file.tell() // 4}")
