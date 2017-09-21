import ROOT as r
import os
r.gSystem.Load(os.path.expandvars('$TOOLSSYS/lib/libtools'))


__importables__ = [
    'TypeDB',
    # 'variable',
    'varEq',
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
    'luminosita',
    'calcChi2',
    'residual',
    'pull',
    'computeAverage',
    'computeAverage2D',
]

for importable in __importables__:
    exec '{0} = r.{0}'.format(importable)
