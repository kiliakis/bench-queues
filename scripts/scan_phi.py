import subprocess
import os
import time
#import signal


# print '\nCpp simulation\n'

# os.environ['GOMP_CPU_AFFINITY'] = '0 28 1 29 2 30 3 31 4 32 5 33 6 34 7 35 8 36 9 37 10 ' + \
#    '38 11 39 12 40 13 41 14 42 15 43 16 44 17 45 18 46 19 47 20 48 21 49 22 50 23 51 24 52 25 53 26 54 27 55'

project_dir = '/home/iliakis/git/bench-queues/'
exe_dir = '/home/iliakis/bench-queues/'
outfiles = '/home/iliakis/git/bench-queues/results/raw/v-double/'

exe_list = ['boost-static', 'boost-static-push', 'boost-static-pop',
            'boost-dynamic', 'boost-dynamic-push', 'boost-dynamic-pop',
            'folly', 'folly-push', 'folly-pop',
            'circularfifo', 'circularfifo-push', 'circularfifo-pop'
            ]
# exe_list = ['histogram4']
n_turns_list = ['100']
# n_elements_list = ['10000']
# n_threads_list = ['1', '2', '4', '8', '14', '28']
n_threads_list = ['1', '4', '16', '32', '57', '114']
repeats = 10

# os.chdir(exe_dir)
total_sims = len(n_turns_list) * len(n_threads_list) * len(exe_list) * repeats
print "Total runs: ", total_sims

current_sim = 0
for exe in exe_list:
    if('push' in exe) or ('pop' in exe):
        n_elems = '10000'
    else:
        n_elems = '100000'
    for n_turns in n_turns_list:
        for n_threads in n_threads_list:
            outdir = outfiles
            name = exe + '-' + 'n_e' + n_elems + 'n_t' + n_turns +\
                'n_thr' + n_threads
            if not os.path.exists(outdir):
                os.makedirs(outdir)
            # res = open(outdir + name + '.txt', 'w')
            stdout = open(outdir + name+'.stdout', 'w')
            # stderr = open(outdir + name+'.stderr', 'w')
            for i in range(0, repeats):
                exe_args = ['ssh',
                            'mic0',
                            '/home/iliakis/export-and-exec.sh',
                            'cmd',
                            exe_dir + exe,
                            '-e' + n_elems,
                            '-b' + '10000',
                            '-t' + n_turns,
                            '-m' + n_threads
                            ]
                print exe, n_turns, n_elems, n_threads, i
                output = subprocess.check_output(exe_args)
                current_sim += 1
                stdout.write(output)
                # time = output.split(
                #     'Elapsed time: ')[1].split('sec')[0]
                # res.write(time+'\n')
                print "%.2f %% is completed" % (100.0 * current_sim /
                                                total_sims)
