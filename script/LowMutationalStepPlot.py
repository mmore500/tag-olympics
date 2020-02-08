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
        'Hamming Metric' : 'Hamming',
        'Hash Metric' : 'Hash',
        'Asymmetric Wrap Metric' : 'Integer',
        'Symmetric Wrap Metric' : 'Integer (bi)',
        'Approx Dual Streak Metric' : 'Streak',
    }[x['Metric']],
    axis=1
)

df_data['Dimension Count'] = df_data['Dimension']

df_data['Dimension'] = df_data.apply(
    lambda x: x['Dimension Type'] + " " + str(x['Dimension']),
    axis=1
)

df_data['Rank'] = 0

for metric in df_data['Metric'].unique():
    for affinity in df_data['Affinity'].unique():
        which = (
            (df_data['Metric'] == metric)
            & (df_data['Affinity'] == affinity)
        )
        df_data.loc[which, 'Rank'] = df_data[which][
            'Match Distance Change'
        ].rank(ascending=0, method='first')

print("Data crunched!")

cmap = sns.diverging_palette(240, 10, l=65, sep=1, n=1000)

def draw(*args, **kwargs):

    df_data = kwargs.pop('data')
    g = sns.barplot(
        data=df_data,
        x="Match Distance Change",
        y="Rank",
        orient="h",
        ci=None,
        palette=list(map(
            lambda x: 'red' if x < 0 else 'blue' if x > 0 else 'white',
            sorted(df_data["Match Distance Change"], reverse=True)
        )),
    )
    g.set_xticklabels(g.get_xticklabels(), rotation=90)

    g.set(yticks=[])
    g.set_ylabel('')

plt.gcf().set_size_inches(3.75, 2.75)

fg = sns.FacetGrid(
    df_data,
    col='Metric',
    row='Affinity',
    hue='Metric',
    margin_titles=True,
    xlim=(-1.01, 1.01),
)
g = fg.map_dataframe(
    draw
)

g.set_ylabels("")

plt.gcf().set_size_inches(7.5, 4)


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
