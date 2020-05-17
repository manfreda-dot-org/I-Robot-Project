# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=I Robot - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to I Robot - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "I Robot - Win32 Release" && "$(CFG)" !=\
 "I Robot - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "I Robot.mak" CFG="I Robot - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "I Robot - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "I Robot - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "I Robot - Win32 Debug"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "I Robot - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\I Robot.exe"

CLEAN : 
	-@erase ".\Release\I Robot.exe"
	-@erase ".\Release\ErrorBox.obj"
	-@erase ".\Release\Globals.obj"
	-@erase ".\Release\WndProc.obj"
	-@erase ".\Release\Joystick.obj"
	-@erase ".\Release\DirectX.obj"
	-@erase ".\Release\MemoryManagement.obj"
	-@erase ".\Release\3d.obj"
	-@erase ".\Release\Config.obj"
	-@erase ".\Release\Application.obj"
	-@erase ".\Release\BankSwitch.obj"
	-@erase ".\Release\WinMain.obj"
	-@erase ".\Release\IO.obj"
	-@erase ".\Release\VideoHardware.obj"
	-@erase ".\Release\Mathbox.obj"
	-@erase ".\Release\EmulationThread.obj"
	-@erase ".\Release\Clipboard.obj"
	-@erase ".\Release\icon.res"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/I Robot.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/icon.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/I Robot.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /subsystem:windows /machine:I386 /fixed
# SUBTRACT LINK32 /pdb:none
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib winmm.lib /nologo /subsystem:windows /incremental:no\
 /pdb:"$(OUTDIR)/I Robot.pdb" /machine:I386 /out:"$(OUTDIR)/I Robot.exe" /fixed 
LINK32_OBJS= \
	"$(INTDIR)/ErrorBox.obj" \
	"$(INTDIR)/Globals.obj" \
	".\DrawHorizontalLineList.obj" \
	"$(INTDIR)/WndProc.obj" \
	"$(INTDIR)/Joystick.obj" \
	"$(INTDIR)/DirectX.obj" \
	"$(INTDIR)/MemoryManagement.obj" \
	"$(INTDIR)/3d.obj" \
	"$(INTDIR)/Config.obj" \
	"$(INTDIR)/Application.obj" \
	".\ScanEdge.obj" \
	"$(INTDIR)/BankSwitch.obj" \
	"$(INTDIR)/WinMain.obj" \
	".\6809.obj" \
	"$(INTDIR)/IO.obj" \
	"$(INTDIR)/VideoHardware.obj" \
	"$(INTDIR)/Mathbox.obj" \
	"$(INTDIR)/EmulationThread.obj" \
	"$(INTDIR)/Clipboard.obj" \
	"$(INTDIR)/icon.res" \
	".\ddraw.lib"

"$(OUTDIR)\I Robot.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "I Robot - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\I Robot.exe"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\I Robot.exe"
	-@erase ".\Debug\Globals.obj"
	-@erase ".\Debug\VideoHardware.obj"
	-@erase ".\Debug\DirectX.obj"
	-@erase ".\Debug\Clipboard.obj"
	-@erase ".\Debug\ErrorBox.obj"
	-@erase ".\Debug\MemoryManagement.obj"
	-@erase ".\Debug\3d.obj"
	-@erase ".\Debug\EmulationThread.obj"
	-@erase ".\Debug\Joystick.obj"
	-@erase ".\Debug\WndProc.obj"
	-@erase ".\Debug\Config.obj"
	-@erase ".\Debug\WinMain.obj"
	-@erase ".\Debug\Mathbox.obj"
	-@erase ".\Debug\Application.obj"
	-@erase ".\Debug\BankSwitch.obj"
	-@erase ".\Debug\IO.obj"
	-@erase ".\Debug\icon.res"
	-@erase ".\Debug\I Robot.ilk"
	-@erase ".\Debug\I Robot.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/I Robot.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/icon.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/I Robot.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib /nologo /subsystem:windows /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib winmm.lib /nologo /subsystem:windows /incremental:yes\
 /pdb:"$(OUTDIR)/I Robot.pdb" /debug /machine:I386 /out:"$(OUTDIR)/I Robot.exe" 
LINK32_OBJS= \
	"$(INTDIR)/Globals.obj" \
	".\DrawHorizontalLineList.obj" \
	"$(INTDIR)/VideoHardware.obj" \
	"$(INTDIR)/DirectX.obj" \
	"$(INTDIR)/Clipboard.obj" \
	"$(INTDIR)/ErrorBox.obj" \
	"$(INTDIR)/MemoryManagement.obj" \
	"$(INTDIR)/3d.obj" \
	"$(INTDIR)/EmulationThread.obj" \
	"$(INTDIR)/Joystick.obj" \
	".\ScanEdge.obj" \
	"$(INTDIR)/WndProc.obj" \
	"$(INTDIR)/Config.obj" \
	"$(INTDIR)/WinMain.obj" \
	".\6809.obj" \
	"$(INTDIR)/Mathbox.obj" \
	"$(INTDIR)/Application.obj" \
	"$(INTDIR)/BankSwitch.obj" \
	"$(INTDIR)/IO.obj" \
	"$(INTDIR)/icon.res" \
	".\ddraw.lib"

"$(OUTDIR)\I Robot.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "I Robot - Win32 Release"
# Name "I Robot - Win32 Debug"

!IF  "$(CFG)" == "I Robot - Win32 Release"

!ELSEIF  "$(CFG)" == "I Robot - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\6809.obj

!IF  "$(CFG)" == "I Robot - Win32 Release"

!ELSEIF  "$(CFG)" == "I Robot - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\DrawHorizontalLineList.obj

!IF  "$(CFG)" == "I Robot - Win32 Release"

!ELSEIF  "$(CFG)" == "I Robot - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\icon.rc
DEP_RSC_ICON_=\
	".\icon1.ico"\
	

"$(INTDIR)\icon.res" : $(SOURCE) $(DEP_RSC_ICON_) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\WndProc.c
DEP_CPP_WNDPR=\
	".\IRobot.h"\
	{$(INCLUDE)}"\ddraw.h"\
	{$(INCLUDE)}"\dsound.h"\
	{$(INCLUDE)}"\d3dtypes.h"\
	
NODEP_CPP_WNDPR=\
	"C:\MSDEV\INCLUDE\subwtype.h"\
	

"$(INTDIR)\WndProc.obj" : $(SOURCE) $(DEP_CPP_WNDPR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\WinMain.c
DEP_CPP_WINMA=\
	".\IRobot.h"\
	{$(INCLUDE)}"\ddraw.h"\
	{$(INCLUDE)}"\dsound.h"\
	{$(INCLUDE)}"\d3dtypes.h"\
	
NODEP_CPP_WINMA=\
	"C:\MSDEV\INCLUDE\subwtype.h"\
	

"$(INTDIR)\WinMain.obj" : $(SOURCE) $(DEP_CPP_WINMA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\VideoHardware.c
DEP_CPP_VIDEO=\
	".\IRobot.h"\
	".\Alphanumerics.h"\
	{$(INCLUDE)}"\ddraw.h"\
	{$(INCLUDE)}"\dsound.h"\
	{$(INCLUDE)}"\d3dtypes.h"\
	
NODEP_CPP_VIDEO=\
	"C:\MSDEV\INCLUDE\subwtype.h"\
	

"$(INTDIR)\VideoHardware.obj" : $(SOURCE) $(DEP_CPP_VIDEO) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\MemoryManagement.c
DEP_CPP_MEMOR=\
	".\IRobot.h"\
	{$(INCLUDE)}"\ddraw.h"\
	{$(INCLUDE)}"\dsound.h"\
	{$(INCLUDE)}"\d3dtypes.h"\
	
NODEP_CPP_MEMOR=\
	"C:\MSDEV\INCLUDE\subwtype.h"\
	

"$(INTDIR)\MemoryManagement.obj" : $(SOURCE) $(DEP_CPP_MEMOR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\IO.c
DEP_CPP_IO_Ca=\
	".\IRobot.h"\
	{$(INCLUDE)}"\ddraw.h"\
	{$(INCLUDE)}"\dsound.h"\
	{$(INCLUDE)}"\d3dtypes.h"\
	
NODEP_CPP_IO_Ca=\
	"C:\MSDEV\INCLUDE\subwtype.h"\
	

"$(INTDIR)\IO.obj" : $(SOURCE) $(DEP_CPP_IO_Ca) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Globals.c
DEP_CPP_GLOBA=\
	{$(INCLUDE)}"\ddraw.h"\
	{$(INCLUDE)}"\dsound.h"\
	{$(INCLUDE)}"\d3dtypes.h"\
	
NODEP_CPP_GLOBA=\
	"C:\MSDEV\INCLUDE\subwtype.h"\
	

"$(INTDIR)\Globals.obj" : $(SOURCE) $(DEP_CPP_GLOBA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ErrorBox.c
DEP_CPP_ERROR=\
	".\IRobot.h"\
	{$(INCLUDE)}"\ddraw.h"\
	{$(INCLUDE)}"\dsound.h"\
	{$(INCLUDE)}"\d3dtypes.h"\
	
NODEP_CPP_ERROR=\
	"C:\MSDEV\INCLUDE\subwtype.h"\
	

"$(INTDIR)\ErrorBox.obj" : $(SOURCE) $(DEP_CPP_ERROR) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\EmulationThread.c
DEP_CPP_EMULA=\
	".\IRobot.h"\
	{$(INCLUDE)}"\ddraw.h"\
	{$(INCLUDE)}"\dsound.h"\
	{$(INCLUDE)}"\d3dtypes.h"\
	
NODEP_CPP_EMULA=\
	"C:\MSDEV\INCLUDE\subwtype.h"\
	

"$(INTDIR)\EmulationThread.obj" : $(SOURCE) $(DEP_CPP_EMULA) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Application.c
DEP_CPP_APPLI=\
	".\IRobot.h"\
	{$(INCLUDE)}"\ddraw.h"\
	{$(INCLUDE)}"\dsound.h"\
	{$(INCLUDE)}"\d3dtypes.h"\
	
NODEP_CPP_APPLI=\
	"C:\MSDEV\INCLUDE\subwtype.h"\
	

"$(INTDIR)\Application.obj" : $(SOURCE) $(DEP_CPP_APPLI) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ddraw.lib

!IF  "$(CFG)" == "I Robot - Win32 Release"

!ELSEIF  "$(CFG)" == "I Robot - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\ScanEdge.obj

!IF  "$(CFG)" == "I Robot - Win32 Release"

!ELSEIF  "$(CFG)" == "I Robot - Win32 Debug"

!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=.\Joystick.c
DEP_CPP_JOYST=\
	".\IRobot.h"\
	{$(INCLUDE)}"\ddraw.h"\
	{$(INCLUDE)}"\dsound.h"\
	{$(INCLUDE)}"\d3dtypes.h"\
	
NODEP_CPP_JOYST=\
	"C:\MSDEV\INCLUDE\subwtype.h"\
	

"$(INTDIR)\Joystick.obj" : $(SOURCE) $(DEP_CPP_JOYST) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\3d.c
DEP_CPP_3D_C16=\
	".\DotVectorPolygon.h"\
	".\RenderOB.h"\
	".\RenderPF.h"\
	

"$(INTDIR)\3d.obj" : $(SOURCE) $(DEP_CPP_3D_C16) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Mathbox.c

"$(INTDIR)\Mathbox.obj" : $(SOURCE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\BankSwitch.c
DEP_CPP_BANKS=\
	".\IRobot.h"\
	{$(INCLUDE)}"\ddraw.h"\
	{$(INCLUDE)}"\dsound.h"\
	{$(INCLUDE)}"\d3dtypes.h"\
	
NODEP_CPP_BANKS=\
	"C:\MSDEV\INCLUDE\subwtype.h"\
	

"$(INTDIR)\BankSwitch.obj" : $(SOURCE) $(DEP_CPP_BANKS) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\DirectX.c
DEP_CPP_DIREC=\
	".\IRobot.h"\
	{$(INCLUDE)}"\ddraw.h"\
	{$(INCLUDE)}"\dsound.h"\
	{$(INCLUDE)}"\d3dtypes.h"\
	
NODEP_CPP_DIREC=\
	"C:\MSDEV\INCLUDE\subwtype.h"\
	

"$(INTDIR)\DirectX.obj" : $(SOURCE) $(DEP_CPP_DIREC) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Clipboard.c
DEP_CPP_CLIPB=\
	".\IRobot.h"\
	{$(INCLUDE)}"\ddraw.h"\
	{$(INCLUDE)}"\dsound.h"\
	{$(INCLUDE)}"\d3dtypes.h"\
	
NODEP_CPP_CLIPB=\
	"C:\MSDEV\INCLUDE\subwtype.h"\
	

"$(INTDIR)\Clipboard.obj" : $(SOURCE) $(DEP_CPP_CLIPB) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Config.c
DEP_CPP_CONFI=\
	".\IRobot.h"\
	{$(INCLUDE)}"\ddraw.h"\
	{$(INCLUDE)}"\dsound.h"\
	{$(INCLUDE)}"\d3dtypes.h"\
	
NODEP_CPP_CONFI=\
	"C:\MSDEV\INCLUDE\subwtype.h"\
	

"$(INTDIR)\Config.obj" : $(SOURCE) $(DEP_CPP_CONFI) "$(INTDIR)"


# End Source File
# End Target
# End Project
################################################################################
