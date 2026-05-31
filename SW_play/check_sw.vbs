On Error Resume Next
Dim swApp
Set swApp = CreateObject("SldWorks.Application")
If Err.Number <> 0 Then
    WScript.Echo "Error: 0x" & Hex(Err.Number) & " - " & Err.Description
    WScript.Quit
End If
On Error GoTo 0

WScript.Echo "Connected to SolidWorks"
swApp.Visible = True

Dim doc
Set doc = swApp.ActiveDoc
If doc Is Nothing Then
    WScript.Echo "No active document"
Else
    WScript.Echo "Active document: " & doc.GetTitle
    WScript.Echo "Document type: " & doc.GetType
End If

Dim docs
Set docs = swApp.GetDocuments()
If Not docs Is Nothing Then
    WScript.Echo "Total open documents: " & docs.Count
End If
