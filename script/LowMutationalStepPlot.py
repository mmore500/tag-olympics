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

    df_data = kwargs.pop('data')
    g = sns.barplot(
        data=df_data,
        x="Match Distance Change",
        y="Normalized Rank",
        orient="h",
        ci=None,
        palette=list(map(
            lambda x: 'red' if x > 0 else 'blue' if x < 0 else 'white',
            sorted(df_data["Match Distance Change"])
        )),
        zorder=100,
    )
    # adapted from https://stackoverflow.com/a/32289054
    plt.setp(g.lines, zorder=100)
    plt.setp(g.collections, zorder=100, label="")

    plt.axvline(x=0, color='black', linewidth=0.8, zorder=1)
    g.set_xticklabels(g.get_xticklabels(), fontdict={'fontsize':8})

    g.set(yticks=[])
    g.set_ylabel('')
    g.spines['left'].set_zorder(-10000)
    g.spines['left'].set_color('lightgray')

    yticks=list(range(0,101,10))
    g.yaxis.set_major_locator(plt.LinearLocator(numticks=len(yticks)))
    g.set_yticklabels(reversed(yticks))

    g.xaxis.set_minor_locator(plt.LinearLocator(numticks=5))

    g.grid(which='major', axis='both', linestyle='-', linewidth=0.5)
    g.grid(which='minor', axis='both', linestyle=':', linewidth=0.5)
    g.set_axisbelow(True)

    vals = sorted(df_data["Match Distance Change"])
    metric, = df_data["Metric"].unique()
    affinity, = df_data["Affinity"].unique()
    print(metric, affinity, "num exactly neutral", sum(v == 0 for v in vals), "/", len(vals))
    plt.hlines(
        [x for x in [
            min(
                [idx for idx, val in enumerate(vals) if val == 0],
                default=None,
            ),
            max(
                [idx for idx, val in enumerate(vals) if val == 0],
                default=None,
            ),
        ] if x is not None],
        xmin=-1,
        xmax=1,
        linestyles='dashed',
        color='black',
    )
    plt.hlines(
        np.mean([
            min(idx for idx, val in enumerate(vals) if val > 0),
            max(idx for idx, val in enumerate(vals) if val < 0),
        ]),
        xmin=-1,
        xmax=1,
        color='black',
    )

fg = sns.FacetGrid(
    df_data,
    col='Metric',
    row='Affinity',
    col_order=sorted(df_data["Metric"].unique()),
    margin_titles=True,
    xlim=(1.01, -1.01),
    ylim=(0, 1),
)
g = fg.map_dataframe(
    draw
)

g.set_ylabels('Percentile')

g.fig.text(0.3, 0.1, s='Match Distance Change', fontdict={'fontsize':10})
g.fig.subplots_adjust(bottom=0.17, wspace=0.3)

for ax, title in zip(g.axes.flat, sorted(df_data["Metric"].unique())):
    ax.set_title(title, fontsize=10)

plt.gcf().set_size_inches(3.75, 4.75)


outfile = kn.pack({
    'title' : kn.unpack(dataframe_filename)['title'],
    'bitweight' : kn.unpack(dataframe_filename)['bitweight'],
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
