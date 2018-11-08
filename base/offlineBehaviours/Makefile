build: offlineVision.cc setup.py robot OfflineBehaviours.py frmOfflineBehaviours.ui
	pyuic frmOfflineBehaviours.ui > frmOfflineBehaviours.py
	rm -f build/lib*/*
	python setup.py build
	if [ ! -f VisionLink.so ] ; then ln -s build/lib*/* ; fi
	touch setup.py

clean:
	rm -fr build/
	rm frmOfflineBehaviours.py frmOfflineBehaviours.pyc
	rm VisionLink.so
