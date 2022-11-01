import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

import csv
data = list()
f = open("./Book_row.csv", 'r', encoding='utf-8-sig')
rea = csv.reader(f)
for row in rea:
    data.append(row)
f.close

df = pd.DataFrame(data)
df_2 = df.drop([0], axis = 1)
df_num_2 = df_2.to_numpy()

TIMEs = df_num_2[0]
CBRs = df_num_2[1]
TESTs = df_num_2[2]

TIMEs = [float (i) for i in TIMEs]
CBRs = [float (i) for i in CBRs]
TESTs = [float (i) for i in TESTs]

fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)

plt.plot(TIMEs, CBRs, color='magenta', marker='o', linestyle='dashed', label='modfied CBR')
plt.plot(TIMEs, TESTs, color='cyan', marker='o', linestyle='solid', label='unmodfied CBR')
plt.title("CBR stabilizing")
plt.xlabel("TIME")
plt.ylabel("CBR")
plt.grid(True)
plt.legend(loc='upper left')
plt.savefig('CSV_ROW.png')