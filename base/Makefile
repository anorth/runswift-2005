CC=gcc

COMPONENTS=client RoboCommander SimpleRoboCommander orient

TARGETS=all clean distclean

.PHONY: $(TARGETS)

$(TARGETS): 
	for dir in $(COMPONENTS); do \
		(cd $$dir && $(MAKE) $@) \
	done


# this is a simple entry to switch the SVN URLs of the robot code
# so that you can access the repository outside of UNITED and insde easily
# use this rule when you're repository is pointing to roborouter and you're working on UNITED
united:
	svn switch --relocate https://roborouter.cse.unsw.edu.au/svn/robocup/ https://192.168.0.200/svn/robocup/ .
                                                                                                                                              
# use this rule when you're outside UNITED (CSE wireless) and you need to point your SVN URL to use roborouter
outside:
	svn switch --relocate https://192.168.0.200/svn/robocup/ https://roborouter.cse.unsw.edu.au/svn/robocup/ .
                                                                                                                                            
