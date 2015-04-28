# ECSE 457 Project Code
# Ashley Simpson
# Wei Sing Ta

# DP strips the useful data from Tool Dump within Brainsight

import math
import re
import io
import sys
import os
import time

# just outputs the last lines of code into a new text file
def remove_last_lines():
    while(1):
        
        # opens the files
        f_in = open(os.path.expanduser("~/Desktop/DP/Tool Locations Dump.txt"),'r')
        f_out = open(os.path.expanduser("//Volumes/Shared/brainsight/output"),'w')
        
        last_lines = f_in.readlines()
        f_out.write(last_lines[-8])
        f_out.write(last_lines[-7])
        f_out.write(last_lines[-6])
        f_out.write(last_lines[-5])
        f_out.write(last_lines[-4])
        f_out.write(last_lines[-3])
        f_out.write(last_lines[-2])
        f_out.write(last_lines[-1])

        # closes the files
        f_in.close()
        f_out.close()
        time.sleep(0.5)

# deals with command line input
if len(sys.argv) == 1:
    remove_last_lines()
else:
    print "Incorrect input! Command: python useful_data.py"

