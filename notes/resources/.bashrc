# .bashrc

# User specific aliases and functions
# Example bashrc, use it with care

# Source global definitions
if [ -f /etc/bashrc ]; then
	. /etc/bashrc
fi

export CVS_RSH=/usr/bin/ssh
export RSYNC_RSH=/usr/bin/ssh

export SVN=https://192.168.0.200/svn/robocup
export SVNA=https://roborouter.cse.unsw.edu.au/svn/robocup
export CLASSPATH=/usr/local/gl4java:/usr/local/gl4java/gl4java.jar:/usr/java/j2sdk1.4.2_03/lib:/usr/local/Jama/Jama.jar:.

export PATH=/home/kcph007/bin:/usr/java/j2sdk1.4.2_03/bin:/etc/sysconfig/networking-scripts:/sbin:/usr/sbin:/usr/local/OPEN_R_SDK/bin:$PATH

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/gl4java

# Add the current directory to the END of the path.
# For security this should always go at the end of the login script

export PATH=${PATH}:.

#for svn commit
export EDITOR=/usr/bin/vim

export sab=192.168.0.200
export ben=192.168.0.202
export dog=192.168.0.201
export cse=cse.unsw.edu.au
export mydog=192.168.0.7



export PS1='\n\[\033]0;\w\007 \033[32m\]\u@\h \[\033[33m\w\033[0m\] \n$ '
export LC_ALL=C
export LANG=C


