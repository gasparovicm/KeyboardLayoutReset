# Draws the app icon: a white keyboard on a blue key, with a "reset" arrow
# badge. Writes src\app.ico and the MSIX logos in packaging\Assets. Each size
# is rendered separately so small sizes stay sharp. Glyphs come from the Segoe
# Fluent Icons font (Windows 11).
Add-Type -AssemblyName System.Drawing

$sizes = 16, 20, 24, 32, 40, 48, 64, 256
$out = Join-Path $PSScriptRoot '..\src\app.ico'
$blue = [System.Drawing.Color]::FromArgb(255, 37, 99, 235)
$iconFont = 'Segoe Fluent Icons'
$keyboard = [string][char]0xE765
$reset = [string][char]0xE72C

function Draw-Glyph($g, $glyph, $px, $color, $x, $y, $w, $h) {
    $font = New-Object System.Drawing.Font $iconFont, $px, ([System.Drawing.FontStyle]::Regular), ([System.Drawing.GraphicsUnit]::Pixel)
    $fmt = New-Object System.Drawing.StringFormat
    $fmt.Alignment = 'Center'
    $fmt.LineAlignment = 'Center'
    $brush = New-Object System.Drawing.SolidBrush $color
    $g.DrawString($glyph, $font, $brush, (New-Object System.Drawing.RectangleF $x, $y, $w, $h), $fmt)
}

function Render-Icon($s) {
    $bmp = New-Object System.Drawing.Bitmap $s, $s
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.SmoothingMode = 'AntiAlias'
    $g.TextRenderingHint = 'AntiAliasGridFit'
    $g.Clear([System.Drawing.Color]::Transparent)

    # Rounded square key cap.
    $pad = [Math]::Max(0.5, $s * 0.04)
    $r = $s * 0.22
    $w = $s - 2 * $pad
    $path = New-Object System.Drawing.Drawing2D.GraphicsPath
    $path.AddArc($pad, $pad, 2 * $r, 2 * $r, 180, 90)
    $path.AddArc($pad + $w - 2 * $r, $pad, 2 * $r, 2 * $r, 270, 90)
    $path.AddArc($pad + $w - 2 * $r, $pad + $w - 2 * $r, 2 * $r, 2 * $r, 0, 90)
    $path.AddArc($pad, $pad + $w - 2 * $r, 2 * $r, 2 * $r, 90, 90)
    $path.CloseFigure()
    $g.FillPath((New-Object System.Drawing.SolidBrush $blue), $path)

    # Keyboard, nudged up-left to leave room for the badge.
    Draw-Glyph $g $keyboard ($s * 0.56) ([System.Drawing.Color]::White) 0 (-$s * 0.08) ($s * 0.9) $s

    # Reset badge: white circle with a blue circular arrow, bottom-right.
    $d = $s * 0.52
    $g.FillEllipse([System.Drawing.Brushes]::White, $s - $d, $s - $d, $d, $d)
    Draw-Glyph $g $reset ($d * 0.62) $blue ($s - $d) ($s - $d + $d * 0.03) $d $d

    $g.Dispose()
    $bmp
}

$pngs = @()
foreach ($s in $sizes) {
    $bmp = Render-Icon $s
    $ms = New-Object System.IO.MemoryStream
    $bmp.Save($ms, [System.Drawing.Imaging.ImageFormat]::Png)
    $pngs += , $ms.ToArray()
    $bmp.Dispose()
}

# ICO container with PNG-compressed entries (supported since Windows Vista).
$fs = [System.IO.File]::Create($out)
$bw = New-Object System.IO.BinaryWriter $fs
$bw.Write([UInt16]0); $bw.Write([UInt16]1); $bw.Write([UInt16]$sizes.Count)
$offset = 6 + 16 * $sizes.Count
for ($i = 0; $i -lt $sizes.Count; $i++) {
    $s = $sizes[$i]; $dim = if ($s -ge 256) { 0 } else { $s }
    $bw.Write([Byte]$dim); $bw.Write([Byte]$dim); $bw.Write([Byte]0); $bw.Write([Byte]0)
    $bw.Write([UInt16]1); $bw.Write([UInt16]32)
    $bw.Write([UInt32]$pngs[$i].Length); $bw.Write([UInt32]$offset)
    $offset += $pngs[$i].Length
}
foreach ($p in $pngs) { $bw.Write($p) }
$bw.Close()
Write-Host "Wrote $out"

# MSIX logos (names referenced by packaging\AppxManifest.xml).
$assets = Join-Path $PSScriptRoot '..\packaging\Assets'
New-Item -ItemType Directory -Force $assets | Out-Null
foreach ($logo in @(@('StoreLogo', 50), @('Square44x44Logo', 44), @('Square150x150Logo', 150))) {
    $bmp = Render-Icon $logo[1]
    $bmp.Save((Join-Path $assets ($logo[0] + '.png')), [System.Drawing.Imaging.ImageFormat]::Png)
    $bmp.Dispose()
}
Write-Host "Wrote $assets"

# Store listing icons (uploaded in Partner Center under Store listings > Store
# logos, not part of the package): 300x300, 150x150 and 71x71.
$art = Join-Path $PSScriptRoot '..\packaging\StoreArt'
New-Item -ItemType Directory -Force $art | Out-Null
foreach ($s in 300, 150, 71) {
    $bmp = Render-Icon $s
    $bmp.Save((Join-Path $art "StoreIcon-${s}x${s}.png"), [System.Drawing.Imaging.ImageFormat]::Png)
    $bmp.Dispose()
}
Write-Host "Wrote $art"
