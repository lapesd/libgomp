#!/usr/bin/perl

# <copyright>
#    Copyright (c) 2013-2016 Intel Corporation.  All Rights Reserved.
#
#    Redistribution and use in source and binary forms, with or without
#    modification, are permitted provided that the following conditions
#    are met:
#
#      * Redistributions of source code must retain the above copyright
#        notice, this list of conditions and the following disclaimer.
#      * Redistributions in binary form must reproduce the above copyright
#        notice, this list of conditions and the following disclaimer in the
#        documentation and/or other materials provided with the distribution.
#      * Neither the name of Intel Corporation nor the names of its
#        contributors may be used to endorse or promote products derived
#        from this software without specific prior written permission.
#
#    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
#    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
#    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
#    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
#    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
#    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
#    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
#    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
#    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# </copyright>

use strict;
use warnings;

use FindBin;
use lib "$FindBin::Bin/lib";

use tools;
use Platform ":vars";

our $VERSION = "0.002";

sub execstack($) {
    my ( $file ) = @_;
    my @output;
    my @stack;
    my $tool;
    if($target_arch eq "mic") {
        $tool = "x86_64-k1om-linux-readelf";
    } else {
        $tool = "readelf";
    }
    execute( [ $tool, "-l", "-W", $file ], -stdout => \@output );
    @stack = grep( $_ =~ m{\A\s*(?:GNU_)?STACK\s+}, @output );
    if ( not @stack ) {
        # Interpret missed "STACK" line as error.
        runtime_error( "$file: No stack segment found; looks like stack would be executable." );
    }; # if
    if ( @stack > 1 ) {
        runtime_error( "$file: More than one stack segment found.", "readelf output:", @output, "(eof)" );
    }; # if
    # Typical stack lines are:
    # Linux* OS IA-32 architecture:
    #    GNU_STACK      0x000000 0x00000000 0x00000000 0x00000 0x00000 RWE 0x4
    # Linux* OS Intel(R) 64:
    #    GNU_STACK      0x000000 0x0000000000000000 0x0000000000000000 0x000000 0x000000 RWE 0x8
    if ( $stack[ 0 ] !~ m{\A\s*(?:GNU_)?STACK(?:\s+0x[0-9a-f]+){5}\s+([R ][W ][E ])\s+0x[0-9a-f]+\s*\z} ) {
        runtime_error( "$file: Cannot parse stack segment line:", ">>> $stack[ 0 ]" );
    }; # if
    my $attrs = $1;
    if ( $attrs =~ m{E} ) {
        runtime_error( "$file: Stack is executable" );
    }; # if
}; # sub execstack

get_options(
    Platform::target_options(),
);

foreach my $file ( @ARGV ) {
    execstack( $file );
}; # foreach $file

exit( 0 );

__END__

=pod

=head1 NAME

B<check-execstack.pl> -- Check whether stack is executable, issue an error if so.

=head1 SYNOPSIS

B<check-execstack.pl> I<optiion>... I<file>...

=head1 DESCRIPTION

The script checks whether stack of specified executable file, and issues error if stack is
executable. If stack is not executable, the script exits silently with zero exit code.

The script runs C<readelf> utility to get information about specified executable file. So, the
script fails if C<readelf> is not available. Effectively it means the script works only on Linux* OS
(and, probably, Intel(R) Many Integrated Core Architecture).

=head1 OPTIONS

=over

=item Standard Options

=over

=item B<--doc>

=item B<--manual>

Print full help message and exit.

=item B<--help>

Print short help message and exit.

=item B<--usage>

Print very short usage message and exit.

=item B<--verbose>

Do print informational messages.

=item B<--version>

Print program version and exit.

=item B<--quiet>

Work quiet, do not print informational messages.

=back

=back

=head1 ARGUMENTS

=over

=item I<file>

A name of executable or shared object to check. Multiple files may be specified.

=back

=head1 EXAMPLES

Check libiomp5.so library:

    $ check-execstack.pl libiomp5.so

=cut

# end of file #

