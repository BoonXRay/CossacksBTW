COSSACKS_BUILD_PATH   =   ../../Cossacks_Build
DESTDIR         =   $${COSSACKS_BUILD_PATH}
TARGET          = dmcr

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_CXXFLAGS += -std=c++11 #-fdata-sections -ffunction-sections #-flto #-Wl,--gc-sections -Wl,--print-gc-sections

DEFINES -= UNICODE
DEFINES += "_WIN32_WINNT=0x0600"

win32:LIBS += libws2_32 libgdi32 libwinmm libdplayx libole32 libdsound

SOURCES += main.cpp \
    Ddini.cpp \
    Fog.cpp \
    Megapolis.cpp \
    GP_Draw.cpp \
    RealWater.cpp \
    3DBars.cpp \
    UnRar.cpp \
    3DGraph.cpp \
    NewMon.cpp \
    ZBuffer.cpp \
    Walls.cpp \
    Interface.cpp \
    NewCode/UdpHolePuncher.cpp \
    3DMapEd.cpp \
    3DRandMap.cpp \
    Mapa.cpp \
    Nature.cpp \
    MPlayer.cpp \
    Economy.cpp \
    Mouse_X.cpp \
    Nation.cpp \
    DrawForm.cpp \
    SelProp.cpp \
    stRecog.cpp \
    StringHash.cpp \
    Weapon.cpp \
    NewCode/SecureDivision.cpp \
    MapSprites.cpp \
    TopoGraf.cpp \
    Multi.cpp \
    ActiveScenary.cpp \
    SaveNewMap.cpp \
    Mine.cpp \
    ActiveZone.cpp \
    Recorder.cpp \
    Brigade.cpp \
    Transport.cpp \
    UnSyncro.cpp \
    GameSound.cpp \
    ShipTrace.cpp \
    CDirSnd.cpp \
    LoadSave.cpp \
    Groups.cpp \
    Build.cpp \
    DeviceCD.cpp \
    VirtScreen.cpp \
    NewAI.cpp \
    EInfoClass.cpp \
    Motion.cpp \
    Path.cpp \
    gFile.cpp \
    Hint.cpp \
    Upgrade.cpp \
    SafeNet.cpp \
    Masks.cpp \
    Dialogs.cpp \
    ExConst.cpp \
    BmpTool.cpp \
    IconTool.cpp \
    NewUpgrade.cpp \
    NewCode/MapOptionsEncoder.cpp \
    Fractal.cpp \
    IChat/GSC_ChatWindow.cpp \
    IntExplorer/ParseRQ.cpp \
    IChat/CS_Chat.cpp \
    CommCore/CommCore.cpp \
    ConstStr.cpp \
    CommCore/CommPeers.cpp \
    CommCore/CommRecv.cpp \
    CommCore/CommInit.cpp \
    CommCore/CommInet.cpp \
    CommCore/CommQueue.cpp \
    TMixer.cpp \
    StrategyResearch.cpp \
    Arc/GSCSet.cpp \
    Arc/GSCarch.cpp \
    Arc/IsiMasks.cpp \
    FastDraw.cpp \
    ResFile.cpp \
    IChat/ChatMain.c \
    IChat/ChatSocket.c \
    IChat/darray.c \
    IChat/hashtable.c \
    IChat/ChatHandlers.c \
    IChat/ChatChannel.c \
    IChat/ChatCallbacks.c \
    IChat/nonport.c \
    CommCore/CommRaw.cpp \
    CommCore/CommServerList.cpp \
    Pinger.cpp \
    IntExplorer/Explore.cpp \
    IntExplorer/DataXchange.cpp \
    IntExplorer/GameOnMap.cpp \
    IntExplorer/FnDefine.cpp \
    IntExplorer/PingEngine.cpp \
    Fonts.cpp \
    CWave.cpp \
    Sort.cpp

HEADERS += \
    Ddini.h \
    Icons.h \
    gFile.h \
    Upgrade.h \
    UnSyncro.h \
    Fog.h \
    Walls.h \
    Nature.h \
    Megapolis.h \
    CDirSnd.h \
    GP_Draw.h \
    3DBars.h \
    Transport.h \
    StrategyResearch.h \
    TopoGraf.h \
    NewAI.h \
    Sort.h \
    MPlayer.h \
    Recorder.h \
    3DMapEd.h \
    EInfoClass.h \
    Recorder.h \
    ActiveScenary.h \
    Fonts.h \
    UnRar.h \
    Arc/GSCSet.h \
    Arc/GSCTypes.h \
    Arc/GSCarch.h \
    3DGraph.h \
    NewMon.h \
    Path.h \
    ConstStr.h \
    NewUpgrade.h \
    ZBuffer.h \
    Masks.h \
    NewCode/UdpHolePuncher.h \
    Dpchat.h \
    IconTool.h \
    3DRandMap.h \
    DrawForm.h \
    BmpTool.h \
    CommCore/CommCore.h \
    LoadSave.h \
    Pinger.h \
    StringHash.h \
    MapSprites.h \
    ActiveZone.h \
    DeviceCD.h \
    TMixer.h \
    IChat/Chat.h \
    IChat/CS_Chat.h \
    IntExplorer/ParseRQ.h \
    Arc/IsiMasks.h \
    FastDraw.h \
    MapDescr.h \
    ResFile.h \
    VirtScreen.h \
    IChat/ChatMain.h \
    IChat/ChatSocket.h \
    IChat/nonport.h \
    IChat/darray.h \
    IChat/hashtable.h \
    IChat/ChatHandlers.h \
    IChat/ChatChannel.h \
    IChat/ChatCallbacks.h \
    SelProp.h \
    BoonUtils.hpp \
    IntExplorer/IntExplorer.h \
    IntExplorer/Explore.h \
    IntExplorer/GameOnMap.h \
    IntExplorer/DataXchange.h \
    Dialogs.h \
    Mapa.h \
    Interface.h \
    Multi.h \
    SaveNewMap.h \
    Fractal.h \
    Nation.h \
    RealWater.h \
    NewCode/MapOptionsEncoder.h \
    stRecog.h \
    main.h \
    Brigade.h \
    Economy.h \
    GameSound.h \
    Mouse_X.h \
    Mine.h \
    Groups.h \
    Motion.h \
    CWave.h \
    ExConst.h \
    ShipTrace.h \
    Weapon.h \
    Hint.h \
    SafeNet.h \
    IntExplorer/FnDefine.h \
    IntExplorer/PingEngine.h \
    Build.h
