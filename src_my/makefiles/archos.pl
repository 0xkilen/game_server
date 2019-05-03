#!/usr/bin/perl -w

# old-style ARCH_OS generator to support nightly build and test systems

my $uname = $ARGV[0];

my $arch;
my $os = `$uname -s`;

# if no uname -- revert to old behavior
if ($? != 0) {
    use Config;
    my $arch_os = $Config{archname};

    # normalize ?86 architecture format
    $arch_os =~ s/[ix]86(pc)?|i[3456]86|athlon/i86pc/;
    $arch_os =~ s/i86pc_64/x86_64pc/;

    # normalize os names
    $arch_os =~ s/Linux/linux/;
    $arch_os =~ s/MSWin32/win32/;

    $arch_os =~ s/-thread//;
    $arch_os =~ s/-multi//;

   	# On win32, the string is reversed
    if ($arch_os =~ /win32/) {
        ($os, $arch) = split(/-/, $arch_os);
    }
    else {
        ($arch, $os) = split(/-/, $arch_os);
    }
}

# uname is present, so use it
else {
    chomp($os);

    #  CYGWIN_NT-5.2-WOW64
    #  CYGWIN_NT-5.0
    
    $os =~ s/Linux/linux/;
    $os =~ s/WindowsNT/win32/;
    $os =~ s/CYGWIN.*/win32/;
    $os =~ s/MINGW32.*/win32/;

    if ($os eq 'win32') {
        $arch = $ENV{PROCESSOR_ARCHITEW6432};
        $arch = $ENV{PROCESSOR_ARCHITECTURE} if !defined($arch);
        if ($arch =~ /64/) {
            $arch =~ s/AMD/x86_/;
        }
        else {
            $arch =~ s/x86/i86/
        }
    }
    else {
        $arch = `$uname -p`;
        $arch = `$uname -m` if ($arch =~ /unknown/);
        $arch =~ s/i[3456]86|athlon/i86/;
    }
    chomp($arch);
    $arch .= "pc" if ($arch =~ /[ix]86/);
}

print("$arch-$os\n");
