
import matplotlib.pyplot as plt

data_f = open("Book_column.csv",'r', encoding='utf-8-sig')

TIMEs = []
CBRs = []
TESTs = []

for line in data_f:
    (TIME, CBR, TEST) = line.split(',')
    TIMEs.append(TIME)
    CBRs.append(CBR)
    TESTs.append(TEST)
    
data_f.close()

TIMEs = TIMEs[1:]
CBRs = CBRs[1:]
TESTs = TESTs[1:]

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
plt.savefig('CSV_COLUMN.png')