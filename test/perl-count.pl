#!/usr/bin/perl

print "Hello.\n";

$i = 0;

&begin;

sub begin {
    print "In begin()\n";
    while (1) {
	if ($i == 20) { last; }
	&print_count;
	&rest;
	&increment;
    }

    print("Now i is $i.\n");
}

sub increment {
    #print "In increment()\n";
    $i++;
}

sub print_count {
    #print "In print_count()\n";
    $| = 1;  # Force STDOUT to flush after each print
    print "hi";
    print "$i";
    print "bye";
    &print_a_space;
}

sub print_a_space {
    print " ";
}

sub rest {
    #print "In rest()\n";
    #sleep(1);
}
