#
# BRFminimal -- by marco tarini. Provided under GNU General Public License
#


CONFIG += console
CONFIG -= qt

debug {
  OBJECTS_DIR = minimal/obj
}

release {
  OBJECTS_DIR = minimal/obj
}

# RC_FILE = openBrf.rc
TARGET = brfMinimal
TEMPLATE = app
SOURCES += \
    saveLoad.cpp \
    brfMesh.cpp \
    brfData.cpp \
    brfShader.cpp \
    brfTexture.cpp \
    brfMaterial.cpp \
    brfSkeleton.cpp \
    brfAnimation.cpp \
    brfBody.cpp \
 #    vcgmesh.cpp \
 #    C:/projects/vcglib/wrap/ply/plylib.cpp \
 #    C:/projects/vcglib/wrap/dae/xmldocumentmanaging.cpp \
    ioSMD.cpp \
 #   ioOBJ.cpp \
 #    ioMB.cpp \
    ioMD3.cpp \ 
    mainBrfMinimal.cpp
HEADERS += \
    saveLoad.h \
    brfMesh.h \
    brfData.h \
    brfShader.h \
    brfTexture.h \
    brfToken.h \
    brfMaterial.h \
    brfSkeleton.h \
    brfAnimation.h \
    brfBody.h \
    vcgmesh.h \
    vcgExport.h \
    vcgImport.h \
    ioSMD.h \
    ddsData.h \
 #   ioOBJ.h \
    ioMD3.h \
    carryPosition.h 
INCLUDEPATH += "C:/projects/vcglib"
INCLUDEPATH += "C:/libs/lib3ds-1.3.0"
INCLUDEPATH += "C:\projects\sendbox\tarini"
win32 { 
    DEFINES += NOMINMAX
    DEFINES += _CRT_SECURE_NO_DEPRECATE
}
INCLUDEPATH += "C:\projects\libraries\include"








