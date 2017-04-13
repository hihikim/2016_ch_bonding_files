import subprocess
import os
import time
import shutil
import datetime


s = datetime.datetime.now()
print "start time : " + str(s)


input_files = []

procs = []

INPUT_FILE_PATH = "./input/ap/"
MAX_PROCESS = 16

try:
    shutil.rmtree('./output')

except OSError as e:
    if e.errno == 2:
        pass
    else:
        raise

os.makedirs('./output')
os.makedirs('./output/ap')
os.makedirs('./output/ap/opt')
os.makedirs('./output/ap/proposed')
os.makedirs('./output/ap/waterfall')

os.makedirs('./output/sta')
os.makedirs('./output/sta/opt')
os.makedirs('./output/sta/proposed')
os.makedirs('./output/sta/waterfall')


for root, dirs, files in os.walk(INPUT_FILE_PATH + "opt/"):
    for filename in files:
        if(filename[0] != '.'):
            input_files.append("opt/" + filename)


for root, dirs, files in os.walk(INPUT_FILE_PATH + "proposed/"):
    for filename in files:
        if(filename[0] != '.'):
            input_files.append("proposed/" + filename)

for root, dirs, files in os.walk(INPUT_FILE_PATH + "waterfall/"):
    for filename in files:
        if(filename[0] != '.'):
            input_files.append("waterfall/" + filename)

'''
for root, dirs, files in os.walk(INPUT_FILE_PATH):
    for filename in files:
        if(filename[0] != '.'):
            input_files.append(filename)
'''
while len(input_files) is not 0:
    while len(procs) < MAX_PROCESS and len(input_files) is not 0 :
        #add jobs
        cmd = "./waf --run scratch/my_test --command-template=\"%s --test_number="
        cmd += input_files.pop(0)
        cmd += "\""
        print cmd
        fd = subprocess.Popen(cmd, shell=True,
                              stdin=subprocess.PIPE,
                              stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE)
        procs.append(fd)

    deleted_num = 0
    for i in range(len (procs)):
        if procs[i - deleted_num ].poll() is not None:
            del procs[i - deleted_num ]
            deleted_num += 1


    time.sleep(1)

while len(procs) is not 0:
    deleted_num = 0
    for i in range(len (procs)):
        if procs[i - deleted_num ].poll() is not None:
            del procs[i-deleted_num]
            deleted_num += 1

    time.sleep(1)

print "all test is end"
e = datetime.datetime.now()
print "End Time : " + str(e)
print "spend time : " + str(e - s)

