#!/usr/bin/env python

import sys
import numpy as np
from numpy import percentile

if __name__ == '__main__':
    fname = sys.argv[1]
    aref = np.genfromtxt(fname, delimiter=' ')
    transposed = np.transpose(aref)
    names = ['Line split', 'Line cat', 'Search filter', 'Search cat', 'Total']
    for (computation_stage, name) in zip(transposed, names):
        # min 1stQuant median 3rdQuant max
        candlesticks = percentile(computation_stage, [0, 25, 50, 75, 100])
        candlesticks.append('"'+name+'"')
        print ' '.join(map(str, candlesticks))
