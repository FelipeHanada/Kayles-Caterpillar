from nim_file_reader import NimFileReader
import os

SRC_DIR = "data/19-03/"
DST_DIR = "data/19-03-texto/"

for file in os.listdir(SRC_DIR):
    if file.endswith(".catnim"):
        input_file = SRC_DIR + file
        output_file = DST_DIR + file[:-6] + ".txt"

        elements_read = 0
        with NimFileReader(input_file) as reader, open(output_file, "w") as writer:
            elements_read = reader.__len__()
            
            writer.write(f"{reader.get_header()}\n")
            for nimber in reader:
                writer.write(f"{nimber} ")

        print(f"Converted {input_file} to {output_file}")
        print(f"Number of elements read: {elements_read}")
        print(f"Number of bytes in file: {os.path.getsize(input_file)}")