rUNSWift crash analysis tools
1. Traditional parsing tools
    see ../../notes/emonLog.txt for instruction

2. Adopted Mi-Pal tools
    2 steps:
        1.Parsing emon.log and generate disassembly files
            ./elp.pl -e emon.log 
            (it will generate disassembly files called aiboDis[n].ass , n is
            the number that makes the file unique from what already there)

            See the printout on the screen, pick up EPC ( runtime crash),
            linkTimeAddr ( linktime below), object name (Aperios object that
            causes the crash)
        2.Analysing the stack trace
            ./StackedIt [--help | -options] runtime linktime
            dissassembly EMON

            (runtime, linktime are taken from step 1)
            for ex: ./StackedIt 0x80db9acc 0x6301ac aiboDis6.ass emon.log

    (The source of StackedIt is in src directory, type make to make binary)
    For more info, read on the below

==============================================================================


Mi-Pal crash analysis tools.
============================

Mi-Pal Griffith University
Australia

This package contains two tools we use in debugging crashes on the Aibos.
For details on the exception log parser (elp) please see doc/ELPREADME.txt.
For instructions on StackedIt (previously called StackCheck) see doc/StackedIt_Readme.txt.

These programs copyrighted to the respective authors and are licensed under the GPL(version2) (see doc/license.txt)
No warantee is provided for these programs so use at your own risk. If you redistribute the programs in this package you must include license.txt as well.
