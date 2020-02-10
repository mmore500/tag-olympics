import pandas as pd
import networkx as nx
import sys
from tqdm import tqdm
import numpy as np

from keyname import keyname as kn
from fileshash import fileshash as fsh

dataframe_filename = sys.argv[1]

in_df = pd.read_csv(dataframe_filename)
out = list()

for (metric, sample), df in tqdm(in_df.groupby(["Metric", "Sample"])):

    G = nx.Graph()

    total_weight = df["Match Score"].sum()

    for idx, row in df.iterrows():
        if row["From"] != row["To"]:
            G.add_edge(
            row["From"],
            row["To"],
            weight=(
                row["Match Score"] / total_weight
                if total_weight else 1.0 / len(df)
            )
        )

    edges=list(sorted(
        G.edges(data=True), key=lambda t: t[2].get('weight', 1)
    ))

    test = set()
    while not nx.is_edge_cover(G, test):
        subject = edges[len(test)]
        test.add( (subject[0], subject[1]) )


    out.append({
        "Metric" : metric,
        "Sample" : sample,
        "Sorted Cover Size" : len(test),
    })

outfile = kn.pack({
    'title' : kn.unpack(dataframe_filename)['title'] + "-stats",
    'bitweight' : kn.unpack(dataframe_filename)['bitweight'],
    'seed' : kn.unpack(dataframe_filename)['seed'],
    '_data_hathash_hash' : fsh.FilesHash().hash_files([dataframe_filename]),
    '_script_fullcat_hash' : fsh.FilesHash(
                                file_parcel="full_parcel",
                                files_join="cat_join"
                            ).hash_files([sys.argv[0]]),
    # '_source_hash' :kn.unpack(dataframe_filename)['_source_hash'],
    'ext' : '.csv'
})
pd.DataFrame.from_records(out).to_csv(
    outfile,
    index=False
)
print("output saved to", outfile)
