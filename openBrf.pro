#
# OpenBRF -- by marco tarini. Provided under GNU General Public License
#

QT += opengl
QT += xml

CONFIG += exceptions

QMAKE_CXXFLAGS += -std=c++0x
#QMAKE_CXXFLAGS += -Werror
VCGLIB = ./vcglib
QMAKE_CXXFLAGS += "-isystem $$VCGLIB"

# RC_FILE = openBrf.rc
TARGET = openBrf
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    glwidgets.cpp \
    saveLoad.cpp \
    brfMesh.cpp \
    brfData.cpp \
    selector.cpp \
    tablemodel.cpp \
    brfShader.cpp \
    brfTexture.cpp \
    brfMaterial.cpp \
    brfSkeleton.cpp \
    brfAnimation.cpp \
    brfBody.cpp \
    guipanel.cpp \
    vcgmesh.cpp \
    askBoneDialog.cpp \
    $$VCGLIB/wrap/ply/plylib.cpp \
    $$VCGLIB/wrap/dae/xmldocumentmanaging.cpp \
    ioSMD.cpp \
    askSkelDialog.cpp \
    askTexturenameDialog.cpp \
    askFlagsDialog.cpp \
    iniData.cpp \
    ioOBJ.cpp \
    askModErrorDialog.cpp \
    ioMB.cpp \
    askTransformDialog.cpp \
    askCreaseDialog.cpp \
    main_info.cpp \
    main_create.cpp \
    main_ImpExp.cpp \
    brfHitBox.cpp \
    ioMD3.cpp \
    platform.cpp \
    askNewUiPictureDialog.cpp \
    askSelectBrfDialog.cpp \
    askUnrefTextureDialog.cpp \
    askIntervalDialog.cpp \
    askHueSatBriDialog.cpp \
    askLodOptionsDialog.cpp \
    askUvTransformDialog.cpp \
    askSkelPairDialog.cpp \
    askColorDialog.cpp \
    myselectionmodel.cpp
HEADERS += mainwindow.h \
    glwidgets.h \
    saveLoad.h \
    brfMesh.h \
    brfData.h \
    selector.h \
    tablemodel.h \
    brfShader.h \
    brfTexture.h \
    brfToken.h \
    brfMaterial.h \
    brfSkeleton.h \
    brfAnimation.h \
    brfBody.h \
    guipanel.h \
    vcgmesh.h \
    vcgExport.h \
    vcgImport.h \
    askBoneDialog.h \
    ioSMD.h \
    askSkelDialog.h \
    askTexturenameDialog.h \
    askFlagsDialog.h \
    iniData.h \
    askModErrorDialog.h \
    ioMB.h \
    askTransformDialog.h \
    bindTexturePatch.h \
    ddsData.h \
    askCreaseDialog.h \
    ioOBJ.h \
    ioMD3.h \
    askNewUiPictureDialog.h \
    askSelectBrfDialog.h \
    askUnrefTextureDialog.h \
    askIntervalDialog.h \
    askHueSatBriDialog.h \
    askLodOptionsDialog.h \
    askUvTransformDialog.h \
    askSkelPairDialog.h \
    askColorDialog.h \
    carryPosition.h \
    myselectionmodel.h
FORMS += guipanel.ui \
    askBoneDialog.ui \
    askSkelDialog.ui \
    askTexturenameDialog.ui \
    askFlagsDialog.ui \
    askModErrorDialog.ui \
    askTransformDialog.ui \
    askCreaseDialog.ui \
    mainwindow.ui \
    askNewUiPictureDialog.ui \
    askSelectBrfDialog.ui \
    askUnrefTextureDialog.ui \
    askIntervalDialog.ui \
    askHueSatBriDialog.ui \
    askLodOptionsDialog.ui \
    askUvTransformDialog.ui \
    askSkelPairDialog.ui
//INCLUDEPATH += "$$VCGLIB"
INCLUDEPATH += "lib3ds-1.3.0"
INCLUDEPATH += "."
RESOURCES += resource.qrc
TRANSLATIONS += translations/openbrf_zh.ts
TRANSLATIONS += translations/openbrf_en.ts
TRANSLATIONS += translations/openbrf_es.ts
TRANSLATIONS += translations/openbrf_de.ts
RC_FILE = openBrf.rc
win32 { 
    DEFINES += NOMINMAX
    DEFINES += _CRT_SECURE_NO_DEPRECATE
}
#INCLUDEPATH += "C:\projects\libraries\include"
# DEFINES += GLEW_STATIC

# SOURCES += "C:\projects\libraries\sources\glew-1.5.3\src\glew.c"
#LIBS += -L"C:\projects\libraries\lib" \
LIBS += "-lGLEW"
LIBS += "-lGLU"
#   % -lglew32
MOC_DIR = tmp
UI_DIR = tmp

OTHER_FILES += shaders/bump_fragment.cpp
OTHER_FILES += shaders/bump_vertex.cpp
OTHER_FILES += shaders/iron_fragment.cpp
OTHER_FILES += femininizer.morpher

LIBS += -lGL -lGLU
#LIBS += -lopengl32 -lglu32

DISTFILES += \
    translations/openbrf_de.ts \
    translations/openbrf_en.ts \
    translations/openbrf_es.ts \
    translations/openbrf_zh.ts








