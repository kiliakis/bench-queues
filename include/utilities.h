#pragma once
#include <optionparser.h>

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