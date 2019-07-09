import pandas as pd
import networkx as nx
import sys
from tqdm import tqdm
import numpy as np

in_df = pd.read_csv(sys.argv[1])

out = list()

for vals, df in tqdm(in_df.groupby(["Metric", "Sample"])):

    guess_cutoff = 0.5
    for dg in (0.5**exp for exp in range(2,12)):

        G = nx.Graph()

        for idx, row in df.iterrows():
            if row["Match Distance"] <= guess_cutoff:
                G.add_edge(row["From"], row["To"])

        degrees = np.array([G.degree(n) for n in G.nodes()])

        if np.mean(degrees) < 5:
            guess_cutoff += dg
        else:
            guess_cutoff -= dg


    G = nx.Graph()

    for idx, row in df.iterrows():
        if row["Match Distance"] <= guess_cutoff:
            G.add_edge(row["From"], row["To"])

    degrees = np.array([G.degree(n) for n in G.nodes()])

    out.append({
        "Metric" : vals[0],
        "Sample" : vals[1],
        "Cutoff" : guess_cutoff,
        "Connected Components" : nx.number_connected_components(G),
        "Mean Degree" : np.mean(degrees),
        "Median Degree" : np.median(degrees),
        "Degree Standard Deviation" : np.std(degrees)
    })


pd.DataFrame.from_records(out).to_csv("binary-search.csv", index=False)
