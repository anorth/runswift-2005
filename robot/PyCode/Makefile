all: Reload.py Player.py

Reload.py: Reload.py.orig
	cp Reload.py.orig Reload.py
Player.py: Player.py.orig
	cp Player.py.orig Player.py

clean:
	rm -r -f *.pyc Reload.py *~

distclean: clean
	rm -f Player.py Reload.py
	rm -rf *.pyc

# use make check to check all python files.
check:
	#pychecker -e -Q Behaviou.py Player.py
	pychecker -e -Q Behaviou.py *.py

install clean prep depend mkdepend:

