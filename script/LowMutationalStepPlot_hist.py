import matplotlib
from matplotlib import ticker as mpl_ticker
matplotlib.use('Agg')
import pandas as pd
import seaborn as sns
import sys
from matplotlib import pyplot as plt
import seaborn as sns
import pandas as pd
import numpy as np
from keyname import keyname as kn
from fileshash import fileshash as fsh

from pylib import (
    calc_propci_wilson_cc,
    lookup_metric_color,
    lookup_metric_priority,
)

# open-type fonts
matplotlib.rcParams['pdf.fonttype'] = 42

dataframe_filename = sys.argv[2]

df_key = pd.read_csv(sys.argv[1])

df_data = pd.read_csv(dataframe_filename)

print("Data loaded!")

key = {
    row['Metric'] : {
        col : row[col]
        for col, val in row.iteritems() if col != 'Metric'
    }
    for idx, row in df_key.iterrows()
}

df_data['Dimension'] = df_data.apply(
    lambda x: key[x['Metric']]['Dimension'],
    axis=1
)

df_data['Dimension Type'] = df_data.apply(
    lambda x: key[x['Metric']]['Dimension Type'],
    axis=1
)

df_data['Inverse'] = df_data.apply(
    lambda x: key[x['Metric']]['Inverse'],
    axis=1
)

df_data['Metric'] = df_data.apply(
    lambda x: (
        ('Sliding ' if key[x['Metric']]['Sliding'] else '')
        + key[x['Metric']]['Base Metric']
    ),
    axis=1
)

df_data['Metric'] = df_data.apply(
    lambda x: (
        ('Inverse ' if x['Inverse'] else '')
        + x['Metric']
    ),
    axis=1
)

df_data['Metric'] = df_data.apply(
    lambda x: {
        'Codon Metric' : 'Codon',
        'Hamming Metric' : 'Hamming',
        'Hamming-Streak Metric' : 'Hamming-Streak',
        'Hash Metric' : 'Hash',
        'Asymmetric Wrap Metric' : 'Integer',
        'Symmetric Wrap Metric' : 'Integer (bi)',
        'Approx Dual Streak Metric' : 'Streak',
        'Approx Single Streak Metric' : 'Simple Streak',
        'Sliding Approx Single Streak Metric' : 'Sli-Sim-Stk',
    }[x['Metric']],
    axis=1
)

df_data['Dimension Count'] = df_data['Dimension']

df_data['Match Closeness Change'] = -df_data['Match Distance Change']

df_data['Dimension'] = df_data.apply(
    lambda x: x['Dimension Type'] + " " + str(x['Dimension']),
    axis=1
)

df_data['Rank'] = 0
df_data['Normalized Rank'] = 0

for metric in df_data['Metric'].unique():
    for affinity in df_data['Affinity'].unique():
        which = (
            (df_data['Metric'] == metric)
            & (df_data['Affinity'] == affinity)
        )
        df_data.loc[which, 'Rank'] = df_data[which][
            'Match Distance Change'
        ].rank(ascending=1, method='first')

        df_data.loc[which, 'Normalized Rank'] = df_data[which][
            'Rank'
        ] / df_data[which][
            'Rank'
        ].max()

print("Data crunched!")

def draw(*args, **kwargs):

    bins = np.linspace(-1,1,12) # yields 11 bins

    df_data = kwargs.pop('data')

    x, binedges = np.histogram(
        df_data["Match Closeness Change"],
        bins=bins,
    )
    bin_centers = 0.5*(bins[1:] + bins[:-1])

    ax = plt.gca()

    cis_p = [
            calc_propci_wilson_cc(
                x_,
                len(df_data),
            ) for x_ in x
    ]
    cis_c = [
        tuple(
            v * len(df_data)
            for v in ci
        )
        for ci in cis_p
    ]
    cis_e = [
        tuple(
            abs(v - m)
            for v in ci
        )
        for ci, m in zip(cis_c, x)
    ]
    metric, = df_data["Metric"].unique()

    ax = plt.gca()
    ax.bar(
        x=bin_centers,
        height=x,
        width=2/len(x),
        error_kw={'lw': 0.5},
        color=lookup_metric_color(metric),
        capsize=1,
        # linefmt='C0-',
        # basefmt='none',
        # markerfmt='none',
        yerr=np.swapaxes(cis_e, 0, 1),
        zorder=2,
    )
    ax.xaxis.set_major_locator(plt.MultipleLocator(1))
    ax.xaxis.set_minor_locator(mpl_ticker.AutoMinorLocator(2))

    ax.yaxis.set_major_locator(plt.MaxNLocator(8))
    ax.yaxis.set_minor_locator(mpl_ticker.AutoMinorLocator(2))

    ax.grid(which='major', axis='both', linestyle='-', linewidth=0.5)
    ax.grid(which='minor', axis='both', linestyle=':', linewidth=0.5)
    ax.set_axisbelow(True)
    ax.axvline(0, color="black", lw=0.8, zorder=1)

    ax.spines['left'].set_visible(False)

    return ax

fg = sns.FacetGrid(
    df_data,
    col='Metric',
    row='Affinity',
    col_order=sorted(
        df_data["Metric"].unique(),
        key=lookup_metric_priority,
    ),
    margin_titles=True,
)
g = fg.map_dataframe(
    draw
)

g.set_ylabels('Count')

g.fig.text(0.2, 0.1, s='Match Closeness Change', fontdict={'fontsize':10})
g.fig.subplots_adjust(bottom=0.17, wspace=0.3)

for ax, title in zip(g.axes.flat, sorted(
    df_data["Metric"].unique(),
    key=lookup_metric_priority,
)):
    ax.set_title(title, fontsize=10)

plt.gcf().set_size_inches(3.75, 4.75)


outfile = kn.pack({
    'title' : kn.unpack(dataframe_filename)['title'],
    'bitweight' : kn.unpack(dataframe_filename)['bitweight'],
    'viz' : 'hist',
    'seed' : kn.unpack(dataframe_filename)['seed'],
    '_data_hathash_hash' : fsh.FilesHash().hash_files([dataframe_filename]),
    '_script_fullcat_hash' : fsh.FilesHash(
                                file_parcel="full_parcel",
                                files_join="cat_join"
                            ).hash_files([sys.argv[0]]),
    # '_source_hash' :kn.unpack(dataframe_filename)['_source_hash'],
    'ext' : '.pdf',
})
plt.savefig(
    outfile,
    transparent=True,
    bbox_inches='tight',
    pad_inches=0
)
print("output saved to", outfile)
