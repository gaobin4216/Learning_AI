Dim swApp
On Error Resume Next
Set swApp = CreateObject("SldWorks.Application")
If Err.Number <> 0 Then
    WScript.Echo "CreateObject error: 0x" & Hex(Err.Number) & " - " & Err.Description
    Err.Clear

    WScript.Echo "Trying GetObject..."
    Set swApp = GetObject(, "SldWorks.Application")
    If Err.Number <> 0 Then
        WScript.Echo "GetObject error: 0x" & Hex(Err.Number) & " - " & Err.Description
        Err.Clear

        WScript.Echo "Trying to launch SolidWorks..."
        Dim shell
        Set shell = CreateObject("WScript.Shell")
        shell.Run "C:\Software\sw2022\SOLIDWORKS\sldworks.exe", 1, False
        WScript.Sleep 10000

        Set swApp = CreateObject("SldWorks.Application")
        If Err.Number <> 0 Then
            WScript.Echo "Still failed: 0x" & Hex(Err.Number) & " - " & Err.Description
            WScript.Quit
        End If
    End If
End If
On Error GoTo 0

WScript.Echo "Connected to SolidWorks!"
WScript.Echo "Version: " & swApp.RevisionNumber
swApp.Visible = True
WScript.Echo "SolidWorks is visible"
