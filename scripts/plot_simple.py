import matplotlib.pyplot as plt
import numpy as np
import sys
import os


def annotate(ax, A, B):
    for x, y in zip(A, B):
        ax.annotate('%.1e' % y, xy=(x, y), textcoords='data', size='12')


def annotate_min(A, B):
    y = min(B)
    i = B.index(y)
    # i = B[y]
    plt.subplot().annotate('%.2e' % float(y), xy=(A[i], y), size='large')


def annotate_max(A, B):
    y = max(B)
    i = B.index(y)

    # i = B[y]
    plt.subplot().annotate('%.2e' % float(y), xy=(A[i], y), size='large')


def import_results(file):
    d = np.genfromtxt(file, dtype='str', delimiter='\t')
    return d.tolist()


from itertools import cycle

markers = cycle(['o', 's', '*'])
cmap = plt.get_cmap('Greys')
colors = cycle(cmap(np.linspace(0.45, .85, 2)))
# colors = cycle([0.25, 0.75])


def plot(x, y, label, xerr=None, yerr=None):

    plt.grid(True, which='major', alpha=0.5)
    # plt.grid(linestyle='dotted')
    # plt.grid(True, which='minor', alpha=0.2)
    # plt.minorticks_on()
    plt.errorbar(
        x, y, xerr=xerr, yerr=yerr, marker=next(markers),
        linewidth='1', label=label,
        markersize=5, capsize=2, color=next(colors))


if __name__ == '__main__':
    if len(sys.argv) < 3:
        print("You should specify input file and output directory")
        exit(-1)
    input_file = sys.argv[1]
    outdir = sys.argv[2]
    if not os.path.exists(outdir):
        os.makedirs(outdir)

    l = import_results(input_file)
    header = l[0]
    l = l[1:]
    d = {}
    for line in l:
        exe = line[0]
        threads = int(line[1])
        types = line[2]
        throughput = float(line[-2])
        stdev = float(line[-1])
        if(exe not in d):
            d[exe] = {}
        if(types not in d[exe]):
            d[exe][types] = ([], [], [])
        d[exe][types][0].append(threads)
        d[exe][types][1].append(throughput)
        d[exe][types][2].append(stdev)

    # plt.figure(figsize=(4, 4))
    # # plt.tick_params(labelright=True)
    # plt.xlabel('Thread Pairs')
    # plt.ylabel('Throughput (Mops/s)')
    # plt.title('Push benchmark')

    # for exe in sorted(d):
    #     types = d[exe]
    #     if('push' in exe):
    #         x = types['producer'][0]
    #         y = types['producer'][1]
    #         err = types['producer'][2]
    #         plot(x, y, exe.split('-push')[0], yerr=err)

    # plt.legend(loc='best', fancybox=True, framealpha=0.4, fontsize='9')
    # # plt.savefig(outdir + '/push.pdf', bbox_inches='tight')
    # plt.tight_layout()
    # plt.close()

    # plt.figure(figsize=(4, 4))    # plt.tick_params(labelright=True)
    # plt.xlabel('Thread Pairs')
    # plt.ylabel('Throughput (Mops/s)')
    # plt.title('Pop benchmark')

    # for exe in sorted(d):
    #     types = d[exe]
    #     if('pop' in exe):
    #         x = types['consumer'][0]
    #         y = types['consumer'][1]
    #         err = types['consumer'][2]
    #         plot(x, y, exe.split('-pop')[0], yerr=err)

    # plt.legend(loc='best', fancybox=True, framealpha=0.3, fontsize='9')
    # # plt.savefig(outdir + '/pop.pdf', bbox_inches='tight')
    # plt.tight_layout()
    # plt.close()

    plt.figure(figsize=(3.5, 2.2))    # plt.tick_params(labelright=True)
    plt.xscale("log", basex=2)
    plt.xlabel('# Threads')
    plt.ylabel('Throughput (GOps/s)')
    # plt.title('Concurrent Push/Pop benchmark')

    for exe in sorted(d):
        types = d[exe]
        if('pop' not in exe) and ('push' not in exe):
            x = 2 * np.array(types['consumer'][0], dtype=int)
            y = np.array(types['consumer'][1]) + np.array(types['producer'][1])
            err = np.array(types['consumer'][2]) + \
                np.array(types['producer'][2])
            plot(x, y/1000, exe, yerr=err/1000)

    plt.xticks(x, x)
            
    plt.legend(loc='upper left', fancybox=True, framealpha=1, fontsize='9')
    plt.tight_layout()
    plt.savefig(outdir + '/concurrent-haswell.pdf', bbox_inches='tight', dpi=300)
    plt.show()
    plt.close()
