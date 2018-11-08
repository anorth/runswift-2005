OPENRSDK_ROOT=/usr/local/OPEN_R_SDK
SUPPORT=cfg share
COMPONENTS=GameController actuatorControl behaviours wireless vision PyCode sound

BUILD=$(SUPPORT) $(COMPONENTS)
INSTALLDIR=$(shell pwd)/MS

include makefile.config

CFG_DIR         = ./cfg
CP = rsync -cr --exclude=.svn
RM = /bin/rm

TARGETS=all install clean distclean depend mkdepend

.PHONY: $(TARGETS)

# We don't want to see the commands echoed 
.SILENT:

$(TARGETS):
	for dir in $(BUILD); do \
		(echo "In dir $$dir" && cd $$dir && $(MAKE) $@) \
	done


cleanstick:
	$(RM) -rf $(MEMSTICK)/*
	$(CP) $(OPENRSDK_ROOT)/OPEN_R/MS_ERS7/WCONSOLE/nomemprot/OPEN-R $(MEMSTICK)

cleanstickMP:
	$(RM) -rf $(MEMSTICK)/*
	$(CP) $(OPENRSDK_ROOT)/OPEN_R/MS_ERS7/WCONSOLE/memprot/OPEN-R $(MEMSTICK)/


prep:	$(CFG_DIR)/connect.cfg
	$(CP) $(CFG_DIR)/EMON.CFG $(MEMSTICK)/open-r/system/conf
	$(CP) $(CFG_DIR)/wlanconf.txt $(MEMSTICK)/open-r/system/conf
	$(CP) $(CFG_DIR)/vrcomm.cfg $(MEMSTICK)/open-r/system/conf
	$(CP) $(CFG_DIR)/connect.cfg $(MEMSTICK)/open-r/mw/conf
	$(CP) $(CFG_DIR)/object.cfg $(MEMSTICK)/open-r/mw/conf
	$(CP) $(CFG_DIR)/nnmc.cal $(MEMSTICK)
	$(CP) $(CFG_DIR)/distance.cal $(MEMSTICK)
	$(CP) $(CFG_DIR)/player.cfg $(MEMSTICK)
	$(CP) $(CFG_DIR)/offsets.txt $(MEMSTICK)
	$(CP) $(CFG_DIR)/udp.cfg $(MEMSTICK)
	$(CP) $(CFG_DIR)/team.cfg $(MEMSTICK)
	$(CP) pos $(MEMSTICK)
	./install.py PyLib $(MEMSTICK)
	./install.py PyCode $(MEMSTICK)
	$(CP) $(CFG_DIR)/TINYFTPD.BIN $(MEMSTICK)/open-r/mw/objs
	$(CP) $(CFG_DIR)/PASSWD $(MEMSTICK)/open-r/mw/conf

notLanPrep:	$(CFG_DIR)/connect.cfg
	$(CP) $(CFG_DIR)/EMON.CFG $(MEMSTICK)/open-r/system/conf
	$(CP) $(CFG_DIR)/nnmc.cal $(CFG_DIR)/distance.cal $(MEMSTICK)
	$(CP) $(CFG_DIR)/object.cfg $(CFG_DIR)/connect.cfg \
		$(MEMSTICK)/open-r/mw/conf
	$(CP) $(CFG_DIR)/*.pos $(MEMSTICK)
	$(CP) $(CFG_DIR)/robotgw.cfg $(MEMSTICK)/open-r/mw/conf
	$(CP) $(CFG_DIR)/offsets.txt $(MEMSTICK)

maybe:	$(CFG_DIR)/connect.cfg
	$(CP) $(CFG_DIR)/EMON.CFG $(MEMSTICK)/open-r/system/conf
	$(CP) $(CFG_DIR)/maybe.cal $(MEMSTICK)/nnmc.cal
	$(CP) $(CFG_DIR)/distance.cal $(MEMSTICK)
	$(CP) $(CFG_DIR)/object.cfg $(CFG_DIR)/connect.cfg \
		$(MEMSTICK)/open-r/mw/conf
	$(CP) $(CFG_DIR)/wlanconf.txt $(MEMSTICK)/open-r/system/conf
	$(CP) $(CFG_DIR)/*.pos $(MEMSTICK)
	$(CP) $(CFG_DIR)/offsets.txt $(MEMSTICK)


# this is a simple entry to switch the SVN URLs of the robot code
# so that you can access the repository outside of UNITED and insde easily
# use this rule when you're repository is pointing to roborouter and you're working on UNITED
united:
	svn switch --relocate https://roborouter.cse.unsw.edu.au/svn/robocup/ https://192.168.0.200/svn/robocup/ .

# use this rule when you're outside UNITED (CSE wireless) and you need to point your SVN URL to use roborouter
outside:
	svn switch --relocate https://192.168.0.200/svn/robocup/ https://roborouter.cse.unsw.edu.au/svn/robocup/ .

