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

print("MEAN")
for metric in df_data['Metric'].unique():
    print(
        metric,
        df_data[df_data['Metric'] == metric]["Match Distance"].mean()
    )

print("MIN")
for metric in df_data['Metric'].unique():
    print(
        metric,
        df_data[df_data['Metric'] == metric]["Match Distance"].min()
    )

print("MAX")
for metric in df_data['Metric'].unique():
    print(
        metric,
        df_data[df_data['Metric'] == metric]["Match Distance"].max()
    )
