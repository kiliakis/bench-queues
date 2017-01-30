
#include <iostream>
#include <random>
#include <chrono>
#include <cameron_queue.h>
#include <utilities.h>
#include <utility>
#include <thread>
#include <atomic>

using namespace std;

atomic<int> fence;
typedef std::pair<uint64_t, uint64_t> data_t;

vector<circ_buffer<data_t> > queues;
vector<double> producer_times;
vector<double> consumer_times;

long int N_turns = 1;
long int N_elems = 100000;
long int buf_size = 10000;
int N_threads = 1;

void parse_args(int argc, char **argv);

void producer(int id)
{
    fence++;
    while (fence < N_threads) ;

    chrono::time_point<chrono::high_resolution_clock> start;
    chrono::duration<double> elapsed_time(0.0);
    start = chrono::system_clock::now();

    long int i = 0;
    while (i < N_elems) {
        if (queues[id].push({i, i}))
            i++;
    }

    elapsed_time = chrono::system_clock::now() - start;
    producer_times[id] += elapsed_time.count();

    // printf("[Producer %d] I am over!\n", id);
}

void consumer(int id)
{
    fence++;
    while (fence < N_threads) ;
    chrono::time_point<chrono::high_resolution_clock> start;
    chrono::duration<double> elapsed_time(0.0);
    start = chrono::system_clock::now();

    long int i = 0;
    while (i < N_elems) {
        data_t res;
        if (queues[id].pop(res)) {
            i++;
        }
    }

    elapsed_time = chrono::system_clock::now() - start;
    consumer_times[id] += elapsed_time.count();

    // printf("[Consumer %d] I am over!\n", id);
}

int main(int argc, char *argv[])
{
    parse_args(argc, argv);
    cout << "Number of turns: " <<  N_turns << "\n";
    cout << "Number of elements: " << N_elems << "\n";
    cout << "Number of threads: " <<  N_threads << "\n";
    // cout << "Buffer size: " <<  buf_size << "\n";

    // producer_times.resize(N_threads, 0);
    consumer_times.resize(N_threads, 0);

    for (int t = 0; t < N_turns; t++) {
        vector<thread> threads;
        fence.store(0);
        for (int i = 0; i < N_threads; i++) {
            queues.push_back(circ_buffer<data_t>(N_elems+1, 100));
            for(long int j = 0; j< N_elems; j++){
                queues[i].push({j, j});
            }
        }
        for (int i = 0; i < N_threads; i++) {
            // threads.push_back(thread(producer, i));
            threads.push_back(thread(consumer, i));
        }
        for (auto &th : threads) th.join();
        queues.clear();
    }

    // for (auto &t : producer_times) t = t / N_turns;
    // for (auto &t : consumer_times) t = t / N_turns;

    // auto mean_producer_time = mean(producer_times);
    // auto std_producer_time = stdev(producer_times, mean_producer_time);

    auto mean_consumer_time = mean(consumer_times);
    // auto std_consumer_time = stdev(consumer_times, mean_consumer_time);

    // double mean_producer_throughput = N_turns * N_elems / mean_producer_time / 1e6;
    double mean_consumer_throughput = N_turns * N_elems / mean_consumer_time / 1e6;

    // cout << "folly dynamic\n";

    // cout << "mean producer time: " << mean_producer_time << " sec\n";
    // cout << "mean producer time std: " << std_producer_time << " sec\n";

    // cout << "mean producer time/operation: " << 1e12 * mean_producer_time / N_turns / N_elems << " picosec\n";
    // cout << "mean producer time/operation std: " << 1e12 * std_producer_time / N_turns / N_elems << " picosec\n";
    // cout << "mean producer throughput: " << mean_producer_throughput << " Melems/sec\n";

    // cout << "mean consumer time: " << mean_consumer_time << " sec\n";
    // cout << "mean consumer time std: " << std_consumer_time << " sec\n";
    // cout << "mean consumer time/operation: " << 1e12 * mean_consumer_time / N_turns / N_elems << " picosec\n";
    // cout << "mean consumer time/operation std: " << 1e12 * std_consumer_time / N_turns / N_elems << " picosec\n";
    cout << "mean consumer throughput: " << mean_consumer_throughput << " Melems/sec\n";

    return 0;
}


void parse_args(int argc, char **argv)
{
    using namespace std;
    using namespace optionparser;

    enum optionIndex {
        UNKNOWN,
        HELP,
        N_THREADS,
        N_TURNS,
        N_ELEMENTS,
        BUF_SIZE,
        OPTIONS_NUM
    };

    const Descriptor usage[] = {
        {
            UNKNOWN, 0, "", "", Arg::None, "USAGE: ./executable [options]\n\n"
            "Options:"
        },
        {
            HELP, 0, "h", "help", Arg::None,
            "  --help,              -h        Print usage and exit."
        },
        {
            N_TURNS, 0, "t", "turns", util::Arg::Numeric,
            "  --turns=<num>,       -t <num>  Number of turns (default: 500)"
        },
        {
            N_ELEMENTS, 0, "e", "elements", util::Arg::Numeric,
            "  --elements=<num>,   -e <num>  Number of elements (default: "
            "100k)"
        },
        {
            BUF_SIZE, 0, "b", "buf_size", util::Arg::Numeric,
            "  --buf_size=<num>,   -b <num>  Buffer size (default: "
            "1000)"
        },
        {
            N_THREADS, 0, "m", "threads", util::Arg::Numeric,
            "  --threads=<num>,     -m <num>  Number of producer/consumer threads (default: 1)"
        },
        {
            UNKNOWN, 0, "", "", Arg::None,
            "\nExamples:\n"
            "\t./executable\n"
            "\t./executable -t 1000 -e 10000 -m 4\n"
        },
        {0, 0, 0, 0, 0, 0}
    };

    argc -= (argc > 0);
    argv += (argc > 0); // skip program name argv[0] if present
    Stats stats(usage, argc, argv);
    vector<Option> options(stats.options_max);
    vector<Option> buffer(stats.buffer_max);
    Parser parse(usage, argc, argv, &options[0], &buffer[0]);

    if (options[HELP]) {
        printUsage(cout, usage);
        exit(0);
    }

    for (int i = 0; i < parse.optionsCount(); ++i) {
        Option &opt = buffer[i];
        // fprintf(stdout, "Argument #%d is ", i);
        switch (opt.index()) {
            case HELP:
            // not possible, because handled further above and exits the program
            case N_TURNS:
                N_turns = atoi(opt.arg);
                // fprintf(stdout, "--numeric with argument '%s'\n", opt.arg);
                break;
            case N_THREADS:
                N_threads = atoi(opt.arg);
                // fprintf(stdout, "--numeric with argument '%s'\n", opt.arg);
                break;
            case BUF_SIZE:
                buf_size = atoi(opt.arg);
                // fprintf(stdout, "--numeric with argument '%s'\n", opt.arg);
                break;
            case N_ELEMENTS:
                N_elems = atoi(opt.arg);
                // fprintf(stdout, "--numeric with argument '%s'\n", opt.arg);
                break;
            case UNKNOWN:
                // not possible because Arg::Unknown returns ARG_ILLEGAL
                // which aborts the parse with an error
                break;
        }
    }
}
