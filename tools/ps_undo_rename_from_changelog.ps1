#..............................................................................
# If powershell script execution is disabled
# you may try running the script with: 
#  powershell -ExecutionPolicy Bypass -File ps_undo_rename_from_changelog.ps1
#..............................................................................
#
# MIT License
# 
# Copyright (c) 2021 Luigi Galli
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

$undolog = "UNDO_RENAME.LOG"
$clogpath = Read-Host 'Please enter path to changelog file: '
if (!(Test-Path $clogpath)) {
  "ERROR -> not found: $clogpath"
  exit
}

$clog = Get-Content -Path $clogpath
[array]::Reverse($clog)
$dtm = Get-Date
"START $dtm " | Tee-Object -Append -FilePath $undolog
#foreach ($line in $clog[($clog.Length)..1]) {
foreach ($line in $clog[0..($clog.Length-3)]) {
  $ord,$old,$new = $line.split("`t");
  $old = $old.Trim('"')
  $new = $new.Trim('"')
  if (Test-Path $new) {
   try { 
    "Renaming: $new to $old"  | Tee-Object -Append -FilePath $undolog
    Move-Item $new $old
   } catch {
    "ERROR -> could nor rename $new to $old" | Tee-Object -Append -FilePath $undolog
   }
  } else {
   "WARNING -> not found: [$ord] $new" | Tee-Object -Append -FilePath $undolog
  }
}
"DONE. See: $undolog" | Tee-Object -Append -FilePath $undolog