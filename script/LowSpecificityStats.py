import sys
import seaborn as sns
import pandas as pd
import numpy as np
import scipy.stats as stats

df = pd.read_csv(sys.argv[1])

# proc = pd.DataFrame(df.groupby(["Metric","Sample"]).mean()["Match Distance"]).reset_index()
#
# proc["Specificity"] = proc["Match Distance"]

def anova(metric):
    fil = df[df["Metric"] == metric]
    print(
        metric,
        stats.f_oneway(*(
            fil["Match Distance"][fil["Sample"] == s]
            for s in fil["Sample"].unique()
        ))
    )

for metric in df["Metric"].unique():
    anova(metric)
