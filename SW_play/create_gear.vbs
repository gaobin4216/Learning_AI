Option Explicit

Dim swApp, model, sketchMgr, swFeat
Dim z, m_val, faceWidth, boreDia
Dim pitchR, outerR, rootR
Dim toothAngle, ca, pi
Dim i, template

pi = 3.14159265358979

z = 20
m_val = 3
faceWidth = 20
boreDia = 20

pitchR = m_val * z / 2
outerR = pitchR + m_val
rootR = pitchR - 1.25 * m_val
toothAngle = 2 * pi / z

WScript.Echo "Gear: z=" & z & " m=" & m_val & " OD=" & (outerR * 2) & " RootD=" & (rootR * 2)

Set swApp = CreateObject("SldWorks.Application")
swApp.Visible = True
WScript.Sleep 500

template = "C:\Software\sw2022\SOLIDWORKS\lang\english\Tutorial\part.prtdot"
Set model = swApp.NewDocument(template, 0, 0, 0)
If model Is Nothing Then
    WScript.Echo "ERROR: Cannot create part"
    WScript.Quit
End If
WScript.Echo "Part created"

Set sketchMgr = model.SketchManager

' === Draw Complete Gear Profile + Bore in One Sketch ===
WScript.Echo "Drawing gear profile..."

model.Extension.SelectByID2 "Front Plane", "PLANE", 0, 0, 0, False, 0, Nothing, 0
sketchMgr.InsertSketch True

' Outer gear profile: for each tooth, draw 4 lines
Dim tipL_x, tipL_y, tipR_x, tipR_y
Dim gapL_x, gapL_y, gapR_x, gapR_y
Dim nextTipL_x, nextTipL_y

For i = 0 To z - 1
    ca = i * toothAngle

    ' Tip edge (outer circle)
    tipL_x = outerR * Cos(ca - toothAngle * 0.2)
    tipL_y = outerR * Sin(ca - toothAngle * 0.2)
    tipR_x = outerR * Cos(ca + toothAngle * 0.2)
    tipR_y = outerR * Sin(ca + toothAngle * 0.2)

    ' Gap edge (root circle)
    gapL_x = rootR * Cos(ca + toothAngle * 0.3)
    gapL_y = rootR * Sin(ca + toothAngle * 0.3)
    gapR_x = rootR * Cos(ca + toothAngle * 0.7)
    gapR_y = rootR * Sin(ca + toothAngle * 0.7)

    ' Next tooth tip-left
    nextTipL_x = outerR * Cos((i + 1) * toothAngle - toothAngle * 0.2)
    nextTipL_y = outerR * Sin((i + 1) * toothAngle - toothAngle * 0.2)

    ' Tooth top
    sketchMgr.CreateLine tipL_x, tipL_y, 0, tipR_x, tipR_y, 0
    ' Right flank down
    sketchMgr.CreateLine tipR_x, tipR_y, 0, gapL_x, gapL_y, 0
    ' Root
    sketchMgr.CreateLine gapL_x, gapL_y, 0, gapR_x, gapR_y, 0
    ' Left flank up (to next tooth)
    sketchMgr.CreateLine gapR_x, gapR_y, 0, nextTipL_x, nextTipL_y, 0
Next

' Center bore (inner circle - creates hole when extruded)
sketchMgr.CreateCircleByRadius 0, 0, 0, boreDia / 2

sketchMgr.InsertSketch True
WScript.Echo "Profile done"

' === Extrude ===
WScript.Echo "Extruding..."
On Error Resume Next
Set swFeat = model.FeatureManager.FeatureExtrusion(True, False, False, 0, 0, faceWidth, 0, False, False, 0, 0, 0, 0, False, False, False, False, True, True, True)
If Err.Number <> 0 Then
    WScript.Echo "Extrude error: " & Err.Description & " (0x" & Hex(Err.Number) & ")"
    Err.Clear
End If
On Error GoTo 0

If swFeat Is Nothing Then
    WScript.Echo "ERROR: Extrusion failed"
    WScript.Quit
End If

model.ViewZoomtofit2
model.ForceRebuild3 True

WScript.Echo "=== GEAR COMPLETE ==="
WScript.Echo "OD=" & (outerR * 2) & "mm RootD=" & (rootR * 2) & "mm Bore=" & boreDia & "mm"
