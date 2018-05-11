#pragma once
#include <optionparser.h>
#include <stddef.h>
#include <unistd.h>
#include <algorithm>
// Kostis
#include <sched.h>
const int NUM_CORES = 28;
const int MIC_CORES = 228;

static cpu_set_t    full_cs;
cpu_set_t* proc_get_full_set(void)
{
    static int          inited = 0;

    if (inited == 0) {
        int i;
        int n_cpus;

        CPU_ZERO (&full_cs);
        n_cpus = sysconf(_SC_NPROCESSORS_ONLN);
        for (i = 0; i < n_cpus; i++) {
            CPU_SET(i, &full_cs);
        }

        inited = 1;
    }

    return &full_cs;
}

/* Bind the calling thread to run on CPU_ID. 
   Returns 0 if successful, -1 if failed. */
static inline int proc_bind_thread (int cpu_id)
{
    cpu_set_t   cpu_set;

    #ifdef __MIC__
        CPU_ZERO (&cpu_set);
        CPU_SET ((cpu_id+1) % MIC_CORES, &cpu_set);
        return sched_setaffinity (0, sizeof(cpu_set), &cpu_set)
    #else
        CPU_ZERO (&cpu_set);
        CPU_SET (cpu_id, &cpu_set);
        return sched_setaffinity (0, sizeof (cpu_set), &cpu_set);
    #endif
}

static inline int proc_unbind_thread ()
{
    return sched_setaffinity (0, sizeof (cpu_set_t), proc_get_full_set());
}

static inline int proc_get_cpuid (void)
{
    int i, ret;
    cpu_set_t cpu_set;
    
    ret = sched_getaffinity (0, sizeof (cpu_set), &cpu_set);
    if (ret < 0) return -1;

    for (i = 0; i < CPU_SETSIZE; ++i)
    {
        if (CPU_ISSET (i, &cpu_set)) break;
    }
    return i;
}

static inline double mean(std::vector<double> const &v)
{
    double m = 0;
    for (int i = 0; i < v.size(); ++i) {
        m += v[i];
    }
    return m / v.size();
};

static inline double stdev(std::vector<double> const &v, double m)
{
    double std = 0;
    for (int i = 0; i < v.size(); ++i) {
        std += (v[i] - m) * (v[i] - m);
    }
    return std::sqrt(std / v.size());
};




namespace util {
    struct Arg : public optionparser::Arg {
        static void printError(const char *msg1, const optionparser::Option &opt,
                               const char *msg2)
        {
            fprintf(stderr, "%s", msg1);
            fwrite(opt.name, opt.namelen, 1, stderr);
            fprintf(stderr, "%s", msg2);
        }

        static optionparser::ArgStatus Unknown(const optionparser::Option &option,
                                               bool msg)
        {
            if (msg)
                printError("Unknown option '", option, "'\n");
            return optionparser::ARG_ILLEGAL;
        }

        static optionparser::ArgStatus Required(const optionparser::Option &option,
                                                bool msg)
        {
            if (option.arg != 0)
                return optionparser::ARG_OK;

            if (msg)
                printError("Option '", option, "' requires an argument\n");
            return optionparser::ARG_ILLEGAL;
        }

        static optionparser::ArgStatus NonEmpty(const optionparser::Option &option,
                                                bool msg)
        {
            if (option.arg != 0 && option.arg[0] != 0)
                return optionparser::ARG_OK;

            if (msg)
                printError("Option '", option,
                           "' requires a non-empty argument\n");
            return optionparser::ARG_ILLEGAL;
        }

        static optionparser::ArgStatus Numeric(const optionparser::Option &option,
                                               bool msg)
        {
            // printf("Inside here\n");
            char *endptr = 0;
            if (option.arg != 0 && strtol(option.arg, &endptr, 10)) {
            };
            if (endptr != option.arg && *endptr == 0)
                return optionparser::ARG_OK;

            if (msg)
                printError("Option '", option,
                           "' requires a numeric argument\n");
            return optionparser::ARG_ILLEGAL;
        }
    };
}