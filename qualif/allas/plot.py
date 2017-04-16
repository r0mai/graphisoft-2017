from __future__ import print_function


def load(fname):
    with open(fname) as f:
        data = f.readlines()

    before = True
    after = False
    result = {}
    for i, line in enumerate(data):
        if before:
            if line.startswith('Csapat'):
                before = False
            continue
        if not line.strip():
            break

        bits = line.split()
        name = ' '.join(bits[1:-5])
        ps = map(float, bits[-5:])
        result[name] = ps

    return result


def timeformat(s):
    return '{}:{}'.format(s[:2], s[2:])


def values(names, data):
    order = [(data[name][4], name) for name in data.keys()]
    order.sort(reverse=True)
    places = {}
    for i, (_, name) in enumerate(order):
        places[name] = i + 1

    return [places[name] for name in names]


def get_top_names(n, data):
    names = []
    for k, v in data.iteritems():
        names.append((v[4], k))

    names.sort(reverse=True)
    return map(lambda x: x[1], names[:n])


def load_times(fnames):
    time_data = []
    for fname in fnames:
        data = load(fname)
        time_data.append((fname, data))

    return time_data


def plot_times(time_data):
    names = get_top_names(30, time_data[-1][1])
    table = [[''] + names]
    for line in time_data:
        table.append([timeformat(line[0])] + values(names, line[1]))

    with open('output.csv', 'wb') as f:
        for line in table:
            print('\t'.join(map(str, line)), file=f)


def diff(time_data):
    ns = {}
    for (fname, data) in time_data:
        for name, values in data.iteritems():
            ns[name] = ns.get(name, [])
            ns[name].append(values[4])

    sd = []
    for k, v in ns.iteritems():
        sd.append((max(v), v[-1] - v[0], k))

    sd.sort(reverse=True)
    for xs in sd:
        print('{:7} {:7}\t{}'.format(*xs))


if __name__ == '__main__':
    fs = [
        '1200',
        '1300',
        '1400',
        '1500',
        '1600',
        '1700',
        '1800',
        '1900',
        '1930',
        '2000',
        '2030',
        '2100',
        '2130',
        '2200',
        '2230',
        '2300']

    td = load_times(fs)
    diff(td)
    # plot_times(td)
