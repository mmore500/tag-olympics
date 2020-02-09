import matplotlib
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
import math

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

df_data['Metric'] = df_data.apply(
    lambda x: {
        'Hamming Metric' : 'Hamming',
        'Hash Metric' : 'Hash',
        'Asymmetric Wrap Metric' : 'Integer',
        'Symmetric Wrap Metric' : 'Integer (bi)',
        'Approx Dual Streak Metric' : 'Streak',
    }[x['Metric']],
    axis=1
)

df_data['Rank'] = 0

for metric in df_data['Metric'].unique():
    df_data.loc[df_data['Metric'] == metric, 'Rank'] = (
        df_data[df_data['Metric'] == metric][
            'Match Distance'
        ].rank(ascending=0, method='first')
    )


print("data crunched!")

g = sns.barplot(
    data=df_data,
    x='Metric',
    y='Match Distance',
    order=sorted(['Hamming', 'Hash', 'Integer', 'Streak', 'Integer (bi)']),
)
g.set(ylim=(0, 1))
g.set_xticklabels(g.get_xticklabels(), rotation=90)

plt.gcf().set_size_inches(3.75, 2.75)

outfile = kn.pack({
    'title' : 'dimensionality_barplot',
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

def draw(*args, **kwargs):

    df_data = kwargs.pop('data')

    g = sns.barplot(
        data=df_data,
        x="Match Distance",
        y="Rank",
        orient="h",
        ci=None,
        palette=list(map(
            lambda x: 'red' if x < 0 else 'blue' if x > 0 else 'white',
            sorted(df_data["Match Distance"], reverse=True)
        )),
    )

    g.set(yticks=[])
    g.set_ylabel('')

fg = sns.FacetGrid(
    df_data,
    col='Metric',
    col_order=sorted(df_data["Metric"].unique()),
    margin_titles=True,
)
g = fg.map_dataframe(
    draw
)

g.set_ylabels("")

g.fig.text(0.37, 0.1, s='Match Distance', fontdict={'fontsize':10})
g.fig.subplots_adjust(bottom=0.225, wspace=0.3)

for ax, title in zip(g.axes.flat, sorted(df_data["Metric"].unique())):
    ax.set_title(title, fontsize=10)
    ax.set_xlim(xmin=0.0, xmax=1)
    ax.set_xticks([0, 1])


plt.gcf().set_size_inches(3.75, 2.75)

outfile = kn.pack({
    'title' : 'dimensionality_distnplot',
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
