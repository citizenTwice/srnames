# srnames
Multi-platform search+replace file & directory rename utility.

##### Table of Contents  
[Headers](#features)  
[Supported platforms](#platforms)  
[Usage](#usage)  

## Features
 - Can rename both files and directories, recursively if needed.
 - Regex support
 - Variables in replacement expressions
 - Preview actions without making changes.
 - Create changes log, for undo (provided by separate scripts.)
 - Unicode support
 - Portable, stand-alone executable.
 - Builds without dependencies other than the platform's compiler/SDK.

## Supported platforms
 - macOS
 - Linux
 - Windows (ASCII & Unicode builds)
 - DOS/Freedos w.DPMI [experimental, some features are not available]
 
### Syntax
```
srnames - search and replace file/dir names.
  Required:
          --search=str1              String to search to for.
          --repl=str2                Replacement string.
  Options:
      -i, --ignore-case              Ignore case in searches.
      -d, --dirs-too                 Also rename directories (default is files-only.)
      -a, --start-at=path            Start searching at given path.
                                     Default is: current directory.
      -r, --recurse                  Recursively include all directories.
          --regex                    Interpret search string as regex.
                                     Makes capture groups available in replacement string.
          --what-if                  Preview results without making any changes.
          --dry-run                  Same as --what-if.
      -q, --quiet                    Do not print changes to stdout.
      -y, --yes                      Skip initial confirmation prompt.
          --chglog=filepath          Log changes (enables undo).
                                     On by default (temp file) for some platforms.
      -n, --no-chglog                Disable changes log.
  Exit codes:                        0 = Success.
                                     1 = Error.
  Regex replacement variables:       $1..n = Capture group n. E.g. 'file_(\d+)'
                                     $&    = The entire regex match
  Repl. string variables:            (applied after regex, when that's available)
                                     #dir  = Parent dir of current item.
                                     #dtm  = YYYYMMDDhhmmss.
                                     #cnt  = Rename counter.
                                     #2cnt = ditto, zero-filled NN.
                                     #3cnt = ditto, zero-filled NNN.
                                     #4cnt = ditto, zero-filled NNNN.
                                     #8cnt = ditto, zero-filled NNNNNNNN.
                                     ##    = literal #.

```
 
### Examples
#### regex search & replace
Prefix images with parent dir name, preserving image number, so files can be merged into a single directory
```
 # DO notice options have been 'quoted' to prevent the shell from corrputing the passed parameters
 % find .
./THUMB
./THUMB/IMG0397.JPG
./THUMB/IMG0437.JPG
./THUMB/IMG0450.JPG
./THUMB/IMG0402.JPG
./img
./PIC/IMG0397.JPG
./PIC/IMG0437.JPG
./PIC/IMG0450.JPG
./PIC/IMG0402.JPG
 % srnames -r --regex '--search=IMG(\d\d\d\d)' '--repl=#dir_$1' -q -y && 
  mv PIC/* img/ &&
  mv THUMB/* img/
 % find . 
./PIC
./THUMB
./img
./img/PIC_0397.JPG
./img/PIC_0402.JPG
./img/PIC_0437.JPG
./img/PIC_0450.JPG
./img/THUMB_0397.JPG
./img/THUMB_0402.JPG
./img/THUMB_0437.JPG
./img/THUMB_0450.JPG
```

#### Rolling-back changes
Creating a changelog is recommended as it makes it possible to undo/'unrname' what was changed. 
The changelog is created by default on some platforms and can be specified via command-line with the --chglog option.
The below scripts (which can be found in the tools directory) can use the changelog to roll-back changes.
##### Mac/Linux
```
./tools/make_undo_script.sh CHANGES_TO_LINUX_MASTER.LST
bash UNDO_SCRIPT_95613.sh
Done!
```
##### Windows
```
powershell -ExecutionPolicy Bypass -File tools\ps_undo_rename_from_changelog.ps1
Please enter path to changelog file: CHANGES.LOG
Done!
```
 
### Demos
undo_demo.mov
![test](https://user-images.githubusercontent.com/20754288/120099884-2f433980-c13e-11eb-9964-cda76bf26d3c.mov)


### Building
#### macOS
- requirements: recent (as of 2021) XCode version
- build command ` build_macos.sh `
#### Linux and other *ix
- requirements: recent (as of 2021) GCC C++ version
- build command ` build_unix.sh `
#### Windows
- requirements: Visual C++ from VS2019 or newer
- build command ` build_vc2019.cmd `
#### DOS
- requirements: recent (as of 2021) OpenWatcom C/C++
- build command ` BUILDOS.BAT `


### License
MIT

### Feedback/Contact
* Email: [LG@citizentwice.nl](mailto:LG@citizentwice.nl)
Thanks for visting.

