import matplotlib
matplotlib.use('Agg')
import pandas as pd
import seaborn as sns
import sys
from matplotlib import pyplot as plt
import seaborn as sns
import pandas as pd
import numpy as np
import itertools
import math
from tqdm import tqdm
from iterpop import iterpop as ip

from keyname import keyname as kn
from fileshash import fileshash as fsh

from pylib import (
    lookup_metric_priority,
)

# open-type fonts
matplotlib.rcParams['pdf.fonttype'] = 42

tqdm.pandas()

filename = sys.argv[1]

df = pd.read_csv(filename)

df = df[df.progress_apply(
    lambda x: (
        x['Update'] != 0
        and math.log2(x['Update']).is_integer()
        or x['Update'] == 1
    ),
    axis=1,
)]

df = df[[
    'Update',
    'Metric',
    'Target Structure',
    'Target Degree',
    'Mutation Rate',
    'seed',
    'Maximum Fitness',
] + (
    ['Target k']
    if 'Target k' in df
    else []
)]

print("Data loaded!")

df['Metric'] = df.progress_apply(
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

df['Target Configuration'] = df.progress_apply(
    lambda x: x['Target Structure'] + " " + str(x['Target Degree']) + (
        ' , k ' + str(x['Target k'])
        if 'Target k' in df
        else ''
    ),
    axis=1,
)

df["Sum Fitness"] = df.groupby([
    'Metric',
    'Target Configuration',
    'Mutation Rate',
]).transform(sum)["Maximum Fitness"]

# collapse down to best across all mutation rates
# adapted from https://stackoverflow.com/a/15705958
idx_bests = df.groupby([
    'Metric',
    'Target Configuration',
])["Sum Fitness"].transform(max) == df['Sum Fitness']

print("Data crunched!")

# PRINT MUT RATES WITH BEST SUM FITNESSES ######################################

for name, group in df[idx_bests].groupby([
    'Metric',
    'Target Configuration',
]):
    print(name, ' mut rate ', ip.pophomogeneous(
        group['Mutation Rate']
    ))
    print(name, ' n obvs ', len(group[group['Update'] == 1]))

# MAX FITNESS BY METRIC, LINE ##################################################
if 'Target k' in df:
    g = sns.FacetGrid(
        df[idx_bests],
        col='Target Configuration',
        margin_titles=True,
    )
else:
    g = sns.FacetGrid(
        df[idx_bests],
        col='Target Degree',
        row='Target Structure',
        row_order=['Regular', 'Irregular'],
        margin_titles=True,
    ).set(ylim=(0, 1))
g.map_dataframe(
    sns.lineplot,
    'Update',
    'Maximum Fitness',
    hue='Metric',
    style='Metric',
    hue_order=sorted(
        df['Metric'].unique(),
        key=lookup_metric_priority,
    ),
    style_order=sorted(df['Metric'].unique()),
).add_legend().set(xscale = 'log')

plt.gcf().set_size_inches(7.5, 6)

outfile = kn.pack({
    'viz' : 'max-fitness-line',
    '_data_hathash_hash' : fsh.FilesHash().hash_files([filename]),
    '_script_fullcat_hash' : fsh.FilesHash(
                                file_parcel="full_parcel",
                                files_join="cat_join"
                            ).hash_files([sys.argv[0]]),
    'ext' : '.pdf'
})
plt.savefig(
    outfile,
    transparent=True,
    bbox_inches='tight',
    pad_inches=0,
)
print("output saved to", outfile)

plt.clf()

# MAX FITNESS BY METRIC, BAR ##################################################
if 'Target k' in df:
    g = sns.FacetGrid(
        df[idx_bests],
        col='Target Configuration',
        margin_titles=True,
    )
else:
    g = sns.FacetGrid(
        df[idx_bests].astype({"Update": int}),
        col='Target Degree',
        row='Target Structure',
        margin_titles=True,
    ).set(ylim=(0, 1))
g.map_dataframe(
    sns.barplot,
    'Update',
    'Maximum Fitness',
    hue='Metric',
    hue_order=sorted(
        df['Metric'].unique(),
        key=lookup_metric_priority,
    ),
    palette=sns.color_palette(),
).add_legend()
g.set_xticklabels(rotation=90)

plt.gcf().set_size_inches(7.5, 6)

outfile = kn.pack({
    'viz' : 'max-fitness-bar',
    '_data_hathash_hash' : fsh.FilesHash().hash_files([filename]),
    '_script_fullcat_hash' : fsh.FilesHash(
                                file_parcel="full_parcel",
                                files_join="cat_join"
                            ).hash_files([sys.argv[0]]),
    'ext' : '.pdf'
})
plt.savefig(
    outfile,
    transparent=True,
    bbox_inches='tight',
    pad_inches=0,
)
print("output saved to", outfile)

plt.clf()

# MAX FITNESS BY MUTATION RATE, LINE ###########################################

g = sns.FacetGrid(
    df,
    row='Metric',
    col='Target Configuration',
    margin_titles=True,
)#.set(ylim=(0, 1))
g.map(
    sns.barplot,
    'Update',
    'Maximum Fitness',
    'Mutation Rate',
    order=sorted(df['Update'].unique()),
).add_legend(title="Mutation\nRate")
g.set_xticklabels(rotation=90)
g.set_titles(col_template='{col_name}')

plt.gcf().set_size_inches(7, 8)
g.fig.get_children()[-1].set_bbox_to_anchor((1.1, 0.5, 0, 0))
plt.tight_layout()

outfile = kn.pack({
    'title' : 'fitness_mutation_barplot',
    '_data_hathash_hash' : fsh.FilesHash().hash_files([filename]),
    '_script_fullcat_hash' : fsh.FilesHash(
                                file_parcel="full_parcel",
                                files_join="cat_join"
                            ).hash_files([sys.argv[0]]),
    'ext' : '.pdf'
})
plt.savefig(
    outfile,
    transparent=True,
    bbox_inches='tight',
    pad_inches=0,
)
print("output saved to", outfile)
