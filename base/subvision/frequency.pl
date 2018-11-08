#!/usr/bin/perl

print "opening $ARGV[0]\n";
open(HANDLE, "$ARGV[0]") || die("Could not open file!");
@data =<HANDLE>;
close HANDLE;
#@sorted = sort @data;
#sorted data;
@sorted = sort by_first_tab @data;   # ASCII-betical sort

#counting
%quantity = {};
%total = {};
$cost = 0;
$totalstep = 0;
$totaledges = 0;
foreach $line (@sorted){
  ($a1,$a2,$a3) = split /\t/,$line;
  if (exists $quantity{$a1}) {
      $quantity{$a1} +=1;
      #total of match
      $total{$a1} += $a3;
  }else {
      $quantity{$a1} = 1;
      $total{$a1} = $a3;
  }
  $cost += $a1*$a2;
  $totaledges += $a2;
  $totalstep += $a1;
}

#
$outputfile = "$ARGV[0].fq";
open(HANDLE, ">$outputfile") || die("Could not open file!");
print "set xlabel \"Steps\"; set ylabel \"Frequency\"; set zlabel \"Average match\"; splot \"$outputfile\"\n";
print "Cost:$cost Steps:$totalstep Edges:$totaledges\n";
print HANDLE "#set xlabel \"Steps\"; set ylabel \"Frequency\"; set zlabel \"Average match\"; splot \"$outputfile\"\n";
print HANDLE "#Cost:$cost Steps:$totalstep Edges:$totaledges\n";

foreach $key (sort { $a <=> $b }keys %quantity){
    if ($key == 0)
    {
        next;#continue;
    }
    $ave= $total{$key}/$quantity{$key};
    print HANDLE "$key\t$quantity{$key}\t$ave\n";
}
close HANDLE;

sub by_first_tab {
  # vars $a and $b automatically passed in

  ($a1,$a2,$a3) = split /\t/,$a;
  ($b1,$b2,$b3) = split /\t/,$b;
  if (not $a1 <=> $b1){
    if (not $a2 <=> $b2) {
       return $a3 <=> $b3;
    } else
    {
       return $a2 <=> $b2;
    }
  }
  else 
  {
    return $a1 <=> $b1;
  }
}
