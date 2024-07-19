import sys
from os.path import join

import numpy as np
import matplotlib.pyplot as plt
import pandas


def get_params(data):
    r0 = data.iloc[0]
    return {
        "vr": r0["vr"],
        "mad": r0["mad"],
        "mps": r0["mps"],
        "pr": r0["pr"],
    }


def get_rates(d, params):
    return d.loc[
        (d["vr"] == params["vr"]) &
        (d["pr"] == params["pr"]) &
        (d["mad"] == params["mad"]) &
        (d["mps"] == params["mps"])
    ]


def get_title(params):
    return f"$A_r = {params['vr']}, D_{{max}} = {params['mad']}, N_{{P}}^{{max}} = {params['mps']}, R = {params['pr']}$"


def get_fn(params):
    return f"plsize_vr_{params['vr']}_mad_{params['mad']}_mps_{params['mps']}_pr_{params['pr']}.pdf"


def discrete_violin_plot(data, rate_data):

    labels = data["bin"].unique()
    labels.sort()
    params = get_params(data)
    rates = get_rates(rate_data, params)
    # split dataset into an array of arrays. the first index depends on the bin (1 km, 2 km, ...)
    # the second array is the list of platoon sizes
    data_sets = []
    for bn in labels:
        data_sets.append(np.array(data[data["bin"] == bn]["platoonSize"]))
    miny = 2
    maxy = int(params["mps"])
    if params["mps"] == 25:
        by = 2
    else:
        by = 1
    fig, ax = plt.subplots()
    plot_bins = np.arange(miny - 0.5, maxy + 1.5, step=1)

    ax.set_ylabel("Platoon size [\#]")
    ax.set_xlabel("Observation point [km]")
    ax.set_xticks(list(range(0, 10)))
    ax.set_xticklabels([f"${x}$" for x in list(range(1, 11))])
    ax.set_yticks(list(range(miny, maxy+1, by)))
    if by == 2:
        ax.set_yticks(list(range(miny, maxy+1)), minor=True)
    ax.set_yticklabels([f"${x}$" for x in list(range(miny, maxy+1, by))])
    ax.set_axisbelow(True)
    ax.grid(axis='y', which='both')
    t = plt.text(.02, .95, f'$N_{{P}}^{{max}} = {int(params["mps"])}$', ha='left', va='top', transform=ax.transAxes)
    t.set_bbox(dict(facecolor='white', alpha=0.5, linewidth=0))
    for i in range(len(labels)):
        n, bins, rects = ax.hist(data_sets[i], bins=plot_bins, density=True, bottom=i+1, orientation="horizontal", rwidth=.9, color='cornflowerblue', edgecolor='black', linewidth=0.1)
        for r in rects:
            r.set_width(r.get_width() * 0.95)
            r.set_x(r.get_x() - r.get_width() / 2)
            if r.get_width() == 0:
                r.set(edgecolor=None)

        ax.plot(i+1, data_sets[i].min(), marker='o', markersize=1, color="black")
        ax.plot(i+1, data_sets[i].max(), marker='o', markersize=1, color="black")

    plt.ylim([miny-1, maxy+1])
    plt.xlim([-1, 10])
    # plt.title(get_title(params))

    plt.annotate('$\eta$', (-1, (maxy+1)+0.02*(maxy-miny)), annotation_clip=False)
    for i, r in rates.iterrows():
        plt.annotate(f'${r["ipr"]:.2f}$', (r["bin"]/1000-1, (maxy+1)+0.02*(maxy-miny)), ha='center', annotation_clip=False)
    plt.savefig(get_fn(params), format="pdf", bbox_inches='tight')
    # plt.show()
    # sys.exit(0)


def plot_violin_plot(data, rate_data):
    labels = data["bin"].unique()
    labels.sort()
    params = get_params(data)
    rates = get_rates(rate_data, params)
    # split dataset into an array of arrays. the first index depends on the bin (1 km, 2 km, ...)
    # the second array is the list of platoon sizes
    data_sets = []
    for bn in labels:
        data_sets.append(np.array(data[data["bin"] == bn]["platoonSize"]))
    fig, ax = plt.subplots()
    ax.violinplot(data_sets, widths=0.8, showmeans=False, showextrema=True)
    ax.set_ylabel("Platoon size [\#]")
    ax.set_xlabel("Observation point [km]")
    ax.set_xticks(list(range(0, 10)))
    ax.set_xticklabels([f"${x}$" for x in list(range(1, 11))])
    miny = 2
    maxy = int(params["mps"])
    if params["mps"] == 25:
        by = 2
    else:
        by = 1
    ax.set_yticks(list(range(miny, maxy+1, by)))
    if by == 2:
        ax.set_yticks(list(range(miny, maxy+1)), minor=True)
    ax.set_yticklabels([f"${x}$" for x in list(range(miny, maxy+1, by))])
    ax.set_axisbelow(True)
    ax.grid(axis='y', which='both')
    t = plt.text(.02, .95, f'$N_{{P}}^{{max}} = {int(params["mps"])}$', ha='left', va='top', transform=ax.transAxes)
    t.set_bbox(dict(facecolor='white', alpha=0.5, linewidth=0))
    plt.ylim([miny-1, maxy])
    plt.xlim([-1, 10])
    # plt.title(get_title(params))

    plt.annotate('$\eta$', (-1, maxy+0.02*(maxy-miny)), annotation_clip=False)
    for i, r in rates.iterrows():
        plt.annotate(f'${r["ipr"]:.2f}$', (r["bin"]/1000-1, maxy+0.02*(maxy-miny)), ha='center', annotation_clip=False)
    plt.savefig(get_fn(params), format="pdf", bbox_inches='tight')
    # plt.show()
    # sys.exit(0)


inputfolder = "results"
inputfile_size = "platoon-size-distr.csv"
inputfile_rate = "in-platoon-rate.csv"
fn = join(inputfolder, inputfile_size)
fnrate = join(inputfolder, inputfile_rate)
plt.rcParams['text.usetex'] = True
plt.rc('text.latex', preamble=r'\usepackage{amsmath}')
plt.rc('font', **{'family': 'serif', 'serif': ['Computer Modern']})
plt.rcParams["figure.figsize"] = (16.0/3, 7.0/3)

data = pandas.read_csv(fn)
maxps = data["platoonSize"].max()
rate_data = pandas.read_csv(fnrate)
# tmp = data.loc[data["mps"]==25]
data.groupby(["vr", "mad", "mps", "pr"]).apply(lambda x: discrete_violin_plot(x, rate_data))
