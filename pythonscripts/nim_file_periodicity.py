import nim_file_reader as nfr

def normalize_sequence(sequence):
    """Normalize a sequence by finding its lexicographically smallest rotation."""
    if sequence is None:
        return None
    n = len(sequence)
    return min(tuple(sequence[i:] + sequence[:i]) for i in range(n))

def run(catnim_file_path):
    with nfr.NimFileReader(catnim_file_path) as reader:
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
                periodic_sequence = nimbers[x0 - n0 : x0 - n0 + periodicity]
                break

        if periodicity is not None:
            break

    # identify non-periodic indices using the periodic sequence
    non_periodic_indices = []
    non_periodic_nimbers = []
    if periodicity is not None:
        for i in range(len(nimbers)):
            if nimbers[i] != periodic_sequence[(i - (x0 - n0)) % periodicity]:
                non_periodic_indices.append(n0 + i)
                non_periodic_nimbers.append(nimbers[i])

    if periodicity is None:
        return None

    return {
        'periodicity': periodicity,
        'periodic_sequence_x0': periodic_sequence,
        'periodic_sequence_length': len(periodic_sequence),
        'x0': x0,
        'non_periodic_indices': non_periodic_indices,
        'non_periodic_nimbers': non_periodic_nimbers,
    }


if __name__ == '__main__':
    print(run("data/19-03/nim_file_0.catnim"))