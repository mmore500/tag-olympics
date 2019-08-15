import networkx as nx
import sys
import random
from matplotlib import pyplot as plt
from keyname import keyname as kn
from fileshash import fileshash as fsh
import argparse
import itertools as it

parser = argparse.ArgumentParser()

parser.add_argument("-s", "--seed", type=int, action="store", default=1)
parser.add_argument("-l", "--lefts", type=int, action="store", default=1)
parser.add_argument("-r", "--rights", type=int, action="store", default=1)

parser.add_argument("-e", "--regular", type=int, action="store", default=1)
parser.add_argument("-i", "--irregular", type=int, action="store", default=0)

# TODO left/right hubs
parser.add_argument("-y", "--lefthubs", type=int, action="store", default=0)
parser.add_argument("-z", "--lefthubextra", type=int, action="store", default=0)

parser.add_argument("-j", "--righthubs", type=int, action="store", default=0)
parser.add_argument("-k", "--righthubextra", type=int, action="store", default=0)

parser.add_argument("-m", "--modules", type=int, action="store", default=1)

parser.add_argument("-x", "--instances", type=int, action="store", default=1)

args = parser.parse_args()

random.seed(args.seed)

for i in range(args.instances):

    modules = []

    for m in range(args.modules):

        # define subgraph and edges
        G = nx.Graph()

        lefts = [
            kn.pack({
                "module" : m,
                "hub" : 1 if i < args.lefthubs else 0,
                "idx" : i,
                "side" : "left",
            }) for i in range(args.lefts)
        ]
        G.add_nodes_from(lefts)

        rights = [
            kn.pack({
                "module" : m,
                "hub" : 1 if i < args.righthubs else 0,
                "idx" : i,
                "side" : "right",
            }) for i in range(args.rights)
        ]
        G.add_nodes_from(rights)

        # add regular edges
        lefts_norepl = it.chain.from_iterable(
            random.sample(lefts, len(lefts))
            for __ in it.count()
        )

        rights_norepl = it.chain.from_iterable(
            random.sample(rights, len(rights))
            for __ in it.count()
        )

        for __ in range(args.regular):
            G.add_edge(*next(
                edge for edge in zip(lefts_norepl, rights_norepl)
                if not G.has_edge(*edge)
            ))

        # add irregular edges
        lefts_withrepl = (
            random.choice(lefts)
            for __ in it.count()
        )

        rights_withrepl = (
            random.choice(rights)
            for __ in it.count()
        )

        for __ in range(args.irregular):
            G.add_edge(*next(
                edge for edge in zip(lefts_withrepl, rights_withrepl)
                if not G.has_edge(*edge)
            ))

        # set up left hubs
        for node in lefts:
            if int(kn.unpack(node)['hub']):
                for target in random.sample(
                    [r for r in rights if r not in G.neighbors(node)],
                    args.lefthubextra
                ):
                    G.add_edge(node, target)



        # set up right hubs
        for node in rights:
            if int(kn.unpack(node)['hub']):
                for target in random.sample(
                    [l for l in lefts if l not in G.neighbors(node)],
                    args.righthubextra
                ):
                    G.add_edge(target, node)

        modules.append(G)

    G = nx.union_all(modules)

    assert nx.is_bipartite(G)

    # nx.draw(G, pos=nx.drawing.layout.bipartite_layout(
    #     G,
    #     [n for n in sorted(G.nodes(), key=lambda n: kn.unpack(n)['module']) if kn.unpack(n)['side'] == 'left']
    # ))
    # plt.show()

    outfile = kn.pack({
        'seed' : args.seed,
        'title' : 'mid-bigraph',
        'instance' : i,
        # '_script_fullcat_hash' : fsh.FilesHash(
        #                             file_parcel="full_parcel",
        #                             files_join="cat_join"
        #                         ).hash_files([sys.argv[0]]),
        'ext' : '.csv'
    })
    nx.write_adjlist(G, outfile, delimiter=',')
    print("output saved to", outfile)
