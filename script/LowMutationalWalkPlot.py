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

sns.lineplot(
    x="Step",
    y="Match Distance",
    hue="Metric",
    data=df
)

plt.savefig(
    "metric-walks.pdf",
    transparent=True
)
