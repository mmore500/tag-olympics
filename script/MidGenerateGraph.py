import networkx as nx
import sys
import random
from matplotlib import pyplot as plt
from keyname import keyname as kn

seed, nodes, degree, extra = map(int, sys.argv[1:])

random.seed(seed)

G = nx.random_regular_graph(degree, nodes, seed)

nonedges = list(nx.non_edges(G))

G.add_edges_from( random.sample(nonedges, extra) )

nx.draw(G)

plt.show()

outfile = kn.pack({
    'seed' : seed,
    'title' : 'mid-graph',
    'node-count' : nodes,
    'base-degree' : degree,
    'extra-edges' : extra,
    # '_script_fullcat_hash' : fsh.FilesHash(
    #                             file_parcel="full_parcel",
    #                             files_join="cat_join"
    #                         ).hash_files([sys.argv[0]]),
    'ext' : '.csv'
})
nx.write_adjlist(G, outfile, delimiter=',')
print("output saved to", outfile)
