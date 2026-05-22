' run-hidden.vbs — launches PowerShell with no visible window.
'
' MSI custom actions that exec powershell.exe directly flash a console
' window briefly even with -WindowStyle Hidden, because PS creates the
' console host before applying the style. Wrapping the launch in this
' VBS and invoking via wscript.exe (which has no console host of its
' own) suppresses the flash entirely.
'
' Usage: wscript.exe //nologo run-hidden.vbs <script.ps1> [args...]

Option Explicit
Dim shell, cmd, i, exitCode
Set shell = CreateObject("WScript.Shell")

' Quote the script path so spaces in install paths don't break.
cmd = "powershell.exe -NoProfile -ExecutionPolicy Bypass -WindowStyle Hidden -File """ & WScript.Arguments(0) & """"

' Append remaining args verbatim — caller is responsible for quoting.
For i = 1 To WScript.Arguments.Count - 1
    cmd = cmd & " " & WScript.Arguments(i)
Next

' shell.Run(cmd, windowStyle, waitOnReturn)
'   windowStyle = 0 → hidden (no taskbar entry, no flash)
'   waitOnReturn = True → block until PS exits so MSI gets the exit code
exitCode = shell.Run(cmd, 0, True)
WScript.Quit exitCode
