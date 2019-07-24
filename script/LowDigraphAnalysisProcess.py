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
out_egv_centralities = list()
out_load_centralities = list()

for (metric, sample), df in tqdm(in_df.groupby(["Metric", "Sample"])):

    G = nx.DiGraph()

    total_weight = df["Distance"].sum()

    n_nodepairs = int( (-1 + np.sqrt(1 + 4 * len(df)))/2 )
    n_edges_l2r = n_nodepairs * n_nodepairs

    for idx, row in df.iterrows():
        G.add_edge(
            row["From"],
            row["To"],
            weight=(
                row["Distance"] / total_weight
                if total_weight else (
                    # make left to right edges sum to 1
                    # wraparound right to left edges are sill 0
                    1.0 / n_edges_l2r if row["To"] >= n_nodepairs
                    else 0.0
                )
            )
        )

    assert abs(G.size(weight='weight') - 1.0) < 1e-6, G.size(weight='weight')

    egv_cs = nx.katz_centrality(G, weight="weight")
    load_cs = nx.load_centrality(G, weight="weight")

    egv_cvals = list(egv_cs.values())
    load_cvals = list(load_cs.values())

    for node, val in egv_cs.items():
        out_egv_centralities.append({
            "Metric" : metric,
            "Sample" : sample,
            "Node" : node,
            "Centrality" : val
        })

    for node, val in load_cs.items():
        out_load_centralities.append({
            "Metric" : metric,
            "Sample" : sample,
            "Node" : node,
            "Centrality" : val
        })

    out.append({
        "Metric" : metric,
        "Sample" : sample,
        "Maximum Eigenvector Centrality" : np.max(egv_cvals),
        "Eigenvector Centrality Variance" : np.var(egv_cvals),
        "Maximum Load Centrality" : np.max(load_cvals),
        "Median Load Centrality" : np.median(load_cvals),
        "Load Centrality Variance" : np.var(load_cvals),
        "Minimum Spanning Weight" : nx.minimum_spanning_tree(G.to_undirected()).size(weight='weight')
    })


pd.DataFrame.from_records(out).to_csv(
    kn.pack({
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
    }),
    index=False
)

pd.DataFrame.from_records(out_egv_centralities).to_csv(
    kn.pack({
        'title' : kn.unpack(dataframe_filename)['title'] + "-eigenvector-centralities",
        'bitweight' : kn.unpack(dataframe_filename)['bitweight'],
        'seed' : kn.unpack(dataframe_filename)['seed'],
        '_data_hathash_hash' : fsh.FilesHash().hash_files([dataframe_filename]),
        '_script_fullcat_hash' : fsh.FilesHash(
                                    file_parcel="full_parcel",
                                    files_join="cat_join"
                                ).hash_files([sys.argv[0]]),
        # '_source_hash' :kn.unpack(dataframe_filename)['_source_hash'],
        'ext' : '.csv'
    }),
    index=False
)

pd.DataFrame.from_records(out_load_centralities).to_csv(
    kn.pack({
        'title' : kn.unpack(dataframe_filename)['title'] + "-load-centralities",
        'bitweight' : kn.unpack(dataframe_filename)['bitweight'],
        'seed' : kn.unpack(dataframe_filename)['seed'],
        '_data_hathash_hash' : fsh.FilesHash().hash_files([dataframe_filename]),
        '_script_fullcat_hash' : fsh.FilesHash(
                                    file_parcel="full_parcel",
                                    files_join="cat_join"
                                ).hash_files([sys.argv[0]]),
        # '_source_hash' :kn.unpack(dataframe_filename)['_source_hash'],
        'ext' : '.csv'
    }),
    index=False
)
