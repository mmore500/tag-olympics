import matplotlib
matplotlib.use('Agg')
import pandas as pd
import seaborn as sns
import sys
from matplotlib import pyplot as plt
import seaborn as sns
import pandas as pd
import os
import re

import numpy as np
from keyname import keyname as kn
import itertools
from tqdm import tqdm

from keyname import keyname as kn
from fileshash import fileshash as fsh

import glob

tqdm.pandas()

# open-type fonts
matplotlib.rcParams['pdf.fonttype'] = 42

df_key = pd.read_csv(sys.argv[1])

filenames = glob.glob(sys.argv[2])
assert len(sys.argv) == 3

dfs = [
    (filename, pd.read_csv(filename))
    for filename in tqdm(filenames)
]

print("Data loaded!")

pattern = re.compile(r"^set MO_MUT_NORMAL_SD (\d*\.?\d*)", re.MULTILINE)
res = []
for filename, df in tqdm(dfs):
    dir = os.path.dirname(filename)
    log = f"{dir}/title=run+rep=0+ext=.log"

    with open(log) as f:
        filetext = f.read()
        match, = re.finditer(pattern, filetext)
        mut_rate = float(match.groups()[0])

    # assumes datafiles are each from one replicate
    for k, v in kn.unpack(filename).items():
        df[k] = v

    df['mut'] = mut_rate

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

df_data['Metric'] = df_data.progress_apply(
    lambda x: key[x['Slug']]['Metric'],
    axis=1
)

df_data['Dimension'] = df_data.progress_apply(
    lambda x: key[x['Slug']]['Dimension'],
    axis=1
)

df_data['Dimension Type'] = df_data.progress_apply(
    lambda x: key[x['Slug']]['Dimension Type'],
    axis=1
)

df_data['Inverse'] = df_data.progress_apply(
    lambda x: key[x['Slug']]['Inverse'],
    axis=1
)

df_data['Metric'] = df_data.progress_apply(
    lambda x: (
        ('Started ' if 'Started' in x['Metric'] else '')
        + ('Sliding ' if 'Sliding' in x['Metric'] else '')
        + key[x['Slug']]['Base Metric']
    ),
    axis=1
)

df_data['Treatment'] = df_data['treatment']

df_data['Update'] = df_data['update']

df_data['Maximum Fitness'] = df_data['max_fitness']

df_data['Mean Fitness'] = df_data['mean_fitness']

df_data['Dimension Count'] = df_data['Dimension']

df_data['Dimension'] = df_data.progress_apply(
    lambda x: x['Dimension Type'] + " " + str(x['Dimension']),
    axis=1
)

df_data['Target Size'] = df_data['target-size']

df_data['Target Degree'] = df_data['target-degree']

df_data['Target Structure'] = df_data['target-structure']

if 'target-k' in df_data:
    df_data['Target k'] = df_data['target-k']

df_data['Mutation Rate'] = df_data['mut']

print("Data crunched!")

outfile = kn.pack({
    'title' : 'masterfitness',
    '_data_hathash_hash' : fsh.FilesHash().hash_files(filenames),
    '_script_fullcat_hash' : fsh.FilesHash(
                                file_parcel="full_parcel",
                                files_join="cat_join"
                            ).hash_files([sys.argv[0]]),
    'ext' : '.csv'
})

df_data.to_csv(outfile, index=False)

print('Output saved to', outfile)
