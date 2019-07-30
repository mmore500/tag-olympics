import networkx as nx
import sys
import random
# from matplotlib import pyplot as plt
from keyname import keyname as kn

seed, nodes, degree, extra = map(int, sys.argv[1:5])

hubs, hub_degree = map(int, sys.argv[5:7]) if len(sys.argv) == 7 else (0, 0)

random.seed(seed)

G = nx.random_regular_graph(degree, nodes, seed)

# exclude self-loops
nonedges = [(a, b) for (a,b) in nx.non_edges(G) if a != b]

G.add_edges_from( random.sample(nonedges, extra) )

# use first hubs nodes as hubs
for node in range(hubs):
    G.add_edges_from([
        (node, neigh)
        for neigh in random.sample(
            [n for n in nx.non_neighbors(G, node) if n != node],
            max(0, hub_degree - G.degree(node))
        )
    ])

# nx.draw(G)
# plt.show()

outfile = kn.pack({
    'seed' : seed,
    'title' : 'mid-graph',
    'node-count' : nodes,
    'base-degree' : degree,
    'extra-edges' : extra,
    'hub-count' : hubs,
    'hub-degree' : hub_degree,
    # '_script_fullcat_hash' : fsh.FilesHash(
    #                             file_parcel="full_parcel",
    #                             files_join="cat_join"
    #                         ).hash_files([sys.argv[0]]),
    'ext' : '.csv'
})
nx.write_adjlist(G, outfile, delimiter=',')
print("output saved to", outfile)
