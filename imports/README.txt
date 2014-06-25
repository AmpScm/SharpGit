 The SharpGit /imports/ directory
 =================================================
  $Id$

This directory contains a number of scripts to allow building LibGit2
in a number of ways. Normally you only build one variant.

  1) An almost static release for SharpSvn with minor patches
  
The scripts require the following tools: (currently used version)
  * NAnt        (0.92)
  * Perl 5      (Activestate perl   5.16.3 x86)
  * Visual Studio 2008, 2010, 2012 or 2013
    (Untested, so might require tweaks: 2005 and '14')

It is assumed all these tools are in the PATH and the settings for
the 'Visual Studio Command Prompt' are loaded.

1) To build all the dependencies (including libgit2) as used by SharpSvn
   Open the visual studio command prompt for the right architecture,
   e.g. VS2012 x64 Cross Tools Command Prompt
   
   I recommend using the 'native x86' or 'x64 cross' tools as the native
   x64 variants can't always find everything in MSBuild.

   * Go to 'imports'
   * Type> 'nant'
   After something between 5 and 30 minutes you should see build succeeded
   (with maybe 'a few' ignored errors)
