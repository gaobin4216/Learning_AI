Add-Type -AssemblyName System.Drawing

$width = 1024
$height = 1024
$outPath = Join-Path (Get-Location) "robot_battle.png"

$bmp = New-Object System.Drawing.Bitmap $width, $height
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
$g.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::AntiAlias

function Brush-FromArgb($a, $r, $gr, $b) {
    return New-Object System.Drawing.SolidBrush ([System.Drawing.Color]::FromArgb($a, $r, $gr, $b))
}

function Pen-FromArgb($a, $r, $gr, $b, $w) {
    return New-Object System.Drawing.Pen ([System.Drawing.Color]::FromArgb($a, $r, $gr, $b)), $w
}

function Fill-RoundRect($graphics, $brush, $x, $y, $w, $h, $radius) {
    $path = New-Object System.Drawing.Drawing2D.GraphicsPath
    $d = $radius * 2
    $path.AddArc($x, $y, $d, $d, 180, 90)
    $path.AddArc($x + $w - $d, $y, $d, $d, 270, 90)
    $path.AddArc($x + $w - $d, $y + $h - $d, $d, $d, 0, 90)
    $path.AddArc($x, $y + $h - $d, $d, $d, 90, 90)
    $path.CloseFigure()
    $graphics.FillPath($brush, $path)
    $path.Dispose()
}

function Stroke-RoundRect($graphics, $pen, $x, $y, $w, $h, $radius) {
    $path = New-Object System.Drawing.Drawing2D.GraphicsPath
    $d = $radius * 2
    $path.AddArc($x, $y, $d, $d, 180, 90)
    $path.AddArc($x + $w - $d, $y, $d, $d, 270, 90)
    $path.AddArc($x + $w - $d, $y + $h - $d, $d, $d, 0, 90)
    $path.AddArc($x, $y + $h - $d, $d, $d, 90, 90)
    $path.CloseFigure()
    $graphics.DrawPath($pen, $path)
    $path.Dispose()
}

function Draw-Laser($graphics, $x1, $y1, $x2, $y2, $color) {
    $glow = New-Object System.Drawing.Pen ([System.Drawing.Color]::FromArgb(70, $color.R, $color.G, $color.B)), 18
    $core = New-Object System.Drawing.Pen ([System.Drawing.Color]::FromArgb(255, 255, 255, 255)), 4
    $beam = New-Object System.Drawing.Pen $color, 8
    $glow.StartCap = [System.Drawing.Drawing2D.LineCap]::Round
    $glow.EndCap = [System.Drawing.Drawing2D.LineCap]::Round
    $beam.StartCap = [System.Drawing.Drawing2D.LineCap]::Round
    $beam.EndCap = [System.Drawing.Drawing2D.LineCap]::Round
    $core.StartCap = [System.Drawing.Drawing2D.LineCap]::Round
    $core.EndCap = [System.Drawing.Drawing2D.LineCap]::Round
    $graphics.DrawLine($glow, $x1, $y1, $x2, $y2)
    $graphics.DrawLine($beam, $x1, $y1, $x2, $y2)
    $graphics.DrawLine($core, $x1, $y1, $x2, $y2)
    $glow.Dispose()
    $beam.Dispose()
    $core.Dispose()
}

function Draw-Explosion($graphics, $cx, $cy, $scale) {
    $orange = Brush-FromArgb 230 255 116 28
    $yellow = Brush-FromArgb 240 255 220 78
    $red = Brush-FromArgb 180 190 36 28
    $points = @()
    for ($i = 0; $i -lt 18; $i++) {
        $angle = (2 * [Math]::PI * $i / 18) - 0.18
        $r = if ($i % 2 -eq 0) { 88 * $scale } else { 38 * $scale }
        $points += New-Object System.Drawing.PointF (($cx + [Math]::Cos($angle) * $r), ($cy + [Math]::Sin($angle) * $r))
    }
    $graphics.FillPolygon($red, [System.Drawing.PointF[]]$points)
    $graphics.FillEllipse($orange, $cx - 48 * $scale, $cy - 42 * $scale, 96 * $scale, 84 * $scale)
    $graphics.FillEllipse($yellow, $cx - 24 * $scale, $cy - 20 * $scale, 48 * $scale, 40 * $scale)
    $orange.Dispose()
    $yellow.Dispose()
    $red.Dispose()
}

function Draw-Robot($graphics, $cx, $baseY, $scale, $flip, $accent) {
    $metal = Brush-FromArgb 255 104 119 132
    $dark = Brush-FromArgb 255 43 49 58
    $mid = Brush-FromArgb 255 157 170 179
    $shadow = Brush-FromArgb 120 0 0 0
    $accentBrush = New-Object System.Drawing.SolidBrush $accent
    $outline = New-Object System.Drawing.Pen ([System.Drawing.Color]::FromArgb(255, 24, 29, 35)), (4 * $scale)
    $glowPen = New-Object System.Drawing.Pen ([System.Drawing.Color]::FromArgb(160, $accent.R, $accent.G, $accent.B)), (5 * $scale)

    $dir = if ($flip) { -1 } else { 1 }
    $graphics.FillEllipse($shadow, $cx - 95 * $scale, $baseY - 20 * $scale, 190 * $scale, 36 * $scale)

    $legW = 34 * $scale
    $graphics.FillRectangle($dark, $cx - 55 * $scale, $baseY - 155 * $scale, $legW, 125 * $scale)
    $graphics.FillRectangle($dark, $cx + 22 * $scale, $baseY - 155 * $scale, $legW, 125 * $scale)
    $graphics.FillRectangle($metal, $cx - 72 * $scale, $baseY - 38 * $scale, 70 * $scale, 25 * $scale)
    $graphics.FillRectangle($metal, $cx + 12 * $scale, $baseY - 38 * $scale, 70 * $scale, 25 * $scale)

    Fill-RoundRect $graphics $metal ($cx - 88 * $scale) ($baseY - 345 * $scale) (176 * $scale) (180 * $scale) (26 * $scale)
    Stroke-RoundRect $graphics $outline ($cx - 88 * $scale) ($baseY - 345 * $scale) (176 * $scale) (180 * $scale) (26 * $scale)
    Fill-RoundRect $graphics $mid ($cx - 55 * $scale) ($baseY - 305 * $scale) (110 * $scale) (62 * $scale) (14 * $scale)
    $graphics.FillEllipse($accentBrush, $cx - 18 * $scale, $baseY - 230 * $scale, 36 * $scale, 36 * $scale)

    Fill-RoundRect $graphics $metal ($cx - 70 * $scale) ($baseY - 445 * $scale) (140 * $scale) (88 * $scale) (22 * $scale)
    Stroke-RoundRect $graphics $outline ($cx - 70 * $scale) ($baseY - 445 * $scale) (140 * $scale) (88 * $scale) (22 * $scale)
    $graphics.FillRectangle($dark, $cx - 50 * $scale, $baseY - 418 * $scale, 100 * $scale, 25 * $scale)
    $graphics.FillEllipse($accentBrush, $cx - 38 * $scale, $baseY - 415 * $scale, 20 * $scale, 20 * $scale)
    $graphics.FillEllipse($accentBrush, $cx + 18 * $scale, $baseY - 415 * $scale, 20 * $scale, 20 * $scale)
    $graphics.DrawLine($outline, $cx, $baseY - 445 * $scale, $cx, $baseY - 488 * $scale)
    $graphics.FillEllipse($accentBrush, $cx - 8 * $scale, $baseY - 503 * $scale, 16 * $scale, 16 * $scale)

    $shoulderY = $baseY - 320 * $scale
    $gunX = $cx + $dir * 178 * $scale
    $handX = $cx + $dir * 93 * $scale
    $graphics.DrawLine($outline, $cx + $dir * 82 * $scale, $shoulderY, $handX, $baseY - 265 * $scale)
    $graphics.DrawLine($glowPen, $cx + $dir * 82 * $scale, $shoulderY, $handX, $baseY - 265 * $scale)
    Fill-RoundRect $graphics $dark ($handX - 18 * $scale) ($baseY - 288 * $scale) (110 * $scale) (38 * $scale) (10 * $scale)
    $graphics.FillRectangle($dark, $gunX - 16 * $scale, $baseY - 280 * $scale, 64 * $scale, 20 * $scale)
    $graphics.FillEllipse($accentBrush, $gunX + 40 * $scale, $baseY - 280 * $scale, 18 * $scale, 20 * $scale)

    $otherX = $cx - $dir * 96 * $scale
    $graphics.DrawLine($outline, $cx - $dir * 82 * $scale, $shoulderY + 5 * $scale, $otherX, $baseY - 230 * $scale)
    $graphics.DrawLine($glowPen, $cx - $dir * 82 * $scale, $shoulderY + 5 * $scale, $otherX, $baseY - 230 * $scale)
    $graphics.FillEllipse($metal, $otherX - 22 * $scale, $baseY - 247 * $scale, 44 * $scale, 44 * $scale)

    $metal.Dispose()
    $dark.Dispose()
    $mid.Dispose()
    $shadow.Dispose()
    $accentBrush.Dispose()
    $outline.Dispose()
    $glowPen.Dispose()
}

$skyRect = New-Object System.Drawing.Rectangle 0, 0, $width, $height
$sky = New-Object System.Drawing.Drawing2D.LinearGradientBrush $skyRect, ([System.Drawing.Color]::FromArgb(255, 25, 28, 43)), ([System.Drawing.Color]::FromArgb(255, 88, 74, 58)), 90
$g.FillRectangle($sky, $skyRect)
$sky.Dispose()

$haze = Brush-FromArgb 45 255 176 92
$g.FillEllipse($haze, -120, 90, 1260, 620)
$haze.Dispose()

$stars = Brush-FromArgb 145 230 241 255
for ($i = 0; $i -lt 90; $i++) {
    $x = (Get-Random -Minimum 0 -Maximum $width)
    $y = (Get-Random -Minimum 0 -Maximum 420)
    $s = (Get-Random -Minimum 1 -Maximum 4)
    $g.FillEllipse($stars, $x, $y, $s, $s)
}
$stars.Dispose()

$ruinBrush = Brush-FromArgb 255 38 42 48
$ruinPen = New-Object System.Drawing.Pen ([System.Drawing.Color]::FromArgb(255, 65, 70, 78)), 3
$g.FillRectangle($ruinBrush, 66, 500, 78, 315)
$g.FillRectangle($ruinBrush, 170, 570, 96, 245)
$g.FillRectangle($ruinBrush, 770, 520, 90, 300)
$g.FillRectangle($ruinBrush, 890, 605, 64, 215)
for ($x = 83; $x -lt 140; $x += 26) { for ($y = 526; $y -lt 735; $y += 48) { $g.DrawRectangle($ruinPen, $x, $y, 15, 22) } }
for ($x = 790; $x -lt 850; $x += 28) { for ($y = 545; $y -lt 745; $y += 50) { $g.DrawRectangle($ruinPen, $x, $y, 16, 23) } }
$ruinBrush.Dispose()
$ruinPen.Dispose()

$groundRect = New-Object System.Drawing.Rectangle 0, 742, $width, 282
$ground = New-Object System.Drawing.Drawing2D.LinearGradientBrush $groundRect, ([System.Drawing.Color]::FromArgb(255, 52, 48, 43)), ([System.Drawing.Color]::FromArgb(255, 21, 22, 25)), 90
$g.FillRectangle($ground, $groundRect)
$ground.Dispose()

$crackPen = New-Object System.Drawing.Pen ([System.Drawing.Color]::FromArgb(160, 13, 15, 18)), 5
$g.DrawLine($crackPen, 0, 820, 180, 785)
$g.DrawLine($crackPen, 180, 785, 320, 845)
$g.DrawLine($crackPen, 560, 815, 750, 780)
$g.DrawLine($crackPen, 750, 780, 1024, 835)
$g.DrawLine($crackPen, 250, 955, 410, 890)
$g.DrawLine($crackPen, 625, 930, 830, 900)
$crackPen.Dispose()

Draw-Explosion $g 512 680 1.45
Draw-Explosion $g 735 615 0.72
Draw-Explosion $g 300 630 0.62

Draw-Laser $g 300 545 500 657 ([System.Drawing.Color]::FromArgb(255, 30, 230, 255))
Draw-Laser $g 723 548 534 660 ([System.Drawing.Color]::FromArgb(255, 255, 60, 70))
Draw-Laser $g 230 402 780 496 ([System.Drawing.Color]::FromArgb(255, 140, 255, 85))

Draw-Robot $g 255 865 1.0 $false ([System.Drawing.Color]::FromArgb(255, 48, 224, 255))
Draw-Robot $g 752 862 1.0 $true ([System.Drawing.Color]::FromArgb(255, 255, 76, 86))

$smokeBrush = Brush-FromArgb 80 190 190 185
$smokeBrush2 = Brush-FromArgb 52 87 92 100
$g.FillEllipse($smokeBrush2, 385, 360, 230, 110)
$g.FillEllipse($smokeBrush, 438, 330, 220, 105)
$g.FillEllipse($smokeBrush2, 585, 395, 260, 120)
$g.FillEllipse($smokeBrush, 160, 455, 250, 115)
$smokeBrush.Dispose()
$smokeBrush2.Dispose()

$vignettePath = New-Object System.Drawing.Drawing2D.GraphicsPath
$vignettePath.AddEllipse(-190, -160, 1404, 1360)
$vignette = New-Object System.Drawing.Drawing2D.PathGradientBrush $vignettePath
$vignette.CenterColor = [System.Drawing.Color]::FromArgb(0, 0, 0, 0)
$vignette.SurroundColors = @([System.Drawing.Color]::FromArgb(145, 0, 0, 0))
$g.FillRectangle($vignette, 0, 0, $width, $height)
$vignette.Dispose()
$vignettePath.Dispose()

$bmp.Save($outPath, [System.Drawing.Imaging.ImageFormat]::Png)
$g.Dispose()
$bmp.Dispose()

Write-Output $outPath
