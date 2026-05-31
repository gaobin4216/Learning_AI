On Error Resume Next
Dim swApp
Set swApp = CreateObject("SldWorks.Application")
If Err.Number <> 0 Then
    WScript.Echo "Error: " & Err.Description
    WScript.Quit
End If
On Error GoTo 0

swApp.Visible = True

Dim tpl
tpl = swApp.GetUserPreferenceStringValue(8)
WScript.Echo "Default template: [" & tpl & "]"

Dim templatePaths
templatePaths = Array( _
    "C:\Software\sw2022\SOLIDWORKS\lang\english\Tutorial\part.prtdot", _
    "C:\Software\sw2022\SOLIDWORKS\lang\chinese-simplified\Tutorial\part.prtdot", _
    "C:\Software\sw2022\SOLIDWORKS\templates\Part.prtdot", _
    "C:\ProgramData\SolidWorks\SOLIDWORKS 2022\templates\Part.prtdot" _
)

Dim fso, t
Set fso = CreateObject("Scripting.FileSystemObject")

For Each t In templatePaths
    If fso.FileExists(t) Then
        WScript.Echo "EXISTS: " & t
    Else
        WScript.Echo "NOT FOUND: " & t
    End If
Next

' Try creating part with each template
WScript.Echo ""
WScript.Echo "Trying to create part..."
Dim model

For Each t In templatePaths
    If fso.FileExists(t) Then
        WScript.Echo "Trying: " & t
        Err.Clear
        Set model = swApp.NewDocument(t, 0, 0, 0)
        If Err.Number = 0 And Not model Is Nothing Then
            WScript.Echo "SUCCESS with: " & t
            Exit For
        Else
            WScript.Echo "FAILED: " & Err.Description
        End If
    End If
Next

If model Is Nothing Then
    WScript.Echo "All templates failed. Trying swApp.NewDocument2..."
    For Each t In templatePaths
        If fso.FileExists(t) Then
            WScript.Echo "Trying NewDocument2: " & t
            Err.Clear
            Set model = swApp.NewDocument2(t, 0, 0, 0)
            If Err.Number = 0 And Not model Is Nothing Then
                WScript.Echo "SUCCESS with NewDocument2: " & t
                Exit For
            Else
                WScript.Echo "FAILED: " & Err.Description
            End If
        End If
    Next
End If

If model Is Nothing Then
    WScript.Echo "All attempts failed."
Else
    WScript.Echo "Part created: " & model.GetTitle
End If
