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

proc = pd.DataFrame(df.groupby(["Metric","Sample"]).mean()["Match Distance"]).reset_index()

proc["Specificity"] = proc["Match Distance"]

print(proc[proc["Metric"] == "Sliding Hamming Distance"]["Specificity"])

sns.distplot(
    proc[proc["Metric"] == "Sliding Streak Distance"]["Specificity"],
    color="skyblue",
    label="Sliding Streak Distance"
)
sns.distplot(
    proc[proc["Metric"] == "Sliding Streak Distance"]["Specificity"],
    color="skyblue",
    label="Sliding Streak Distance"
)
sns.distplot(
    proc[proc["Metric"] == "Hamming Distance"]["Specificity"],
    color="green",
    label="Hamming Distance"
)
sns.distplot(
    proc[proc["Metric"] == "Streak Distance"]["Specificity"],
    color="red",
    label="Streak Distance"
)
# sns.plt.legend()

plt.savefig(
    "specificity-analysis.pdf",
    transparent=True
)
