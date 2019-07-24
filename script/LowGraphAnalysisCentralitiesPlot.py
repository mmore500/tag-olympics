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

df_data['Centrality'] = df_data.apply(
    lambda x: x['Centrality'] + np.random.normal(0, 1e-8),
    axis=1
)

df_data['Dimension Count'] = df_data['Dimension']

df_data['Dimension'] = df_data.apply(
    lambda x: x['Dimension Type'] + " " + str(x['Dimension']),
    axis=1
)
print("Data crunched!")

g = sns.FacetGrid(
    df_data,
    col='Metric',
    row='Dimension',
    hue='Dimension Count',
    margin_titles=True,
    sharey=False,
    col_order=(
        sorted(
            [x for x in df_data['Metric'].unique() if 'Inverse' in x]
        ) + sorted(
            [x for x in df_data['Metric'].unique() if 'Inverse' not in x],
        )
    ),
    row_order=(
        sorted(
            [x for x in df_data['Dimension'].unique() if 'Mean' in x],
            key=lambda str: next(int(s) for s in str.split() if s.isdigit())
        ) + sorted(
            [x for x in df_data['Dimension'].unique() if 'Minimum' in x],
            key=lambda str: next(int(s) for s in str.split() if s.isdigit())
        )
    )
)
g.map(sns.distplot, "Centrality", hist=False, rug=True)

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
