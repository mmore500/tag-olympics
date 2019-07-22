import matplotlib
matplotlib.use('Agg')
import pandas as pd
import seaborn as sns
import sys
from matplotlib import pyplot as plt
import seaborn as sns
import pandas as pd
import numpy as np

# open-type fonts
matplotlib.rcParams['pdf.fonttype'] = 42

df_key = pd.read_csv(sys.argv[1])

df_data = pd.read_csv(sys.argv[2])

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

df_data['Detour Difference'] = df_data.apply(
    lambda x: x['Detour Difference'] + np.random.normal(0, 1e-8)
)

g = sns.FacetGrid(
    df_data[(df_data['Dimension Type'] == 'Minimum') & df_data['Inverse']],
    col='Metric',
    row='Dimension',
    margin_titles=True
).set(xlim=(-1, 2))
g.map(sns.distplot, "Detour Difference", hist=False, rug=True)

plt.savefig(
    "inverse-minimum-low-triplet-plot.pdf",
    transparent=True
)

g = sns.FacetGrid(
    df_data[(df_data['Dimension Type'] == 'Mean') & df_data['Inverse']],
    col='Metric',
    row='Dimension',
    margin_titles=True
).set(xlim=(-1, 2))
g.map(sns.distplot, "Detour Difference", hist=False, rug=True)

plt.savefig(
    "inverse-mean-low-triplet-plot.pdf",
    transparent=True
)


g = sns.FacetGrid(
    df_data[(df_data['Dimension Type'] == 'Minimum') & ~df_data['Inverse']],
    col='Metric',
    row='Dimension',
    margin_titles=True
).set(xlim=(-1, 2))
g.map(sns.distplot, "Detour Difference", hist=False, rug=True)

plt.savefig(
    "minimum-low-triplet-plot.pdf",
    transparent=True
)

g = sns.FacetGrid(
    df_data[(df_data['Dimension Type'] == 'Mean') & ~df_data['Inverse']],
    col='Metric',
    row='Dimension',
    margin_titles=True
).set(xlim=(-1, 2))
g.map(sns.distplot, "Detour Difference", hist=False, rug=True)

plt.savefig(
    "mean-low-triplet-plot.pdf",
    transparent=True
)
