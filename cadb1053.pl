#!/usr/bin/perl
use strict;
use warnings;
use Time::HiRes qw( time );
use POSIX;

sub valid_htwl {
    my $hpwl = -1;
    my $dir = $_[0];
    open(FILE, "<", $dir) or die $!;
    while(<FILE>){
        chomp $_;
        my $line = $_;
        # print($line);
        $hpwl = $1 if( $line =~ /Total HPWL for this design is (\d+)/);
        
    }
    close(FILE);
    return $hpwl;
}

my $start = time();
# print($#ARGV ."\n");

if ($#ARGV != 1) {
    print("Usage: ./cadb1053_beta <input> <output>\n");
    exit();
} else {
    # print($ARGV[0] ."\n");
    # print($ARGV[1] ."\n");
    my $temp = "temp.txt";
    my $best_hpwl = POSIX::INT_MAX;
    
    my $iter = 1;
    while (time() - $start < 60) {
        system "./bin/3d-placer " .$ARGV[0] ." " .$temp ." -safe_mode -no_dump &> placer.log";
        system "./evaluator/evaluator " .$ARGV[0] ." " .$temp ." &> evaluator/temp.log";
        my $hpwl = &valid_htwl("evaluator/temp.log");
        print ("[" .$iter ."] " ."HPWL = " .$hpwl ."\n");
        if ($hpwl > 0 && $hpwl < $best_hpwl) {
            $best_hpwl = $hpwl;
            system "cp " .$temp ." " .$ARGV[1];
            print ("Update Best HPWL = " .$hpwl ."\n");
        }
        $iter += 1;
    }
    print ("Best HPWL=" .$best_hpwl ."\n");
}

# my $end = time();
# printf("Execution Time: %0.02f s\n", $end - $start);