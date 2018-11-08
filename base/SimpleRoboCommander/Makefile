COMPONENTS=CPlaneClient

TARGETS=all clean distclean

.PHONY: $(TARGETS)

$(TARGETS): 
	for dir in $(COMPONENTS); do \
		(cd $$dir && $(MAKE) $@) \
	done


main:
	###########################
	#"Nothing needs to be made"
	###########################
joystick:
	patch ClientServer.py < joystick_enabled.patch
