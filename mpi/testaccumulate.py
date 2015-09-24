#!/usr/bin/python -u

import sys # so that we can return a value at the end.
import random # for randum number generators
import getopt
import os.path

from testutils import *

sys.stdin.close()
os.close(0);

def main(argv):

    usage = "Usage:\n"\
            "./testfft2.py\n"\
            "\t-s\t\tSpecify a short run\n"\
            "\t-h\t\tShow usage"

    shortrun = False
    try:
        opts, args = getopt.getopt(argv,"sh")
    except getopt.GetoptError:
        print "Error in arguments"
        print usage
        sys.exit(2)
    for opt, arg in opts:
        if opt in ("-s"):
            shortrun = True
        if opt in ("-h"):
            print usage
            sys.exit(0)
            
    retval = 0

    print "MPI transpose unit test"

    proglist = []
    proglist.append("accumulate")
    proglist.append("accumulateyz")
    proglist.append("accumulatexy")

    logfile = 'testaccumulate.log' 
    print "Log in " + logfile + "\n"
    log = open(logfile, 'w')
    log.close()

    Xlist = [1,2,3,4,5,random.randint(10,64)]
    Ylist = [1,2,3,4,5,random.randint(10,64)]
    Zlist = [1,2,random.randint(3,64)]
    Plist = [1,2,3,4,5]
    
    if shortrun:
        Xlist = [2,random.randint(10,64)]
        Ylist = [2,5,random.randint(10,64)]
        Zlist = [1,2,random.randint(3,64)]
        Plist = [1,2]
    
    ntests = 0
    nfails = 0
    failcases = ""

    for progname in proglist:
        if not os.path.isfile(progname):
            msg = "Error: executable " + str(progname) + " not present!"
            print msg
            log = open(logfile, 'a')
            log.write(msg)
            log.close()
            retval += 1
        else:
            msg = "Running " + str(progname)
            print msg
            log = open(logfile, 'a')
            log.write(msg)
            log.close()

            timeout = 0

            for X in Xlist:
                for Y in Ylist:
                    for Z in Zlist:
                        for P in Plist:
                            ntests += 1
                            args = []
                            args.append("-x" + str(X))
                            args.append("-y" + str(Y))
                            args.append("-z" + str(Z))
                            args.append("-q")
                            rtest, cmd = runtest(progname, P, args, logfile, \
                                                 timeout)
                            if not rtest == 0:
                                nfails += 1
                                failcases += " ".join(cmd)
                                failcases += "\t(code " + str(rtest) + ")"
                                failcases += "\n"

    if nfails > 0:
        print "Failure cases:"
        print failcases
        retval += 1
    print "\n", nfails, "failures out of", ntests, "tests." 

    sys.exit(retval)
    

if __name__ == "__main__":
    main(sys.argv[1:])
