from math import factorial

def pairings(n, i=0, remaining=None, paired=None):
    if remaining == None:
        remaining = set(range(n))
    if paired == None:
        paired = []
    if i == n:
        assert len(paired)*2 == n
        yield paired
    else:
        if i in remaining:
            remaining.remove(i)
            for j in remaining:
                remaining.remove(j)
                for p in pairings(n, i+1, remaining, paired + [(i,j)]):
                    yield p
                remaining.add(j)
            remaining.add(i)
        else:
            for p in pairings(n, i+1, remaining, paired):
                yield p

def pairings_helper(n):
    remaining = set(range(n))
    paired = []
    def makepairings(i):
        if i == n:
            assert len(paired)*2 == n
            yield paired
        else:
            if i in remaining:
                remaining.remove(i)
                for j in remaining:
                    remaining.remove(j)
                    paired.append((i,j))
                    for p in makepairings(i+1):
                        yield p
                    paired.pop()
                    remaining.add(j)
                remaining.add(i)
            else:
                for p in makepairings(i+1):
                    yield p
    return makepairings(0)

def pairings_iterative(n):
    remaining = set(range(n))
    paired = []
    len_prev_stack = 0
    stack = [0]
    while stack:
        assert set(sum(list(map(list, paired)),[])).union(remaining) == set(list(range(n)))
        # print (stack, '\t', paired, '\t', remaining)
        i = stack[-1]
        # print (paired)
        pushing = len_prev_stack < len(stack)
        len_prev_stack = len(stack)
        if pushing:
            if i == n:
                assert len(paired)*2 == n
                stack.pop()
                yield paired[:]
                continue
            for j in range(0, n):
                if i!=j and j in remaining:
                    remaining.remove(i)
                    remaining.remove(j)
                    paired.append((i,j))
                    while i not in remaining and i < n:
                        i += 1
                    stack.append(i)
                    break
        else:
            assert paired[-1][0] == i
            j = paired.pop()[1]
            assert j not in remaining
            remaining.add(j)
            for j in range(j+1, n):
                if j!=i and j in remaining:
                    remaining.remove(j)
                    paired.append((i,j))
                    while i not in remaining and i < n:
                        i += 1
                    stack.append(i)
                    break
            else:
                remaining.add(i)
                stack.pop()

if __name__ == "__main__":
    from sys import argv
    for i in (list(pairings_iterative(int(argv[1])))): print (i)
    def num_pairings(n):
        return factorial(n)//(2**(n//2))//factorial(n//2)
    for i in range(14,14,2):
        print (i)
        iterpairs = list(pairings_iterative(i))
        helppairs = list(pairings(i))
        recupairs = list(pairings(i))
        npairs = num_pairings(i)
        assert len(recupairs) == npairs
        assert helppairs == recupairs
        assert iterpairs == recupairs
