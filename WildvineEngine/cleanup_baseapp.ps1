$lines = Get-Content "Include\BaseApp.h"
# Find all }; positions
$closes = @()
for ($i = 0; $i -lt $lines.Count; $i++) {
    if ($lines[$i].TrimEnd() -eq "};") { $closes += $i }
}
Write-Host "Found }; at lines: $($closes -join ', ')"
# The documented class closes at the one right before the duplicate block (which starts with #include)
# Find first #include after the class declaration area
$dupStart = -1
for ($i = 50; $i -lt $lines.Count; $i++) {
    if ($lines[$i] -match '^#include') { $dupStart = $i; break }
}
Write-Host "Duplicate block starts at line: $dupStart"
if ($dupStart -gt 0) {
    $cutLine = $dupStart - 1
    $lines[0..$cutLine] | Set-Content "Include\BaseApp.h"
    Write-Host "Saved $(($cutLine+1)) lines."
}
