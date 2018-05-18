.\" vi: set ts=8 shiftwidth=8 noexpandtab:
.\"
.\" VNPForth - Compiled native Forth for x86 Linux
.\" Copyright (C) 2005-2013  Simon Baldwin (simon_baldwin@yahoo.com)
.\"
.\" This program is free software; you can redistribute it and/or
.\" modify it under the terms of the GNU General Public License
.\" as published by the Free Software Foundation; either version 2
.\" of the License, or (at your option) any later version.
.\"
.\" This program is distributed in the hope that it will be useful,
.\" but WITHOUT ANY WARRANTY; without even the implied warranty of
.\" MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
.\" GNU General Public License for more details.
.\"
.\" You should have received a copy of the GNU General Public License
.\" along with this program; if not, write to the Free Software
.\" Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
.\"
.\"
.TH LIBFORTH 3 "VNPForth runtime library" "VNPForth" \" -*- nroff -*-
.SH NAME
.\"
libforth \- VNPForth runtime library functions
.\"
.\"
.\"
.SH SYNOPSIS
.\"
.B -lforth
.br
.B forthrt1.o
.PP
.\"
.\"
.\"
.SH DESCRIPTION
.\"
.PP
.B libforth.a
and
.B libforth.so
contain the runtime support functions for the VNPForth compiler.  Each
library contains implementations of the core Forth words, and also
some of the extended word set.  The static library may be used for
creating statically linked, pure Forth binaries.  The shared object
library may be used when the program is to be linked with ``crt1.o''
and the C runtime.  All library functions are weak symbols, allowing
them to be be overridden in user code.
.PP
.B forthrt1.o
contains a minimal process runtime that can be used to link a
completely self-contained, static Linux executable.  It defines the
symbol ``_start'', at which Linux executables begin running, and
makes a call to ``main'' after some minimal setup. \fBforthrt1.o\fP
may be used only with \fBlibforth.a\fP.  For mixed-language
programming with C or other languages, or to use the shared object
library of Forth words, other ``_start'' runtimes, such as ``crt1.o'',
must be used instead.
.PP
.\"
.\"
.\"
.SH LIBRARY FUNCTIONS
.\"
.B libforth
offers the following library variables:
.PP
include(forthvariables)
.PP
.B libforth
contains implementations of the following library functions.  Most of
these functions are standard Forth words, but the library also includes
some primitives used to implement basic VNPForth functions:
.PP
include(forthwords)
.PP
.B libforth
contains implementations of the following additional library functions.
These functions provide access to the Linux system calls, and enumerate
various system call error conditions:
.PP
include(extraforthwords)
.PP
.\"
.\"
.\"
.SH NOTES
VNPForth contains a sufficiently complete implementation of Forth to
pass most of the 1993 Johns Hopkins University / Applied Physics
Laboratory tests for core words of an ANS Forth system.  The only
words not implemented in VNPForth, but checked by the test, are those
associated with word dictionary lookups; because VNPForth is a native,
linked environment, it has no word dictionary.
.PP
By default, the libraries are compiled with debugging information
enabled.  This means that programs that use them can interact fully
with debugging tools such as gdb(1).  To remove the debugging information
from a binary, use strip(1).
.PP
If a program overruns or underruns one of its stacks, the runtime will
abort that program with SIGABRT.  This causes the program to dump core.
If the program was compiled with debug enabled, a debugger such as
gdb(1) can help to find the problem.
.PP
For linking with 'C', the include file libforth.h contains declarations
of all the VNPForth runtime library variables and functions.
.\"
.\"
.\"
.SH EXAMPLES
.\"
The following command lines create a statically linked ``hello world''
program:
.IP
.nf
forthc hello.ft
.fi
.br
.nf
ld -static -m elf_i386 -o hello hello.o \\
.br
    -lforth /usr/lib/forthrt1.o
.fi
.PP
The following lines create a dynamically linked program from the same
source file:
.IP
.nf
forthc hello.ft
.fi
.br
.nf
cc -o hello hello.o -lforth
.fi
.PP
.\"
.\"
.\"
.SH ERRORS AND OMISSIONS
.\"
Double numbers are an illusion.  They are simply implemented as single
numbers with zero in the most significant cell.
.PP
Several Forth words exist only for the purposes of passing ANS tests,
and have no real meaning in VNPForth.  There is no concept of
interpreting words in VNPForth, so ``['', ``]'', LITERAL, and
IMMEDIATE are null word definitions.  The value returned by HERE must
be explicitly set before ``,'', ``C,'' and ``ALIGN'' can be used.
.PP
ACCEPT is implemented inefficiently.  It uses KEY to obtain each input
character, resulting in more calls to the read() system call than is
desirable.
.PP
There are probably many more errors, and certainly many more omissions.
.PP
.\"
.\"
.\"
.SH FILES
.\"
/usr/lib/libforth.so, or /usr/local/lib/libforth.so
.br
/usr/lib/libforth.a, or /usr/local/lib/libforth.a
.br
/usr/lib/libforth.h, or /usr/local/lib/libforth.h
.br
/usr/lib/forthrt1.o, or /usr/local/lib/forthrt1.o
.\"
.\"
.\"
.SH SEE ALSO
.\"
Man page for \fBforthc\fP(1).
.\"
