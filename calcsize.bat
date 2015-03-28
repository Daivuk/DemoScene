call :getFilesize Release/DemoScene.exe
echo %fileSize% bytes

echo MsgBox "%fileSize% bytes"> msgbox.vbs
cscript msgbox.vbs

exit /b

:getFilesize
set filesize=%~z1
exit /b
