import os
import ROOT as r

r.gSystem.Load(os.path.expandvars('$TOOLSSYS/lib/libtools'))
r.gSystem.Load(os.path.expandvars('$TOOLSSYS/lib/libroofit'))

#If you have compiled with CMake and you want to load both the libraries in one go, change the previous lines to 
#gSystem.Load(os.path.expandvars('$TOOLSSYS/build/libeasyanalysis.so') and make sure you have LD_LIBRARY_PATH set to $TOOLSSYS/build)

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
