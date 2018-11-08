NOTE
----

Compiling:
1. Type "make" in offvision directory.
2. It compiles files in robot directory.
3. If it doesn't work, try "make clean" in robot directory.


Running:

1. Create a frames directory 
	mkdir frames
2. Create a objects directory
	mkdir objects 
	mkdir gps
3. Assuming your logfile is CPLANE_LOG
   Type "extract.pl CPLANE_LOG" to extract frames into the frames directory.
4. If you want to know how many frames generated, then look at the frames
   directory.   
4. Type "process <start of squence> <end of sequence> <PREFIX>

   to start the OffVision Module.
   
   ie, process 1 100 CPLANE_LOG
       will process CPLANE_LOG.1 .. to CPLANE_LOG.100
       and produce CPLANE_LOG.1.object .. to CPLANE_LOG.100.object
       You can scroll all the way from 1 to 100.

---------------------
-- Sanity Learning --
---------------------

# split the file into one frame by itself
extract.pl logFile

# list the files to be processed
java ListFile startIndex endIndex logFile

# filter the lines used to generate the c4.5 names
java AttributeFilter > c4.5attributes.txt
change the order of some of the lines as specified there
copy and paste the c4.5attributes.txt to FindBall()

# generate the blobs and c4.5 names files
offvision < logFile

# manually generate the c4.5 data files
java OffVision -blob logFile.startIndex

# automatically generate the c4.5 data files (all invalid)
java OffVision -blob logFile.startIndex endIndex
