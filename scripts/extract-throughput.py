#!/usr/bin/python
import os
import csv
import sys
import numpy as np


def string_between(string, before, after):
    temp = string
    if before:
        temp = temp.split(before)[1]
    if after:
        temp = temp.split(after)[0]
    return temp


def extract_results(input, out):
    header = ['exe', 'threads', 'type', 'elems', 'turns',
              'throughput', 'stdev']
    records = []
    for dirs, subdirs, files in os.walk(input):
        for file in files:
            if ('.stdout' not in file):
                continue
            d = {
                'total': [],
                'producer': [],
                'consumer': []}

            threads = string_between(file, 'n_thr', '.')
            elems = string_between(file, 'n_e', 'n_t')
            turns = string_between(file, 'n_t', 'n_thr')
            exe = string_between(file, '', '-n_e')
            #implementation = dirs.split('/')[-2]
            for line in open(os.path.join(dirs, file), 'r'):
                if 'throughput:' in line:
                    temp = string_between(line, ':', 'M')
                    types = string_between(line, 'mean', 'throughput')
                    d[types.strip()].append(float(temp.strip()))
            for k, v in d.items():
                if not v:
                    continue
                records.append([exe, threads, k, elems, turns,
                                np.mean(v), np.std(v)])
                print file
                percent = 100.0 * np.std(v) / np.mean(v)
                if percent > 10:
                    print "The previous file has %.2f %% error" % percent
    records.sort(key=lambda a: (a[0], int(a[1]), int(a[2])))
    writer = csv.writer(open(out, 'w'), lineterminator='\n', delimiter=',')
    writer.writerow(header)
    writer.writerows(records)


def import_results(output_file):
    d = np.loadtxt(output_file, dtype='str')
    return d.tolist()


if __name__ == '__main__':
    if len(sys.argv) < 3:
        print "You should specify input directory and output file"
        exit(-1)
    input_dir = sys.argv[1]
    output_file = sys.argv[2]
    extract_results(input_dir, output_file)
