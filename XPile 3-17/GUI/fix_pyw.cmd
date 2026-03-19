@echo off
ftype PythonwFile=C:\Users\azt12\AppData\Local\Programs\Python\Python312\pythonw.exe "%1" %*
assoc .pyw=PythonwFile
echo Done.
pause
