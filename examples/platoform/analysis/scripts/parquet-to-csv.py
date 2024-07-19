import sys

import pandas as pd
df = pd.read_parquet(sys.argv[1])
df.to_csv(f"{sys.argv[1]}.csv")
