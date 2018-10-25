!if !EXIST(..\Makefiles\nmake\config.mak)
!error No config file found, please run 'configure --help' first.
!endif

!include ..\Makefiles\nmake\config.mak

!ifndef COMPILER_NAME
!error No compiler set, please run 'configure --help' first and chose a compiler.
!endif

!if (("$(COMPILER_NAME)" != "vc6") && \
     ("$(COMPILER_NAME)" != "vc70") && \
     ("$(COMPILER_NAME)" != "vc71") && \
     ("$(COMPILER_NAME)" != "vc8") && \
     ("$(COMPILER_NAME)" != "vc9") && \
     ("$(COMPILER_NAME)" != "vc14") && \
     ("$(COMPILER_NAME)" != "icl"))
!error '$(COMPILER_NAME)' not supported by this make file, please rerun 'configure' script and follow instructions.
!endif

SRCROOT=..

STLPORT_INCLUDE_DIR = ../../stlport

STLPORT_NATIVE_INCLUDE_PATHS= /DMSVCINCLUDEC="C:\Program Files (x86)\Windows Kits\10\Include\10.0.17134.0" /DMSVCINCLUDECPP="c:\Program Files (x86)\Microsoft Visual Studio\2017\Professional\VC\Tools\MSVC\14.15.26726"

# pull in the MSVC include directories that were set up by the vcvars*.bat. In this way we don't hardcode too much.
# ignore all standard include paths as this might cause the build to fail.
INCLUDES=/X $(INCLUDES) /I$(STLPORT_INCLUDE_DIR) /I"$(INCLUDE:;=" /I")" $(STLPORT_NATIVE_INCLUDE_PATHS)

!message INCLUDES=$(INCLUDES)
!include Makefile.inc

RC_FLAGS_REL = /I$(STLPORT_INCLUDE_DIR) /D "COMP=$(COMPILER_NAME)"
RC_FLAGS_DBG = /I$(STLPORT_INCLUDE_DIR) /D "COMP=$(COMPILER_NAME)"
RC_FLAGS_STLDBG = /I$(STLPORT_INCLUDE_DIR) /D "COMP=$(COMPILER_NAME)"

!if ("$(COMPILER_NAME)" != "vc14")
#Even if pure release and dbg targets do not need additionnal memory
#to be built they might if user wants to build a STL safe release build
#for instance.
OPT = /Zm800
!endif

!include $(SRCROOT)/Makefiles/nmake/top.mak
