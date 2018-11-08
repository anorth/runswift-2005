##     Copyright 2005 The University of New South Wales (UNSW) and National  
##     ICT Australia (NICTA).
##  
##     This file is part of the 2005 team rUNSWift RoboCup entry.  You may  
##     redistribute it and/or modify it under the terms of the GNU General  
##     Public License as published by the Free Software Foundation; either  
##     version 2 of the License, or (at your option) any later version as  
##     modified below.  As the original licensors, we add the following  
##     conditions to that license:
##  
##     In paragraph 2.b), the phrase "distribute or publish" should be  
##     interpreted to include entry into a competition, and hence the source  
##     of any derived work entered into a competition must be made available  
##     to all parties involved in that competition under the terms of this  
##     license.
##  
##     In addition, if the authors of a derived work publish any conference  
##     proceedings, journal articles or other academic papers describing that  
##     derived work, then appropriate academic citations to the original work  
##     should be included in that publication.
##  
##     This rUNSWift source is distributed in the hope that it will be useful,  
##     but WITHOUT ANY WARRANTY; without even the implied warranty of  
##     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU  
##     General Public License for more details.
##  
##     You should have received a copy of the GNU General Public License along  
##     with this source code; if not, write to the Free Software Foundation,  
##     Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA


#!/usr/bin/python

from distutils.core import setup, Extension

# A few of the behaviours ones are unused but required for linking. Remove
# them from this list if they are ever extricated from the code
sources = [ 'offlineVision.cc',
            'robot/behaviours/Behaviours.cc',
            'robot/behaviours/firePawKick.cc',
            'robot/behaviours/locateBall.cc',
            'robot/behaviours/pythonBehaviours.cc',
            'robot/behaviours/UNSWSkills.cc',
            'robot/behaviours/utility.cc',
            'robot/share/CommandData.cc',
            'robot/share/Common.cc',
            'robot/vision/CommonSense.cc',
            'robot/vision/CorrectedImage.cc',
            'robot/vision/gps.cc',
            'robot/vision/FieldLineVision.cc',
            'robot/vision/InfoManager.cc',
            'robot/vision/KalmanInfo2D.cc',
            'robot/vision/KI2DWithVelocity.cc',
            'robot/vision/Obstacle.cc',
            'robot/vision/Profile.cc',
            'robot/vision/PyEmbed.cc',
            'robot/vision/SanityChecks.cc',
            'robot/vision/SlamChallenge.cc',
            'robot/vision/SubObject.cc',
            'robot/vision/SubVision.cc',
            'robot/vision/VisualCortex.cc']

module1 = Extension('VisionLink',
                    libraries = ['stdc++'],  # so c++ files will build
                    define_macros = [('OFFLINE', '1')], # #define OFFLINE
                    extra_compile_args = ['-O0'],
                    sources = sources)

setup (name = 'VisionLink',        # name to be imported
       version = '0.1',         # perpetual alpha :)
       description = 'Extending the offline behaviours',
       ext_modules = [module1])
