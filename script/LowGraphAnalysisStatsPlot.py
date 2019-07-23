import matplotlib
# matplotlib.use('Agg')
import pandas as pd
import seaborn as sns
import sys
from matplotlib import pyplot as plt
import seaborn as sns
import pandas as pd
import numpy as np
from slugify import slugify

from keyname import keyname as kn
from fileshash import fileshash as fsh

# open-type fonts
matplotlib.rcParams['pdf.fonttype'] = 42

dataframe_filename = sys.argv[2]

df_key = pd.read_csv(sys.argv[1])

df_data = pd.read_csv(dataframe_filename)

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

df_data['Type'] = df_data.apply(
    lambda x: (
        ("Inverse " if x['Inverse'] else "Direct") + " / " + x['Dimension Type']
    ),
    axis=1
)

df_data['Metric'] = df_data.apply(
    lambda x: (
        ('Sliding ' if key[x['Metric']]['Sliding'] else '')
        + key[x['Metric']]['Base Metric']
    ),
    axis=1
)

for col in (
    col for col in list(df_data)
    if col not in [
        "Weight", "Metric", "Sample", "Dimension",
        "Inverse", "Dimension Type", "Type"
    ]):

    g = sns.FacetGrid(
        df_data,
        col='Metric',
        row='Type',
        hue='Inverse',
        margin_titles=True
    ).set(xlim=(-1, 2))
    g.map(sns.barplot, "Dimension", col)

    plt.savefig(
        kn.pack({
            'title' : kn.unpack(dataframe_filename)['title'],
            'bitweight' : kn.unpack(dataframe_filename)['bitweight'],
            'seed' : kn.unpack(dataframe_filename)['seed'],
            'stat' : slugify(col),
            '_data_hathash_hash' : fsh.FilesHash().hash_files([dataframe_filename]),
            '_script_fullcat_hash' : fsh.FilesHash(
                                        file_parcel="full_parcel",
                                        files_join="cat_join"
                                    ).hash_files([sys.argv[0]]),
            # '_source_hash' :kn.unpack(dataframe_filename)['_source_hash'],
            'ext' : '.pdf'
        }),
        transparent=True,
        bbox_inches='tight',
        pad_inches=0
    )
