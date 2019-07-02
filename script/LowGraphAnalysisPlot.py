import matplotlib
matplotlib.use('Agg')
import pandas as pd
import seaborn as sns
import sys
from matplotlib import pyplot as plt
import seaborn as sns
import pandas as pd
import numpy as np

# open-type fonts
matplotlib.rcParams['pdf.fonttype'] = 42

df = pd.read_csv(sys.argv[1])

g = sns.lineplot(
    x="Cutoff",
    y="Connected Components",
    hue="Metric",
    data=df
)

g.set(xscale="log")
g.set(yscale="log")


plt.savefig(
    "low-connected-components.pdf",
    transparent=True
)

plt.clf()

g = sns.lineplot(
    x="Cutoff",
    y="Mean Degree",
    hue="Metric",
    data=df
)

g.set(xscale="log")
g.set(yscale="log")


plt.savefig(
    "low-mean-degree.pdf",
    transparent=True
)

plt.clf()

g = sns.lineplot(
    x="Cutoff",
    y="Median Degree",
    hue="Metric",
    data=df
)

g.set(xscale="log")
g.set(yscale="log")


plt.savefig(
    "low-median-degree.pdf",
    transparent=True
)

plt.clf()

g = sns.lineplot(
    x="Median Degree",
    y="Connected Components",
    hue="Metric",
    data=df
)

g.set(xscale="log")
g.set(yscale="log")


plt.savefig(
    "low-degree-cc.pdf",
    transparent=True
)

plt.clf()

g = sns.lineplot(
    x="Cutoff",
    y="Degree Standard Deviation",
    hue="Metric",
    data=df
)

g.set(xscale="log")
g.set(yscale="log")


plt.savefig(
    "low-std-degree.pdf",
    transparent=True
)
