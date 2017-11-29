#!/usr/bin/env python

import os
import easyanalysis as ea

#a = ea.parser.create_analysis(os.path.expandvars("$TOOLSSYS/Tutorials/test.yaml"))
#a = ea.parser.create_analysis(os.path.expandvars("test.yaml"))
a = ea.parser.create_analysis(os.path.expandvars("$TOOLSSYS/python/test.yaml"))

a.Fit()
