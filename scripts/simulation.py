import argparse
import json
import statistics
import matplotlib.pyplot as plt
import itertools
import numpy

_color_cycle = itertools.cycle(plt.rcParams['axes.prop_cycle'].by_key()['color'])

def add_distribution(data, bins=20, label=None):
    ax = plt.gca()

    color = next(_color_cycle)

    ax.hist(
        data,
        bins=bins,
        alpha=0.4,
        edgecolor="black",
        color=color,
        label=label,
        density=True
    )

    mean = statistics.mean(data)
    std = statistics.stdev(data) if len(data) > 1 else 0

    ax.axvline(mean, color=color, linestyle="--")
    ax.axvline(mean - std, color=color, linestyle=":")
    ax.axvline(mean + std, color=color, linestyle=":")

def get_pve_values_by_name(pve, name):
    values = []

    for run in pve:
        for result in run["results"]:
            if result["name"] == name:
                values.append(result["score"])
    return values

def get_pvp_values_by_names(pvp, name1, name2):
    values_first  = []
    values_second = []
    for run in pvp:
        for result in run["results"]:
            if result["first-name"] == name1 and result["second-name"] == name2:
                values_first.append(result["first-score"])
                values_second.append(result["second-score"])
    return values_first, values_second

def parse_pve(results):
    plt.figure(figsize=(8, 5))
    values_smart = get_pve_values_by_name(results, "Smart")
    values_dumb  = get_pve_values_by_name(results, "Dumb")
    all_data = values_smart + values_dumb
    bins=numpy.linspace(min(all_data), max(all_data), 50)

    add_distribution(values_smart, bins=bins, label="Smart")
    add_distribution(values_dumb,  bins=bins, label="Dumb")

    plt.legend()
    plt.grid(alpha=0.3)
    plt.title("Bots PvE comparison (distribution of scores in the end of game)")
    plt.show()

def parse_pvp(results):
    values_dumb, values_smart = get_pvp_values_by_names(results, "Dumb", "Smart")

    wins = []
    for d, s in zip(values_dumb, values_smart):
        if   d > s: wins.append(-1)
        elif d < s: wins.append( 1)
        else:       wins.append( 0)

    wins_mean = statistics.mean(wins)
    wins_sigma = statistics.stdev(wins)
    print(f"Wins mean and sigma (-1='dumb won', 0='draw', 1='smart won'): {wins_mean} +- {wins_sigma}")

    smart_wins_count = 0
    total_count = 0
    for i in wins:
        total_count += 2
        if i == 1:
            smart_wins_count += 2
        elif i == 0:
            smart_wins_count += 1
    smart_winrate = smart_wins_count / total_count
    print(f"Smart bot winrate: {smart_winrate}")


def main():
    parser = argparse.ArgumentParser(description="Process JSON simulation output")
    parser.add_argument("input", help="Input JSON simulation output")
    args = parser.parse_args()

    with open(args.input, "r", encoding="utf-8") as f:
        data = json.load(f)

    parse_pve(data["pve"])
    parse_pvp(data["pvp"])

if __name__ == "__main__":
    main()
