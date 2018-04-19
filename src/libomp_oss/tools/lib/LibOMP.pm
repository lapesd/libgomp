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
package LibOMP;

use strict;
use warnings;

use tools;

sub empty($) {
    my ( $var ) = @_;
    return not exists( $ENV{ $var } ) or not defined( $ENV{ $var } ) or $ENV{ $var } eq "";
}; # sub empty

my ( $base, $out, $tmp );
if ( empty( "LIBOMP_WORK" ) ) {
    # $FindBin::Bin is not used intentionally because it gives real path. I want to use absolute,
    # but not real one (real path does not contain symlinks while absolute path may contain
    # symlinks).
    $base = get_dir( get_dir( abs_path( $0 ) ) );
} else {
    $base = abs_path( $ENV{ LIBOMP_WORK } );
}; # if

if ( empty( "LIBOMP_EXPORTS" ) ) {
    $out = cat_dir( $base, "exports" );
} else {
    $out = abs_path( $ENV{ LIBOMP_EXPORTS } );
}; # if

if ( empty( "LIBOMP_TMP" ) ) {
    $tmp = cat_dir( $base, "tmp" );
} else {
    $tmp = abs_path( $ENV{ LIBOMP_TMP } );
}; # if

$ENV{ LIBOMP_WORK    } = $base;
$ENV{ LIBOMP_EXPORTS } = $out;
$ENV{ LIBOMP_TMP     } = $tmp;

return 1;

__END__

=pod

=head1 NAME

B<LibOMP.pm> --

=head1 SYNOPSIS

    use FindBin;
    use lib "$FindBin::Bin/lib";
    use LibOMP;

    $ENV{ LIBOMP_WORK    }
    $ENV{ LIBOMP_TMP     }
    $ENV{ LIBOMP_EXPORTS }

=head1 DESCRIPTION

The module checks C<LIBOMP_WORK>, C<LIBOMP_EXPORTS>, and C<LIBOMP_TMP> environments variables.
If a variable set, the module makes sure it is absolute. If a variable does not exist, the module
sets it to default value.

Default value for C<LIBOMP_EXPORTS> is C<$LIBOMP_WORK/exports>, for C<LIBOMP_TMP> --
C<$LIBOMP_WORK/tmp>.

Value for C<LIBOMP_WORK> is guessed. The module assumes the script (which uses the module) is
located in C<tools/> directory of libomp directory tree, and uses path of the script to calculate
C<LIBOMP_WORK>,

=cut

# end of file #

