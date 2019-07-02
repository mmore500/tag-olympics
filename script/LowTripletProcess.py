import pandas as pd
import networkx as nx
import sys
from tqdm import tqdm
import numpy as np
import itertools
import random

in_df = pd.read_csv(sys.argv[1])
out = list()

n_max = in_df["From"].max()

for vals, df in tqdm(in_df.groupby(["Metric", "Sample"])):

    lookup = dict()

    for idx, row in df.iterrows():
        lookup[frozenset({row["From"], row["To"]})] = row["Match Distance"]

    res = list()
    for reps in range(1000):
        x,y,z = random.sample(range(n_max),3)

        xy = lookup[frozenset({x,y})]
        yz = lookup[frozenset({y,z})]

        xz = lookup[frozenset({x,z})]

        if not xy or not yz or not xz:
            continue

        out.append({
            "Metric" : vals[0],
            "Sample" : vals[1]*1000+reps,
            "Triangle Ratio" : xy+yz-xz
        })


pd.DataFrame.from_records(out).to_csv("triplets.csv", index=False)
