
#include <iostream>
#include <random>
#include <chrono>
#include <boost_queue_static_batched.h>
#include <utilities.h>
#include <thread>
#include <atomic>

using namespace std;

atomic<int> fence;
typedef double data_t;
vector<circ_buffer<data_t> > queues;
vector<double> producer_times;
vector<double> consumer_times;

long int N_turns = 1;
long int N_elems = 100000;
int N_threads = 1;

void parse_args(int argc, char **argv);

void producer(int id)
{
    fence++;
    while (fence < 2 * N_threads) ;

    chrono::time_point<chrono::high_resolution_clock> start;
    chrono::duration<double> elapsed_time(0.0);
    start = chrono::system_clock::now();

    long int i = 0;
    while (i < N_elems) {
        if (queues[id].push(i))
            i++;
    }
    // printf("[Producer %d] Going for the last push!\n", id);

    // queues[id].push_final();

    elapsed_time = chrono::system_clock::now() - start;
    producer_times[id] += elapsed_time.count();

    // printf("[Producer %d] I am over!\n", id);
}

void consumer(int id)
{
    fence++;
    while (fence < 2 * N_threads) ;
    chrono::time_point<chrono::high_resolution_clock> start;
    chrono::duration<double> elapsed_time(0.0);
    start = chrono::system_clock::now();

    long int i = 0;
    data_t sum = 0;
    auto f = [&sum](data_t e) { sum+=e; return;};

    while (i < N_elems) {
        // data_t res;
        i += queues[id].consume_all(f);
        // cout << i << "\n";
        // if (queues[id].pop(res)) {
        //     i++;
        // }
    }

    elapsed_time = chrono::system_clock::now() - start;
    consumer_times[id] += elapsed_time.count();

    printf("[Consumer %d] I am over!, the sum is: %lf\n", id, sum);
}

int main(int argc, char *argv[])
{
    parse_args(argc, argv);
    cout << "Number of turns: " <<  N_turns << "\n";
    cout << "Number of elements: " << N_elems << "\n";
    cout << "Number of threads: " <<  N_threads << "\n";

    producer_times.resize(N_threads, 0);
    consumer_times.resize(N_threads, 0);

    for (int i = 0; i < N_threads; i++) {
        queues.push_back(circ_buffer<data_t>(1000, 100));
    }

    for (int t = 0; t < N_turns; t++) {
        // cout << "Starting turn " << t << "\n";
        vector<thread> threads;
        fence.store(0);
        for (int i = 0; i < N_threads; i++) {
            threads.push_back(thread(producer, i));
            threads.push_back(thread(consumer, i));
        }
        for (auto &th : threads) th.join();
    }

    // for (auto &t : producer_times) t = t / N_turns;
    // for (auto &t : consumer_times) t = t / N_turns;

    auto mean_producer_time = mean(producer_times);
    auto std_producer_time = stdev(producer_times, mean_producer_time);

    auto mean_consumer_time = mean(consumer_times);
    auto std_consumer_time = stdev(consumer_times, mean_consumer_time);

    double mean_producer_throughput = N_turns * N_elems / mean_producer_time / 1e6;
    double mean_consumer_throughput = N_turns * N_elems / mean_consumer_time / 1e6;

    // cout << "boost static\n";

    // cout << "mean producer time: " << mean_producer_time << " sec\n";
    // cout << "mean producer time std: " << std_producer_time << " sec\n";
    cout << "mean producer throughput: " << mean_producer_throughput << " Melems/sec\n";

    // cout << "mean consumer time: " << mean_consumer_time << " sec\n";
    // cout << "mean consumer time std: " << std_consumer_time << " sec\n";
    cout << "mean consumer throughput: " << mean_consumer_throughput << " Melems/sec\n";

    cout << "mean total throughput: "
         << mean_consumer_throughput + mean_producer_throughput
         << " Melems/sec\n";

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
