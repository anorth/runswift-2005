#!/usr/bin/perl

# Insert code from one file into a specific position in a template file
# (marked with tag <insert code here>).
# Used to insert decision tree code into tree2cube.cc

$usage = "usage: perl insertCode.pl <template file> <code file> <output file>\n";
$tag = "<insert code here>";

if ($#ARGV != 2) {
    print $usage;
    exit 1;
}

$templateFile = $ARGV[0];
$codeFile = $ARGV[1];
$outFile = $ARGV[2];

open(IN, $templateFile) or die "Can't open template file";

$file = "";

while(<IN>)
{
    chomp;
    $line = $_;

    if($line ne $tag) {
	$file .= "$line\n";
    }
    else {
	open(CODE, $codeFile) or die "Can't open code file";
	open(OUT, "> $outFile");
	print OUT $file;
	while(<CODE>) {
	    print OUT $_;
	}
	while(<IN>) {
	    print OUT $_;
	}
	close OUT;
	close IN;
	close CODE;
	exit 0;
    }
}

close IN;
print "$templateFile does not contain the line $tag\n";
