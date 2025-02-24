#!/bin/python
from glob import glob
from time import time_ns
import argparse
from sys import argv
from os.path import isdir

from charset_normalizer import detect
from chardet import detect as chardet_detect

from statistics import mean
from math import ceil


def calc_percentile(data, percentile):
    n = len(data)
    p = n * percentile / 100
    sorted_data = sorted(data)

    return sorted_data[int(p)] if p.is_integer() else sorted_data[int(ceil(p)) - 1]


def performance_compare(arguments):
    parser = argparse.ArgumentParser(
        description="Performance CI/CD check for Charset-Normalizer"
    )

    parser.add_argument('-s', '--size-increase', action="store", default=1, type=int, dest='size_coeff',
                        help="Apply artificial size increase to challenge the detection mechanism further")

    args = parser.parse_args(arguments)

    if not isdir("./char-dataset"):
        print("This script require https://github.com/Ousret/char-dataset to be cloned on package root directory")
        exit(1)

    chardet_results = []
    charset_normalizer_results = []

    for tbt_path in sorted(glob("./char-dataset/**/*.*")):
        print(tbt_path)

        # Read Bin file
        with open(tbt_path, "rb") as fp:
            content = fp.read() * args.size_coeff

        before = time_ns()
        chardet_detect(content)
        chardet_results.append(
            round((time_ns() - before) / 1000000000, 5)
        )
        print("  --> Chardet: " + str(chardet_results[-1]) + "s")
        before = time_ns()
        detect(content)
        charset_normalizer_results.append(
            round((time_ns() - before) / 1000000000, 5)
        )
        print("  --> Charset-Normalizer: " + str(charset_normalizer_results[-1]) + "s")

    chardet_avg_delay = mean(chardet_results)
    chardet_99p = calc_percentile(chardet_results, 99)
    chardet_95p = calc_percentile(chardet_results, 95)
    chardet_50p = calc_percentile(chardet_results, 50)

    charset_normalizer_avg_delay = mean(charset_normalizer_results)
    charset_normalizer_99p = calc_percentile(charset_normalizer_results, 99)
    charset_normalizer_95p = calc_percentile(charset_normalizer_results, 95)
    charset_normalizer_50p = calc_percentile(charset_normalizer_results, 50)

    print("")

    print("------------------------------")
    print("--> Chardet Conclusions")
    print("   --> Avg: " + str(chardet_avg_delay) + "s")
    print("   --> 99th: " + str(chardet_99p) + "s")
    print("   --> 95th: " + str(chardet_95p) + "s")
    print("   --> 50th: " + str(chardet_50p) + "s")

    print("------------------------------")
    print("--> Charset-Normalizer Conclusions")
    print("   --> Avg: " + str(charset_normalizer_avg_delay) + "s")
    print("   --> 99th: " + str(charset_normalizer_99p) + "s")
    print("   --> 95th: " + str(charset_normalizer_95p) + "s")
    print("   --> 50th: " + str(charset_normalizer_50p) + "s")

    return 0 if chardet_avg_delay > charset_normalizer_avg_delay and chardet_99p > charset_normalizer_99p else 1


if __name__ == "__main__":
    exit(
        performance_compare(
            argv[1:]
        )
    )
