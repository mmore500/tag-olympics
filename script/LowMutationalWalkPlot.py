import matplotlib
matplotlib.use('Agg')
import pandas as pd
import seaborn as sns
import sys
from matplotlib import pyplot as plt
from matplotlib import ticker as mpl_ticker
import seaborn as sns
import pandas as pd
import numpy as np
from keyname import keyname as kn
from fileshash import fileshash as fsh
import math

from pylib import (
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

df_data['Metric'] = df_data.apply(
    lambda x: (
        ('Sliding ' if key[x['Metric']]['Sliding'] else '')
        + key[x['Metric']]['Base Metric']
    ),
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

print("data crunched!")

g = sns.FacetGrid(
    df_data,
    col='Metric',
    hue='Metric',
    hue_order=sorted(df_data["Metric"].unique()),
    col_order=sorted(df_data["Metric"].unique()),
    margin_titles=True,
).set(ylim=(0, 1))
g.map(sns.lineplot, "Mutational Step",  "Match Distance", ci="sd")
g.set_titles("{col_name}")

for ax, title in zip(g.axes.flat, sorted(df_data["Metric"].unique())):
    ax.set_title(title, fontsize=10)

plt.gcf().set_size_inches(7.5, 1.5)

outfile = kn.pack({
    'title' : 'mutational_walk_lineplot',
    'bitweight' : kn.unpack(dataframe_filename)['bitweight'],
    'seed' : kn.unpack(dataframe_filename)['seed'],
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

plt.clf()
plt.close(plt.gcf())

g = sns.barplot(
    data=df_data[df_data.apply(
        lambda x: x['Mutational Step'] == 0 or math.log2(x['Mutational Step']).is_integer(),
        axis=1
    )],
    x='Mutational Step',
    y='Match Distance',
    hue='Metric',
    hue_order=sorted(
        df_data["Metric"].unique(),
        key=lookup_metric_priority,
    ),
).set(ylim=(0, 1))

plt.gcf().set_size_inches(3.75, 3.75)

outfile = kn.pack({
    'title' : 'mutational_walk_barplot',
    'bitweight' : kn.unpack(dataframe_filename)['bitweight'],
    'seed' : kn.unpack(dataframe_filename)['seed'],
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

plt.clf()
plt.close(plt.gcf())

fil = df_data.apply(
    lambda x: x['Mutational Step'] == 0 or math.log2(x['Mutational Step']).is_integer(),
    axis=1
)
g = sns.lineplot(
    data=df_data[fil],
    x='Mutational Step',
    y='Match Distance',
    markers=True,
    hue='Metric',
    style='Metric',
    hue_order=sorted(
        df_data["Metric"].unique(),
        key=lookup_metric_priority,
    ),
)

g.set(ylim=(0, 1))
g.set_xscale('symlog', base=2)
g.set_xticks(sorted(
    df_data[fil]['Mutational Step'].unique()
))
g.xaxis.set_major_formatter(mpl_ticker.FormatStrFormatter("%d"))


plt.gcf().set_size_inches(3.75, 3.75)

outfile = kn.pack({
    'title' : 'mutational_walk_lineplot_ci',
    'bitweight' : kn.unpack(dataframe_filename)['bitweight'],
    'seed' : kn.unpack(dataframe_filename)['seed'],
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

g = sns.FacetGrid(
    # need to compensate for custom heatmap binning below
    data=pd.concat([
        df_data,
        df_data[df_data["Mutational Step"] <= 2],
    ]),
    # data=df_data.groupby(['bin', "Metric", "Sample", "Replicate"]).mean().reset_index(),
    col='Metric',
    hue='Metric',
    col_wrap=3,
    hue_order=sorted(
        df_data["Metric"].unique(),
        key=lookup_metric_priority,
    ),
    col_order=sorted(
        df_data["Metric"].unique(),
        key=lookup_metric_priority,
    ),
    margin_titles=True,
).set(ylim=(0, 1), xlim=df_data['Mutational Step'].agg(['min', 'max']))
g.map(
    sns.histplot,
    "Mutational Step",
    "Match Distance",
    bins=([-0.5, 0.5, 1.5] + [2*x + 2.5 for x in range(32)], 10),
)
g.set_titles("{col_name}")

for ax, title in zip(g.axes.flat, sorted(
    df_data["Metric"].unique(),
    key=lookup_metric_priority,
)):
    ax.set_xlim(left=-0.5)
    ax.set_title(title, fontsize=10)
    ax.set_xscale('symlog', base=2)
    ax.set_xticks(sorted(
        df_data[fil]['Mutational Step'].unique()
    ))
    ax.xaxis.set_major_formatter(mpl_ticker.FormatStrFormatter("%d"))

plt.gcf().set_size_inches(7.5, 3)
plt.tight_layout()
outfile = kn.pack({
    'title' : 'mutational_walk_heatplot',
    'bitweight' : kn.unpack(dataframe_filename)['bitweight'],
    'seed' : kn.unpack(dataframe_filename)['seed'],
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
