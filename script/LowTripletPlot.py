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

sns.distplot(
    df[df["Metric"] == "Streak Distance"]["Detour Difference"],
    color="skyblue",
    label="Streak Distance"
)
sns.distplot(
    df[df["Metric"] == "Hamming Distance"]["Detour Difference"],
    color="red",
    label="Hamming Distance"
)
# sns.plt.legend()

plt.savefig(
    "detour-dists.pdf",
    transparent=True
)
