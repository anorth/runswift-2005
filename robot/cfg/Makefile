all: wlanconf.txt player.cfg team.cfg udp.cfg nnmc.cal

player.cfg: player.cfg.orig
	cp player.cfg.orig player.cfg

team.cfg: team.cfg.orig
	cp team.cfg.orig team.cfg

wlanconf.txt: wlanconf.txt.orig
	cp wlanconf.txt.orig wlanconf.txt
	
udp.cfg: udp.cfg.orig
	cp udp.cfg.orig udp.cfg

nnmc.cal: loosennmc.cal tightnnmc.cal ../../base/colour/mergennmc
	../../base/colour/mergennmc loosennmc.cal tightnnmc.cal > nnmc.cal

distclean: clean
	rm -f wlanconf.txt player.cfg robotgw.cfg udp.cfg

clean:
	rm -f nnmc.cal

install prep depend mkdepend:

