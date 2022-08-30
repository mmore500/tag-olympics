from pyqt_fit import kde, kde_methods
from  iterpop import iterpop as ip
import itertools as it
import matplotlib
matplotlib.use('Agg')
import pandas as pd
import seaborn as sns
import sys
from matplotlib import pyplot as plt
from matplotlib import collections as mpl_collections
from matplotlib import patches as mpl_patches
from matplotlib import ticker as mpl_ticker
import seaborn as sns
import pandas as pd
import numpy as np
from keyname import keyname as kn
from fileshash import fileshash as fsh
import more_itertools as mit

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

print("data loaded!")

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

df_data['Dimension'] = df_data.apply(
    lambda x:  x['Dimension Type'] + " " + str(x['Dimension']),
    axis=1
)

df_data['Uniformified'] = df_data.apply(
    lambda x: key[x['Metric']]['Uniformified'],
    axis=1
)

df_data['Metric'] = df_data.apply(
    lambda x: (
        ('Sliding ' if key[x['Metric']]['Sliding'] else '')
        + key[x['Metric']]['Base Metric']
    ),
    axis=1
)

df_data['Match Score'] = df_data.apply(
    lambda x: x['Match Score'] + np.random.normal(0, 1e-8),
    axis=1
)

df_data['Match Distance'] = df_data['Match Score']

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

df_data['Rank'] = 0

for metric in df_data['Metric'].unique():
    for affinity in df_data['Uniformified'].unique():
        which = (
            (df_data['Metric'] == metric)
            & (df_data['Uniformified'] == affinity)
        )
        df_data.loc[which, 'Rank'] = df_data[which][
            'Match Distance'
        ].rank(ascending=1, method='first')

df_data['Uniformified'] = df_data.apply(
    lambda x: 'Uniformified' if x['Uniformified'] else 'Raw',
    axis=1
)

print("data crunched!")

def draw_bar(*args, **kwargs):

    bins = np.linspace(0,1,66) # yields 65 bins

    df_data = kwargs.pop('data')

    x, binedges = np.histogram(
        df_data["Match Distance"],
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
        width=1/len(x),
        error_kw={'lw': 0.5},
        color=lookup_metric_color(metric),
        # linefmt='C0-',
        # basefmt='none',
        # markerfmt='none',
        yerr=np.swapaxes(cis_e, 0, 1),
    )

    return ax

fg = sns.FacetGrid(
    df_data,
    row='Metric',
    col='Uniformified',
    row_order=sorted(
        df_data["Metric"].unique(),
        key=lookup_metric_priority,
    ),
    col_order=sorted(['Raw', 'Uniformified']),
    margin_titles=True,
    # xlim=(0, 1.01),
)
g = fg.map_dataframe(
    draw_bar
)

g.set_ylabels('Count')
g.set_xlabels('Match Distance')

# g.fig.text(0.36, 0.1, s='Match Distance', fontdict={'fontsize':10})
# g.fig.subplots_adjust(bottom=0.17, wspace=0.3)

for ax, title in zip(g.axes.flat, ['Raw', 'Uniformified']):
    ax.set_title(title, fontsize=10)

# adapted from https://cduvallet.github.io/posts/2018/11/facetgrid-ylabel-access
# Iterate thorugh each axis
for ax in g.axes.flat:

    # Make right ylabel more human-readable and larger
    # Only the 2nd and 4th axes have something in ax.texts
    if ax.texts:
        # This contains the right ylabel text
        ax.texts[0].set_text(
            ax.texts[0].get_text().split('=')[1]
        )

    ax.set_xticks([0, 0.5, 1])
    ax.set_xticklabels(['0', '0.5', '1'])
    ax.xaxis.set_minor_locator(plt.LinearLocator(numticks=5))

    ax.yaxis.set_major_locator(plt.MaxNLocator(4))
    ax.yaxis.set_minor_locator(mpl_ticker.AutoMinorLocator(2))


    ax.grid(which='major', axis='both', linestyle='-', linewidth=0.5)
    ax.grid(which='minor', axis='both', linestyle=':', linewidth=0.5)
    ax.set_axisbelow(True)


plt.gcf().set_size_inches(4.75, 3.75)

outfile = kn.pack({
    'title' : kn.unpack(dataframe_filename)['title'],
    'bitweight' : kn.unpack(dataframe_filename)['bitweight'],
    'seed' : kn.unpack(dataframe_filename)['seed'],
    'viz' : 'hist',
    '_data_hathash_hash' : fsh.FilesHash().hash_files([dataframe_filename]),
    '_script_fullcat_hash' : fsh.FilesHash(
                                file_parcel="full_parcel",
                                files_join="cat_join"
                            ).hash_files([sys.argv[0]]),
    # '_source_hash' :kn.unpack(dataframe_filename)['_source_hash'],
    'ext' : '.pdf'
})
plt.savefig(
    outfile,
    transparent=True,
    bbox_inches='tight',
    pad_inches=0
)
print("output saved to", outfile)
