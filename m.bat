del sy.exe
del sy.obj
cl /nologo sy.cxx /O2i /EHac /Zi /DUNICODE /D_AMD64_ /link ntdll.lib /OPT:REF /incremental:no

