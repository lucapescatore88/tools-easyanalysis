import os
import ROOT as r

r.gSystem.Load(os.path.expandvars('$TOOLSSYS/lib/libtools'))
r.gSystem.Load(os.path.expandvars('$TOOLSSYS/lib/libroofit'))

__importables__ = [
    'TypeDB',
    'TreeReader',
    'CutOptimizer',
    'ModelBuilder',
    'EffComp',
    'CutFlow',
    'Scaler',
    'Analysis',
    'MultiAnalysis',
    'genRndm',
    'luminosity',
    'calcChi2',
    'residual',
    'pull',
    'computeAverage',
    'computeAverage2D',
]

for importable in __importables__:
    exec '{0} = r.{0}'.format(importable)

import parser
