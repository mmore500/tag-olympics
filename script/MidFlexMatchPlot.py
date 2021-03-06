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

from keyname import keyname as kn
from fileshash import fileshash as fsh

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
]]

print("Data loaded!")

df['Metric'] = df.progress_apply(
    lambda x: {
        'Hamming Metric' : 'Hamming',
        'Hash Metric' : 'Hash',
        'Asymmetric Wrap Metric' : 'Integer',
        'Symmetric Wrap Metric' : 'Integer (bi)',
        'Approx Dual Streak Metric' : 'Streak',
    }[x['Metric']],
    axis=1
)

df['Target Configuration'] = df.progress_apply(
    lambda x: x['Target Structure'] + " " + str(x['Target Degree']),
    axis=1,
)

df["Sum Fitness"] = df.groupby([
    'Metric',
    'Target Structure',
    'Target Degree',
    'Mutation Rate',
]).transform(sum)["Maximum Fitness"]

# collapse down to best across all mutation rates
# adapted from https://stackoverflow.com/a/15705958
idx_bests = df.groupby([
    'Metric',
    'Target Structure',
    'Target Degree',
])["Sum Fitness"].transform(max) == df['Sum Fitness']

print("Data crunched!")

# MAX FITNESS BY METRIC, LINE ##################################################

g = sns.FacetGrid(
    df[idx_bests],
    col='Target Degree',
    row='Target Structure',
    margin_titles=True,
).set(ylim=(0, 1))
g.map_dataframe(
    sns.lineplot,
    'Update',
    'Maximum Fitness',
    hue='Metric',
    style='Metric',
    hue_order=sorted(df['Metric'].unique()),
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

# MAX FITNESS BY MUTATION RATE, LINE ###########################################

g = sns.FacetGrid(
    df,
    row='Metric',
    col='Target Configuration',
    margin_titles=True,
).set(ylim=(0, 1))
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
plt.tight_layout()
g.fig.get_children()[-1].set_bbox_to_anchor((1.1, 0.5, 0, 0))

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
