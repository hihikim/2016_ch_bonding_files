import subprocess
import os
import time
import shutil
import datetime
import gflags
import random
import sys

OUTPUT_FILE_PATH = "./output/ap/"
FLAGS = gflags.FLAGS

'''
Definition of flags
'''
gflags.DEFINE_bool('up', False, 'Is the test for uplink, otherwise downlink will be used as a default')


'''
Function to make command line
'''
def buildCommandLine(uplink, input_file):
    cmd = "./waf --run scratch/my_test --command-template=\"%s --link_type="
    cmd += uplink
    cmd += " --test_number="
    cmd += input_file
    cmd += "\""
    # DEBUG: cmd = "echo " + input_file
    return cmd

'''
Function to run new process 
'''
def runCommand(command):
    return subprocess.Popen(command, shell=True,
                                  stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE,
                                  stderr=subprocess.PIPE)

'''
Delete completed process in the list

'''
def deleteCompleteProcess(proc_list, input_list):
    for proc in proc_list:
        if proc.poll() is not None:
            #check the output
            if len(input_list) != len(proc_list):
                # something wrong
                print("ERROR:: some input file names are leaked")
                print("Len(input) [%d], Len(proc) [%d]" % (len(input_list), len(proc_list)))
                del proc
                exit(-1)

            output_file = input_list[proc_list.index(proc)]
            
            del input_list[proc_list.index(proc)]
            del proc_list[proc_list.index(proc)]

            if not os.path.isfile(OUTPUT_FILE_PATH + output_file):
                # if there is no file, rerun process
            	cmd = ''
	    	if FLAGS.up == True:
			cmd = buildCommandLine("True", output_file)
 	    	else:
			cmd = buildCommandLine("False", output_file)
                
		print("Re-run: " + cmd)
                fd = runCommand(cmd)
                procs.append(fd)
                input_list.append(output_file)


'''
Main script
'''

if __name__ == '__main__':

    FLAGS(sys.argv)
    s = datetime.datetime.now()
    print "start time : " + str(s)


    input_files = []

    procs = []

    INPUT_FILE_PATH = "./input/ap/"
    MAX_PROCESS = 15

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
    input_file_name_list = []

    while len(input_files) is not 0:
        while len(procs) < MAX_PROCESS and len(input_files) is not 0 :
            input_file_name = input_files.pop(0)
            input_file_name_list.append(input_file_name)

            #add jobs
            cmd = ''
        if FLAGS.up == True:
            cmd = buildCommandLine("True", input_file_name)
        else:
            cmd = buildCommandLine("False", input_file_name)
        print cmd
        time.sleep(random.random() * 5)  # time delay 0~5 sec
        fd = runCommand(cmd)
        procs.append(fd)

        # delete process that is finished
        deleteCompleteProcess(procs, input_file_name_list)

        # little wait
        time.sleep(1)

    while len(procs) is not 0:
        deleteCompleteProcess(procs, input_file_name_list)
        time.sleep(1)

    print "all test is end"
    e = datetime.datetime.now()
    print "End Time : " + str(e)
    print "spend time : " + str(e - s)
               

