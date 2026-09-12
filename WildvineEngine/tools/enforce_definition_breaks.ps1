param(
  [string]$Root = (Split-Path -Parent $PSScriptRoot)
)

$ErrorActionPreference = "Stop"
$utf8NoBom = New-Object System.Text.UTF8Encoding($false)
$extensions = @("*.h", "*.hpp", "*.cpp")
$excludedNames = @("stb_image.h", "cgltf.h", "OBJ_Loader.h")
$files = foreach ($extension in $extensions) {
  Get-ChildItem -LiteralPath "$Root\include", "$Root\source" `
    -Recurse -File -Filter $extension
}

$files = $files | Where-Object {
  $_.FullName -notmatch "[\\/]fbx[\\/]" -and
  $_.Name -notin $excludedNames
}

$changed = 0
foreach ($file in $files) {
  $content = [System.IO.File]::ReadAllText($file.FullName)
  $newline = if ($content.Contains("`r`n")) { "`r`n" } else { "`n" }

  # Repair enum-class declarations produced by older documentation formatting.
  $enumPattern =
    '(?m)^([ \t]*)enum\s*\r?\n\s*/\*\*[^\r\n]*\*/\s*\r?\n\s*class\s+' +
    '([A-Za-z_][A-Za-z0-9_]*)\s*\{'
  $content = [regex]::Replace(
    $content,
    $enumPattern,
    { param($match)
      $match.Groups[1].Value + "enum class" + $newline +
        $match.Groups[1].Value + $match.Groups[2].Value + " {"
    }
  )

  # Project rule: class/struct keyword and defined type name use separate lines.
  $definitionPattern =
    '(?m)^([ \t]*)(class|struct)[ \t]+' +
    '([A-Za-z_][A-Za-z0-9_]*)([ \t]*(?::[^\r\n]*)?\{[^\r\n]*)$'
  $updated = [regex]::Replace(
    $content,
    $definitionPattern,
    { param($match)
      $match.Groups[1].Value + $match.Groups[2].Value + $newline +
        $match.Groups[1].Value + $match.Groups[3].Value +
        $match.Groups[4].Value
    }
  )

  if ($updated -ne [System.IO.File]::ReadAllText($file.FullName)) {
    [System.IO.File]::WriteAllText($file.FullName, $updated, $utf8NoBom)
    ++$changed
  }
}

$violations = Select-String -Path ($files.FullName) `
  -Pattern '^\s*(class|struct)\s+[A-Za-z_][A-Za-z0-9_]*(\s*[:{])'
if ($violations) {
  $violations | ForEach-Object {
    Write-Error "$($_.Path):$($_.LineNumber): $($_.Line)"
  }
  exit 1
}

Write-Output "Files updated: $changed"
Write-Output "Class/struct definition violations: 0"
