from nim_file_reader import NimFileReader

CATNIM_FILE_PATH = 'data/19-03/nim_file_1.catnim'

if __name__ == '__main__':
    # read file and search for periodicity
    # return the minimum value of x_0 such that nimbers[x] == nimbers[x + p] for all x >= x_0 until the end of the sequence 

    with NimFileReader(CATNIM_FILE_PATH) as reader:
        headers = reader.get_header()
        n0 = headers['n0']
        nimbers = [nimber for nimber in reader]
    
    # search for periodicity
    periodicity = None
    x0 = None
    for p in range(1, len(nimbers) // 2):
        for x in range(min(len(nimbers) - p, len(nimbers) // 2)):
            if all(nimbers[i] == nimbers[i + p] for i in range(x, len(nimbers) - p)):
                periodicity = p
                x0 = n0 + x
                break

        if periodicity is not None:
            break

    print(f'Periodicity: {periodicity}')
    if periodicity is not None:
        print(f'Periodic sequence: {nimbers[:periodicity]}')
        print(f'Periodic sequence length: {len(nimbers[:periodicity])}')
    if x0 is not None:
        print(f'Periodicity starting from x_0 = {x0}')
        print(f'x_0 - n0 = {x0 - n0}')
