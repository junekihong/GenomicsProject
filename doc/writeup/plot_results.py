#! /usr/bin/env python2

import numpy as np
import numpy.ma as ma
from scipy import optimize as opt
from matplotlib import pyplot as pyplot
from matplotlib import rc as rc
import os.path

fig = None
def new_axes():
    global fig
    fig = pyplot.figure(figsize=(6.4,5.2))
    #fig = pyplot.figure(figsize=(3.2,2.6))
    fig.subplots_adjust(left=0.2)
    fig.subplots_adjust(bottom=0.14)
    ax = fig.add_subplot(1, 1, 1)
    return ax

baseline = [2.0261590480804443, 2.013432025909424, 2.016541004180908, 2.0370638370513916, 1.953212022781372]
no_cache = {
        1:[67.923126936, 65.339925, 65.205662, 64.384956, 67.854135],
        2:[48.048969, 47.655516, 47.913405, 47.406641, 47.866398],
        4:[69.2973, 54.1127, 59.7719, 58.9647, 68.0434]
        }
remote_workers = {
        1:[58.1676, 58.1074, 58.1262, 58.0815, 59.3343],
        2:[50.6946, 50.7415, 50.9035, 50.6764, 50.7032],
        4:[52.5831, 51.7112, 69.9821, 63.3045, 54.9869]
        }
with_cache = {
        1:[68.2526, 68.4053, 68.4070, 68.3525, 68.4050],
        2:[64.3910, 64.2469, 64.8717, 64.9866, 65.4091],
        4:[62.4556, 61.9180, 63.7052, 62.7369, 62.1144]
        }
baseline_avg = np.mean(baseline)
no_averages = map( lambda (k, v): np.mean(v), no_cache.iteritems())
remote_averages = map( lambda (k, v): np.mean(v), remote_workers.iteritems())
cache_averages = map( lambda (k, v): np.mean(v), with_cache.iteritems())

baseline_stddev = np.std(baseline)
no_stddev = map( lambda (k, v): np.std(v), no_cache.iteritems())
remote_stddev = map( lambda (k, v): np.std(v), remote_workers.iteritems())
cache_stddev = map( lambda (k, v): np.std(v), with_cache.iteritems())

ax =new_axes()
ax.set_xlim(left=0)
ax.set_xlim(right=5)
#ax.set_ylim(bottom=0)
ax.plot([1,2,4], [baseline_avg] * 3, 'o-', label='Baseline')
ax.plot([1,2,4], no_averages, 'o-', label='No caching, 1 machine')
ax.plot([1,2,4], remote_averages, 'o-', label='No caching, multiple machines')
ax.plot([1,2,4], cache_averages, 'o-', label='Caching, 1 machine')
ax.set_xlabel('Number of worker processes')
ax.set_ylabel('Time for 200 alignments (seconds)')
pyplot.legend(loc=0)

pyplot.savefig('benchmarks.png', dpi=160)




if __name__ == '__main__':
    rc('font', size=10)
    
