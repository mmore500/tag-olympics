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

plt.clf()

g = sns.barplot(
    x="Metric",
    y="Connected Components",
    hue="Metric",
    data=df
)

# g.set(yscale="log")

plt.savefig(
    "low-degree-cc-bar.pdf",
    transparent=True
)

print(df.groupby("Metric")["Mean Degree"].mean())
