
def table_size(N):
    return (N*(N+1))//2

def table_index(i, j, N):
    if i < j:
        i, j = j, i

    ix = i + j*N

    if j >= N/2:
        ix = (N+1)*N-1 - ix

    return ix

def test_c(n):
    return chr(ord('a')+n)

def test_val(a, b):
    if a < b:
        a, b = b, a
    return test_c(a)+test_c(b)

def test_table(N):
    tab = [None]*(N*(N+1)//2)
    for i in range(N):
        for j in range(N):
            ix = table_index(i, j, N)
            v = test_val(i, j)
            assert tab[ix] in (None, v)
            tab[ix] = v
    for off in range(0, len(tab), N):
        print(' '.join(tab[off:off+N]))
    print()

if __name__ == '__main__':
    test_table(10)
    test_table(9)
