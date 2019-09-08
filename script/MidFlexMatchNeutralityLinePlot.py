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
import itertools

from keyname import keyname as kn
from fileshash import fileshash as fsh

# open-type fonts
matplotlib.rcParams['pdf.fonttype'] = 42

df_key = pd.read_csv(sys.argv[1])

dataframe_filenames = sys.argv[2:]

dfs = [
    (filename, pd.read_csv(filename))
    for filename in dataframe_filenames
]

print("Data loaded!")

res = []
for filename, df in dfs:

    # assumes datafiles are each from one replicate
    for k, v in kn.unpack(filename).items():
        df[k] = v

    res.append(df)

df_data = pd.concat(res)

df_data['Slug'] = df_data['metric-slug']

key = {
    row['Slug'] : {
        col : row[col]
        for col, val in row.iteritems() if col != 'Slug'
    }
    for idx, row in df_key.iterrows()
}

df_data['Metric'] = df_data.apply(
    lambda x: key[x['Slug']]['Metric'],
    axis=1
)

df_data['Dimension'] = df_data.apply(
    lambda x: key[x['Slug']]['Dimension'],
    axis=1
)

df_data['Dimension Type'] = df_data.apply(
    lambda x: key[x['Slug']]['Dimension Type'],
    axis=1
)

df_data['Inverse'] = df_data.apply(
    lambda x: key[x['Slug']]['Inverse'],
    axis=1
)

df_data['Metric'] = df_data.apply(
    lambda x: (
        ('Started ' if 'Started' in x['Metric'] else '')
        + key[x['Slug']]['Base Metric']
    ),
    axis=1
)

df_data['Treatment'] = df_data['treatment']

df_data['Dimension Count'] = df_data['Dimension']

df_data['Dimension'] = df_data.apply(
    lambda x: x['Dimension Type'] + " " + str(x['Dimension']),
    axis=1
)

df_data['Target Size'] = df_data['target-size']

df_data['Target Configuration'] = df_data['target-config']

print("Data crunched!")

df_data['Genetic Distance'] = df_data['Value']

g = sns.FacetGrid(
    df_data[
        (df_data['Measure'] == 'Genetic Distance')
        & (df_data['Statistic'] == 'Median')
    ],
    col='Target Configuration',
    row='Target Size',
    hue='Metric',
    hue_kws={
        'ls' : list(itertools.islice(
            itertools.cycle(['-', '--', '-.', ':']),
            len(df_data['Metric'].unique())
        )),
        'color' : sns.color_palette()
    },
    margin_titles=True
).set(ylim=(0, 1))
g.map(
    sns.lineplot,
    'Step',
    'Genetic Distance',
    style_order=list(df_data['Metric'].unique())
).add_legend()

assert len({kn.unpack(f)['experiment'] for f in dataframe_filenames}) == 1
assert len({kn.unpack(f)['bitweight'] for f in dataframe_filenames}) == 1
assert len({kn.unpack(f)['fit-fun'] for f in dataframe_filenames}) == 1

outfile = kn.pack({
    'bitweight' : kn.unpack(dataframe_filenames[0])['bitweight'],
    'fit-fun' : kn.unpack(dataframe_filenames[0])['fit-fun'],
    'viz' : 'neutrality-ancestry-line',
    '_data_hathash_hash' : fsh.FilesHash().hash_files(dataframe_filenames),
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

g = sns.FacetGrid(
    data=df_data[
        (df_data['Statistic'] == 'Median')
    ].pivot_table(
        index=['Step', 'Metric', 'seed', 'Target Configuration', 'Target Size'],
        columns='Measure',
        values='Value',
        aggfunc='first'
    ).reset_index(),
    col='Target Configuration',
    row='Target Size',
    hue='Metric',
    hue_kws={
        'ls' : list(itertools.islice(
            itertools.cycle(['-', '--', '-.', ':']),
            len(df_data['Metric'].unique())
        )),
        'color' : sns.color_palette()
    },
    margin_titles=True
).set(ylim=(0, 1))
g.map(
    sns.lineplot,
    'Updates Elapsed',
    'Genetic Distance',
    style_order=list(df_data['Metric'].unique())
).add_legend()

assert len({kn.unpack(f)['experiment'] for f in dataframe_filenames}) == 1
assert len({kn.unpack(f)['bitweight'] for f in dataframe_filenames}) == 1
assert len({kn.unpack(f)['fit-fun'] for f in dataframe_filenames}) == 1

outfile = kn.pack({
    'experiment' : kn.unpack(dataframe_filenames[0])['experiment'],
    'bitweight' : kn.unpack(dataframe_filenames[0])['bitweight'],
    'fit-fun' : kn.unpack(dataframe_filenames[0])['fit-fun'],
    'viz' : 'distance-update',
    '_data_hathash_hash' : fsh.FilesHash().hash_files(dataframe_filenames),
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
