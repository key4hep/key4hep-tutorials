import matplotlib.pyplot as plt
from podio.root_io import Reader

reader = Reader('higgs_recoil_out.root')
events = reader.get('events')
higgs = []
z = []
for frame in events:
    # Don't do anything when the collection is empty
    if len(frame.get('Higgs')):
        higgs.append(frame.get('Higgs')[0].getMass())
        z.append(frame.get('Z')[0].getMass())

fig, ax = plt.subplots(1, 1)
ax.hist(higgs, bins=100, range=(0, 250), histtype='step', label='Higgs')
ax.hist(z, bins=100, range=(0, 250), histtype='step', label='Z')
ax.set_xlabel('Mass [GeV]')
ax.set_ylabel('Entries')
ax.legend()
fig.savefig('histogram.pdf')
