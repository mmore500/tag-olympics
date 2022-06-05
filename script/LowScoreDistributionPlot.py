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

def draw(*args, **kwargs):

    df_data = kwargs.pop('data')
    g = sns.barplot(
        data=df_data,
        x="Match Score",
        y="Rank",
        orient="h",
        ci=None,
        palette=list(map(
            lambda x: 'blue',
            sorted(df_data["Match Score"])
        )),
    )
    g.set_xticklabels([0, 1], fontdict={'fontsize':8})

    g.set(yticks=[])
    g.set_ylabel('')
    g.spines['left'].set_position('zero')
    g.spines['left'].set_zorder(-10000)
    g.spines['left'].set_color('black')

    plt.plot([1, 0], [5000, 0], 'k--', lw=0.5)


fg = sns.FacetGrid(
    df_data,
    col='Metric',
    row='Uniformified',
    col_order=sorted(df_data["Metric"].unique()),
    row_order=sorted(['Raw', 'Uniformified']),
    margin_titles=True,
    xlim=(0, 1.01),
)
g = fg.map_dataframe(
    draw
)

g.set_ylabels('Percentile')

g.fig.text(0.36, 0.1, s='Match Distance', fontdict={'fontsize':10})
g.fig.subplots_adjust(bottom=0.17, wspace=0.3)

for ax, title in zip(g.axes.flat, sorted(df_data["Metric"].unique())):
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

    yticks=list(range(0,101,10))
    ax.set_ylim(0, 5000)
    ax.yaxis.set_major_locator(plt.LinearLocator(numticks=len(yticks)))
    ax.set_yticklabels(yticks)

    ax.xaxis.set_minor_locator(plt.LinearLocator(numticks=5))

    ax.grid(which='major', axis='both', linestyle='-', linewidth=0.5)
    ax.grid(which='minor', axis='both', linestyle=':', linewidth=0.5)
    ax.set_axisbelow(True)


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
    'ext' : '.pdf'
})
plt.savefig(
    outfile,
    transparent=True,
    bbox_inches='tight',
    pad_inches=0
)
print("output saved to", outfile)
