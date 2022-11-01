import matplotlib.pyplot as plt

data_f = open("V2X_200_0.08.csv",'r', encoding='utf-8-sig')

BSM_TIMEs = []
CBRs = []
WSA_recieved_TIMEs = []
ITTs = []

for line in data_f:
    (bsm_time, cbr, wsa_recieved_time, itt) = line.split(',')
    BSM_TIMEs.append(bsm_time)
    CBRs.append(cbr)
    WSA_recieved_TIMEs.append(wsa_recieved_time)
    ITTs.append(itt)
    
data_f.close()

BSM_TIMEs = [float (i) for i in BSM_TIMEs]
CBRs = [float (i) for i in CBRs]
WSA_recieved_TIMEs = [float (i) for i in WSA_recieved_TIMEs]
WSA_recieved_TIMEs = [i-1 for i in WSA_recieved_TIMEs]
ITTs = [float (i) for i in ITTs]

fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)

plt.plot(CBRs, ITTs, color='purple', marker='o', linestyle='', label='ITT')
plt.title("ITT stabilizing")
plt.xlabel("CBR")
plt.ylabel("ITT")
plt.grid(True)
plt.legend(loc='upper left')
plt.savefig('V2X_200_0.08_CBR_ITT.png')