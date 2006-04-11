#
# VRJuggler
#   Copyright (C) 1997,1998,1999,2000
#   Iowa State University Research Foundation, Inc.
#   All Rights Reserved
#
# Original Authors:
#   Allen Bierbaum, Christopher Just,
#   Patrick Hartling, Kevin Meinert,
#   Carolina Cruz-Neira, Albert Baker
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
#
# -----------------------------------------------------------------
# File:          $RCSfile$
# Date modified: $Date$
# Version:       $Revision$
# -----------------------------------------------------------------
#

# -----------------------------------------------------------------------------
# Installation operations Perl 5 module.  This module defines the following
# subroutines for use in the Perl code that uses this.  The subroutines are
# (in alphabetical order):
#
#     - recurseDir(): Recurse through the given directory tree.
#     - newDir(): Create a new directory in a given directory tree.
#     - installFile(): Install a given file with specified permissions to a
#       destination directory.
#     - replaceTags(): Replace tags of the form "@...@" in the given file
#       with known replacment values.
# -----------------------------------------------------------------------------

package InstallOps;

require 5.003;

use Cwd;
use File::Basename;
use File::Copy;
use File::Path;

require Exporter;

@ISA = qw(Exporter);
@EXPORT = qw(recurseDir newDir installFile replaceTags);

@dirstack = ();

my $caller = caller();

my $Win32 = 1 if $ENV{'OS'} =~ /Windows/;

# -----------------------------------------------------------------------------
# Recurse through the given directory tree starting at $start_dir.  If a file
# is encountered, run &main::recurseAction() on that file.  The new directory
# tree that will be created is rooted at $base_inst_dir.  This routine
# requires that the calling package define a subroutine called
# "recurseAction()" that defines what actions to take when a normal file is
# encountered during the recursion process.
#
# Syntax:
#     recurseDir("$start_dir", "$base_inst_dir");
#
# Arguments:
#         $start_dir: The name of the directory from which the recursion
#                     begins.
#     $base_inst_dir: The base directory to which the existing directory
#                     tree will be installed.
# -----------------------------------------------------------------------------
sub recurseDir ($$) {
    my $start_dir = shift;
    my $base_inst_dir = shift;

    # Save the current directory.
    my $prevdir = cwd();

    chdir("$start_dir") or die "ERROR: Cannot chdir to $start_dir: $!\n";

    opendir(SRCDIR, ".");
    my(@files) = readdir(SRCDIR);
    closedir(SRCDIR);

    my $curfile;

    foreach $curfile ( @files ) {
	next if $curfile =~ /^\.\.?$/;		# Skip . and ..

	# $curfile is a directory.
	if ( -d "$curfile" ) {
	    next if "$curfile" eq "CVS";	# SKip CVS directories

	    newDir("$base_inst_dir", "$curfile");
	    push(@dirstack, "$curfile");
	    recurseDir("$curfile");
	    pop(@dirstack);
	}
	# $curfile is something other than a directory (most likely a normal
	# file).
	else {
	    &{"${caller}::recurseAction"}("$curfile");
	}
    }

    # Go back to the previous directory so as not to intrude upon the actions
    # of the caller.
    chdir("$prevdir");
}

# -----------------------------------------------------------------------------
# Add a new directory ($newdir) to the directory tree rooted at $base_dir.
#
# Syntax:
#     newDir("$base_dir", "$newdir");
#
# Arguments:
#     $base_dir: The base of the directory tree to which the new directory 
#                will be added.
#       $newdir: The new directory to add.
# -----------------------------------------------------------------------------
sub newDir ($$) {
    my $base_dir = shift;
    my $newdir = shift;

    # Save the current directory.
    my $prevdir = cwd();

    # As long as $newdir does not exist, use mkpath() to create it.
    if ( ! -d "$newdir" ) {
	chdir("$base_dir")
	    or die "newDir(): WARNING: Could not chdir to $base_dir: $!\n";

	umask(002);
	mkpath("$newdir", 0, 0755)
	    or warn "newDir(): WARNING: Could not make $newdir: $!\n";
    }

    # Go back to the previous directory so as not to intrude upon the actions
    # of the caller.
    chdir("$prevdir");
}

# -----------------------------------------------------------------------------
# Install the given file with the specified permissions to the destination
# directory.
# 
# Syntax:
#     $filename: The file to install.
#          $uid: The ID of the user who will own the file.
#          $gid: The ID of the group that will own the file.
#         $mode: The mode bits for the file.
#     $dest_dir: The destination directory for the file.
# -----------------------------------------------------------------------------
sub installFile ($$$$$) {
    my $filename = shift;
    my $uid = shift;
    my $gid = shift;
    my $mode = shift;
    my $dest_dir = shift;

    my $src_file = "$filename";

    my $root = $dirstack[0];
    $dirstack[0] = ".";
    my $inst_path = join('/', @dirstack);
    my $inst_dir = "$dest_dir/$inst_path";
    $dirstack[0] = "$root";

    print "$inst_path/$src_file -> $inst_dir/$filename\n";

    umask(002);
    mkpath("$inst_dir", 0, 0755) or "mkpath: $!\n";
    copy("$src_file", "$inst_dir") or "copy: $!\n";
    chown($uid, $gid, "$inst_dir/$filename") or die "chown: $!\n";
    chmod(oct($mode), "$inst_dir/$filename") or die "chmod: $!\n";
}

# -----------------------------------------------------------------------------
# Replace tags of the form "@...@" found in $infile with known values.  The
# values for the tags are given in the %VARS hash which is indexed by the tag
# name (without the surrounding @'s).
#
# Syntax:
#     $count = replaceTags("$infile", %VARS);
#
# Aguments:
#      $infile: The input file to be read.
#        %VARS: The hash of tags with replacement values.
#
# Returns:
#     The count of tags replaced in the input file (>= 0) on success.
#     -1 on error.
# -----------------------------------------------------------------------------
sub replaceTags ($%) {
    my $infile = shift;
    my(%VARS) = @_;

    my $count = 0;

    my $progname = (fileparse("$0"))[0];

    # Open the input file, read its contents into @input_file and close it.
    # Once it is in the array, we no longer need to worry about it.
    if ( ! open(INPUT, "$infile") ) {
	warn "WARNING: Cannot read from $infile: $!\n";
	return -1;
    }

    my(@input_file) = <INPUT>;
    close(INPUT) or warn "WARNING: Cannot close $infile: $!\n";

    # Loop over all the lines in @input_array and replace occurrences of the
    # tags (the keys of %VARS) with the corresponding values.
    my $line;
    foreach $line ( @input_file ) {
	my $tag;
	foreach $tag ( keys(%VARS) ) {
	    $count++ if $line =~ /\@$tag\@/;
	    $line =~ s/\@$tag\@/$VARS{"$tag"}/g;
	}
    }

    # Create the output file by overwriting the input file.  The purpose is
    # to replace the input file anyway, and overwriting it is the easiest way
    # to accomplish this.
    if ( ! open(OUTPUT, "> $infile") ) {
	warn "WARNING: Cannot create $infile: $!\n";
	return -1;
    }

    # Now generate the new output file using the contents of @input_file with
    # all @..@ (that can be substituted) replaced.
    foreach ( @input_file ) {
	print OUTPUT;
    }

    close(OUTPUT) or warn "WARNING: Cannot save $infile: $!\n";

    return $count;
}

1;
