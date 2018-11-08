#!/usr/bin/env python

import sys, os, re, filecmp, shutil

def copyFile(pyRe, trimRe, copiedNames, src, dst, name, allDstFiles):
	pyMatch = pyRe.match(name)
	# print "Process File.  src: " + src + " dst: " + dst + " name: " + name
	if pyMatch != None:
		baseName = pyMatch.group(1)
		# print "Match " + baseName
		dstName = baseName + ".py"
		if copiedNames.get(dstName.upper(), None) != None:
			raise Exception("Copying over another file", dstName, name)
		copiedNames[dstName.upper()]=name
		# print "File dstName " + dstName
		srcPath = os.path.join(src, name)
		dstPath = os.path.join(dst, dstName)
		if not (os.path.exists(dstPath) and filecmp.cmp(srcPath, dstPath)):
			shutil.copyfile(srcPath, dstPath)
			print "Copying " + srcPath + " to " + dstPath
			compName = baseName.upper()+".PYC"
			for compFile in allDstFiles:
				# print "testing " + compFile.upper() + " and " + compName
				if compFile.upper() == compName:
					compPath = os.path.join(dst, compFile)
					print "removing compiled version: " + compFile
					os.remove(compPath)
		# else:
			# print srcPath + " and " + dstPath + " are the same"
	else:
		trimMatch = trimRe.match(name)
		if trimMatch == None:
			raise "File will not fit on 8.3 filesystem", name
		if copiedNames.get(name.upper(), None) != None:
			raise "Copying over another file", name
		copiedNames[name.upper()]=name
		srcPath = os.path.join(src, name)
		dstPath = os.path.join(dst, name)
		if not (os.path.exists(dstPath) and filecmp.cmp(srcPath, dstPath)):
			shutil.copyfile(srcPath, dstPath)
			print "Copying " + srcPath + " to " + dstPath
		

def copyTree(pyRe, trimRe, src, dst):
	# print "Copying " + src + " to " + dst
	if not os.path.exists(dst):
		os.mkdir(dst)
	elif not os.path.isdir(dst):
		raise Exception("Non-directory already exists where we need a directory", dst)
	errors = []
	names = os.listdir(src)
	dstNames = os.listdir(dst)
	copiedNames = {}
	for name in names:
		if name[-3:] != ".py": # this will NOT copy .py files in any subdirectories
			continue	
		if name[0] == "." or name[-3:] == "swp" or name[:10] == "VisionLink" or name[-3:] == "pyc" or name[-4:] == "orig" or name[-1] == "~" or name == "doc" or name[0] == "#":
			continue
        
		srcName = os.path.join(src, name)
		dstName = os.path.join(dst, name)
		try:
			if os.path.isdir(srcName):
				copyTree(pyRe, trimRe, srcName, dstName)
			else:
				copyFile(pyRe, trimRe, copiedNames, src, dst, name, dstNames)
		except (IOError, os.error), why:
			errors.append((srcName, dstName, why))
	if errors:
		raise Error, errors


pyRe = re.compile("^([^.]{,8})[^.]*\.py$",re.IGNORECASE)
trimRe = re.compile("^([^.]{,8})(\.[^.]{0,3})?$")

src = sys.argv[1]
dst = sys.argv[2]
if not os.path.exists(src):
	raise "source doesn't exist", src
srcd,name = os.path.split(src)
if os.path.isdir(src):		#we're copying a dir
	copyTree(pyRe, trimRe, src, os.path.join(dst,name))
else:						# we're copying a file
	if os.path.exists(dst) and os.path.isdir(dst):
		copyFile(pyRe, trimRe, {}, srcd, dst, name, os.listdir(dst))
	else:
		dst, dstname = os.path.split(dst)
		if dstname != name:
			raise "Cannot rename a file using install.py", name
		copyFile(pyRe, trimRe, {}, srcd, dst, name, os.listdir(dst))
