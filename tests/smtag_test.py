#!/usr/bin/env python

import sys, os, shutil, time, threading, argparse, test_common
from subprocess import *

parser = argparse.ArgumentParser()
parser.add_argument('-b', '--bindir', help="set directory where img2sm lives", default=test_common.FindBinDir('img2sm'))

args = parser.parse_args()

fixed = test_common.CheckAndFixDir(args.bindir)
if not fixed:
    errexit("bindir %s does not exist.  Please use the --bindir argument."%bindir)
bindir = fixed

smtag = test_common.FindBinary(bindir, "smtag")

bindir = os.path.abspath(os.path.dirname(img2sm))
sys.stderr.write( "bindir is: %s\n"%bindir)
sys.stderr.write( "Found smtag at %s"% smtag)

