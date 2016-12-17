#!/usr/bin/python

from subprocess import Popen, PIPE
import sys
import os

blacklist = [
    # The following files are for non-x86 targets
    # they cause false positives when looking up symbols
    'qemu/target-alpha/',
    'qemu/target-arm/',
    'qemu/target-cris/',
    'qemu/target-m68k/',
    'qemu/target-microblaze/',
    'qemu/target-mips/',
    'qemu/target-ppc/',
    'qemu/target-s390x/',
    'qemu/target-sh4/',
    'qemu/target-sparc/',
    'qemu/darwin-user/',
    'qemu/bsd-user/',
    'qemu/pc-bios/',
    'qemu/roms/',
    'qemu/tests/',
    'qemu/tcg/arm/',
    'qemu/tcg/hppa/',
    'qemu/tcg/i386/',
    'qemu/tcg/mips/',
    'qemu/tcg/ppc/',
    'qemu/tcg/ppc64/',
    'qemu/tcg/s390/',
    'qemu/tcg/sparc/',
]

if os.path.isdir('.git'):
    git_files = Popen(['git', 'ls-files'],
                  stdout=PIPE).communicate()[0].split('\n')
else:
    sys.stderr.write('This is not a git repository!\n')
    sys.exit(1)

git_files.sort()

dirs = set([""])
qemu_files = open('qemu.files', 'w')
qemu_includes = open('qemu.includes', 'w')
for fname in git_files:
    for b in blacklist:
        if fname.startswith(b):
            break
    else:
        if not os.path.isdir(fname):
            qemu_files.write(fname + '\n')

            fdir = fname
            while fdir != "":
                fdir = os.path.dirname(fdir)
                if fdir not in dirs and os.path.isdir(fdir):
                    qemu_includes.write(fdir + '\n')
                    dirs.add(fdir)

qemu_includes.write('\n'.join([
]))

qemu_files.close()
qemu_includes.close()

with open('qemu.creator', 'w') as fp:
    print >>fp, "[General]"

