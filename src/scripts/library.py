#
# Copyright (C) 2021 Michele Segata <segata@ccs-labs.org>
#
# Documentation for these modules is at http://veins.car2x.org/
#
# SPDX-License-Identifier: GPL-2.0-or-later
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

from os.path import join, relpath, isfile, dirname
from subprocess import check_output, DEVNULL, CalledProcessError
from optparse import OptionParser
import sys


class Library:
    def __init__(self, name, library, default_path, versions, source_folder,
                 lib_folder, images_folder, version_script, defs=[],
                 run_version_script=True, ned_folder=None):
        self.name = name
        self.library = library
        self.default_path = default_path
        self.versions = versions
        self.src_dir = source_folder
        self.lib_dir = lib_folder
        self.images_dir = images_folder
        self.version_script = version_script
        self.defs = defs
        self.run_version_script = run_version_script
        if ned_folder is None:
            self.ned_dir = self.src_dir
        else:
            self.ned_dir = ned_folder

    def run_version(self, path):
        fname = join(path, self.version_script)
        return check_output(["env", fname], stderr=DEVNULL).strip().decode()

    def read_version(self, path):
        fname = join(path, self.version_script)
        with open(fname, "r") as file:
            return file.read().rstrip()

    def add_to_parser(self, parser):
        help = "link with a version of {} installed in PATH [default %default]"
        help = help.format(self.name)
        parser.add_option("--with-{}".format(self.library), dest=self.library,
                          help=help, metavar="PATH", default=self.default_path)

    def print_unsupported_version(self, version):
        txt = "Unsupported {} Version. Expecting {}, found {}"
        txt = txt.format(self.name, " or ".join(self.versions), version)
        print(txt)

    def print_library_error(self):
        versions = " or ".join(self.versions)
        txt = "Could not determine {name} version (by running {script}): "\
              "Check the path to {name} (--with-{lib}=... option) "\
              "and the {name} version (should be version {versions})"
        txt = txt.format(name=self.name, script=self.version_script,
                         lib=self.library, versions=versions)
        print(txt)

    def print_file_error(self):
        versions = " or ".join(self.versions)
        txt = "Could not determine {name} version (by reading {script}): " \
              "Check the path to {name} (--with-{lib}=... option) " \
              "and the {name} version (should be version {versions})"
        txt = txt.format(name=self.name, script=self.version_script,
                         lib=self.library, versions=versions)
        print(txt)

    def print_invalid_version_files(self):
        scripts = ", ".join(self.version_script)
        txt = "None of the provided version files exist ({})".format(scripts)
        print(txt)

    def check(self, options, flags, libs, neds, imgs):
        if hasattr(options, self.library):
            path = getattr(options, self.library)
            name = self.name
            found = False
            if isinstance(self.version_script, list):
                for script in self.version_script:
                    fname = join(path, script)
                    if isfile(fname):
                        self.version_script = script
                        found = True
                        break
                if not found:
                    self.print_invalid_version_files()
                    sys.exit(1)
            try:
                print("Determining {} version.".format(name))
                if self.run_version_script:
                    version = self.run_version(path)
                else:
                    version = self.read_version(path)
                if version not in self.versions:
                    print("")
                    print("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
                    self.print_unsupported_version(version)
                    print("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!")
                    print("")
                else:
                    print("Found {} version {}. Okay.".format(name, version))

                    header_dirs = [
                        join(relpath(path, self.lib_dir), self.lib_dir)
                    ]
                    includes = ["-I" + s for s in header_dirs]
                    link = [
                        "-L" + join(relpath(path, self.lib_dir), self.lib_dir),
                        "-l{}$(D)".format(self.library)
                    ]
                    defs = self.defs
                    flags.extend(includes + link + defs)
                    lib = join(dirname(self.src_dir), self.library)
                    libs.append(relpath(join(path, lib)))
                    neds.append(relpath(join(path, self.ned_dir)))
                    imgs.append(relpath(join(path, self.images_dir)))

            except CalledProcessError:
                self.print_library_error()
                sys.exit(1)
            except FileNotFoundError:
                self.print_file_error()
                sys.exit(1)


class LibraryChecker:
    def __init__(self):
        self.parser = OptionParser()
        self.libs = []

    def add_lib(self, lib):
        lib.add_to_parser(self.parser)
        self.libs.append(lib)

    def check_libraries(self, flags, libs, neds, imgs):
        options, args = self.parser.parse_args()
        for lib in self.libs:
            lib.check(options, flags, libs, neds, imgs)

