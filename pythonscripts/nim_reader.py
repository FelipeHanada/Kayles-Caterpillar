class NimFileReader:
    HEADER_POS_N_REDUCED = 0
    HEADER_SIZE_N_REDUCED = 4
    HEADER_POS_X_CLASS = HEADER_POS_N_REDUCED + HEADER_SIZE_N_REDUCED
    HEADER_SIZE_X_CLASS = 4
    HEADER_POS_N0 = HEADER_POS_X_CLASS + HEADER_SIZE_X_CLASS
    HEADER_SIZE_N0 = 4
    HEADER_SIZE = HEADER_SIZE_N_REDUCED + HEADER_SIZE_X_CLASS + HEADER_SIZE_N0
    NIM_SIZE = 4

    def __init__(self, file_path: str):
        self.file_path = file_path
        self.file = open(file_path, 'rb')
        self.header = self.read_header()
        self.current_index = self.header['n0']

    def __del__(self):
        self.file.close()

    def __len__(self) -> int:
        self.file.seek(0, 2)
        return (self.file.tell() - self.HEADER_SIZE) // self.NIM_SIZE

    def __iter__(self):
        self.current_index = self.header['n0']
        return self

    def __next__(self):
        if self.current_index < self.header['n0'] + self.__len__():
            nim_value = self.read_nim(self.current_index)
            self.current_index += 1
            return nim_value
        else:
            raise StopIteration
        
    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        self.file.close()

    def read_header(self) -> dict:
        header = dict()
        self.file.seek(0)
        header['n_reduced'] = int.from_bytes(self.file.read(self.HEADER_SIZE_N_REDUCED), byteorder='little')
        header['x_class'] = int.from_bytes(self.file.read(self.HEADER_SIZE_X_CLASS), byteorder='little')
        header['n0'] = int.from_bytes(self.file.read(self.HEADER_SIZE_N0), byteorder='little')
        return header
    
    def get_header(self) -> dict:
        return self.header

    def read_nim(self, n: int) -> int:
        self.file.seek(self.HEADER_SIZE + max(0, n - self.header['n0']) * self.NIM_SIZE)
        return int.from_bytes(self.file.read(self.NIM_SIZE), byteorder='little')
    
    def read_all_nims(self) -> dict:
        nims = {}
        self.file.seek(self.HEADER_SIZE)

        for i in range(self.header['n0'], self.header['n0'] + self.__len__()):
            nims[i] = self.read_nim(i)

        return nims
