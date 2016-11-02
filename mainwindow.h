/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>
#include <QLabel>

//#include <map>

#include "brfData.h"
#include "iniData.h"
#include "glwidgets.h"
#include "selector.h"
#include "guipanel.h"
#include "brfHitBox.h"
#include "carryPosition.h"


class AskTransformDialog;
class AskUvTransformDialog;
//namespace Ui
//{
//    class MainWindow;
//}

struct UndoLevel {
	// info on the action(s) which took us to the status:
	QString actionName;
	int actionRepetitions;
	QString actionDescription() const;

	// the status:
	BrfData data;
	bool needsBeSaved;

};

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow();

	int loadModAndDump(QString modpath, QString file);

	int GetFirstUnusedRefLetter() const;
	bool loadFile(const QString &fileName);

	static int getLanguageOption();
	QString getNextTranslatorFilename(){return nextTranlationFilename;};

	void setUseAlphaCommands(bool mode);
private:
	BrfData brfdata;
	BrfData reference;
	BrfData clipboard; // also used for frames...
	BrfData brfdataBackup;
	BrfData brfdataTmp;
	IniData inidata;
	MeshMorpher femininizer;
	std::vector<CarryPosition> carryPositionSet;
	BrfData hitboxSet;

	bool editingRef;
	bool editingVertexData;
	template<class BrfType> bool addNewGeneral(QStringList def);

	bool useAlphaCommands;

	AskTransformDialog* askTransformDialog;
	AskUvTransformDialog* askUvTransformDialog;


private slots:


	void notifyCheckboardChanged();
	bool setEditingRef(bool mode);
	bool setEditingVertexData(bool mode);
    void setViewmodeMult(int i);

	void closeEvent(QCloseEvent *event);
	void newFile();
	bool open();
	bool save();
	bool saveAs();
	bool saveHitboxes();
	bool createScenePropText();
	bool openRecentFile();
	bool openRecentMod();
	bool editRef();
	void enterOrExitVertexDataMode();

	void registerExtension();

	void aniExtractInterval();
	void aniMirror();
	void aniRemoveInterval();
	void aniMerge();
	void aniReskeletonize();
	void aniToVertexAni();


	void about();
	void aboutCheckboard();
	void aboutColorAssignment();
	void aboutCurrentShader();
	void breakAni(int which, bool useIni);
	void shiftAni();
	void bodyMakeQuadDominant();
	void bodyMerge();
	void reskeletonize();
	void transferRigging();
	void flip();
	void scale();
	void transform();
	void smoothenRigging();
	void stiffenRigging();
	void onChangeMeshMaterial(QString newName);
	void onChangeFlags(QString flags); // of any object
	void onChangeTimeOfFrame(QString flags);
	void onSelectedPoint(float, float, float);

	//bool hitboxToBody();
	//bool bodyToHitbox();
	void hitboxEdit(int whichAttrib, int dir);
	void hitboxSymmetrize();
	void hitboxSetRagdollOnly(bool);
	void hitboxActivate(bool);
	void hitboxReset();

	void setSelection(const QModelIndexList &l,int);

	void updateDataMaterial();
	void updateDataShader();
	void updateDataBody();

	void updateTextureAccessDup();
	void updateTextureAccessDel();
	void updateTextureAccessAdd();

	//bool exportBrf();
	bool exportCollisionBody();
	bool exportMeshGroup();
	bool exportMeshGroupManyFiles();
	bool exportBodyGroupManyFiles();
	bool exportStaticMesh();
	bool importStaticMesh();
	bool exportSkinnedMesh();
	bool importSkinnedMesh();
	bool importMovingMeshFrame();
	bool exportMovingMesh();
	bool importMovingMesh();
	bool exportSkeletonMod();
	bool importSkeletonMod();
	bool exportSkeletonAndSkin();
	bool exportSkeleton();
	bool importSkeleton();
	bool importAnimation();
	bool exportAnimation();
	bool importCollisionBody();
	bool addNewMaterial();
	bool addNewUiPicture();
	bool addNewTexture();
	bool addNewShader();

	bool reimportMesh();
	bool reimportAnimation();
	bool reimportCollisionBody();

	bool importBrf();

    void moveSel(int dir);

	void moveUpSel();
	void moveDownSel();
    void moveUpPageSel();
    void moveDownPageSel();
    void moveUpAllSel();
    void moveDownAllSel();

    void renameSel();
	void deleteSel();
	void duplicateSel();
	void addToRef(); // add current selected item to ref
	void addToRefMesh(int);

	bool performUndo();
	bool performRedo();

	void editCut();
	void editCopy();
	void editAddToCopy();
	void editPaste();
	void editCutFrame();
	void editCopyFrame();
	void editCopyComplete();
	void editCopyHitbox();
	void editCutComplete();
	void editPasteMergeMesh();
	void editPasteFrame();
	void editPasteRigging();
	void editPasteMod();
	void editPasteAniLowerParts();
	void editPasteTimings();
	void editPasteTextcoords();
	void editPasteVertColors();
	void editPasteVertAni();
	void editPasteHitbox();
	void sortEntries();
	void meshRecomputeNormalsAndUnifyDoIt();
	void meshRecomputeNormalsAndUnify_onSlider(int i);
	void meshRecomputeNormalsAndUnify_onCheckbox(bool i);
	void meshRecomputeNormalsAndUnify();
	void meshUnify();
	void meshFixRiggingRigidParts();
	void meshSubdivideIntoComponents();
	void meshMerge();
	void meshToBody();
	void meshMountOnBone();
	void meshRemoveBack();
	void meshAddBack();
	void meshDiscardRig();
	void meshDiscardCol();
	void meshDiscardAni();
	void meshDiscardNor();
	void meshDiscardTan();
	void meshRecolor();
	void meshTuneColor();
	void meshComputeAo();
	void meshColorWithTexture();
	void meshTuneColorDo(int,int,int,int);
	void meshFemininize();
	void meshComputeLod();
	void meshRecomputeTangents();
	void meshFreezeFrame();
	void meshAniMerge();
	void meshAniSplit();
	void meshTellBoundingBox();
	void meshUvTransform();
	void meshUvTransformDoIt();
	void meshUnmount();
	void meshToVertexAni();
	void learnFemininzation(); // from current selection
	void optionFemininzationUseDefault();
	void optionFemininzationUseCustom();
    void optionSetAutocomputeTangents( bool );

	void setFlagsShader();
	void setFlagsShaderRequires();
	void setFlagsMesh();
	void setFlagsMaterial();
	void setFlagsTexture();
	void setFlagsBody();

	void onClipboardChange();

	bool navigateLeft();
	bool navigateRight();
	bool navigateUp();
	bool navigateDown();
	bool searchBrf();
	void refreshReference();
	bool refreshIni();
	bool checkIni();
	bool searchIni();
	bool refreshSkeletonBodiesXml();
	//bool saveSkeletonBodiesXml();
	void skeletonDiscardHitbox();
    void skeletonScale();

	void openModuleIniFile();
	bool openNextInMod();
	bool openPrevInMod();
	void optionAutoFixTextureUpdated();
	void optionAutoFixTextureShowInfo();

	void optionLanguageSet0();
	void optionLanguageSet1();
	void optionLanguageSet2();
	void optionLanguageSet3();
	void optionLanguageSet4();
	void optionLanguageSetCustom();
	void optionSetBgColor();
	void optionLodSettings();

	void mab2tldHead();
	void tld2mabHead();
	void mab2tldArmor();
	void tld2mabArmor();
	void tldGrassAlphaPaint();
	void tldHead(float verse);
	void tldMakeDwarfSlim();
	void tldMakeDwarfBoots();
	void tldShrinkAroundBones();

	void goUsedBy();
	void computeUsedBy();
	void selectBrfData();
	void showUnrefTextures();
	void showModuleStats();
	void moduleSelect();
	void moduleOpenFolder();
	void exportNames();

	void onActionTriggered(QAction* q);
	void undoHistoryAddAction(QAction *q);
	void undoHistoryAddAction();
	void undoHistoryAddEditAction();

	void repeatLastCommand();
	void setUseOpenGL2(bool);
	void setNormalmap(int);
    void setSpecularmap(int);
	void updateSelectedMenu();

public slots:
	void displayInfo(QString st, int howlong);
	void activateRuler(bool mode);
	void activateFloatingProbe(bool mode);

private:
	bool loadCarryPositions();
	bool loadCarryPositions(QString filename);
	bool executingRepeatedCommand;
	bool askIfUseOpenGL2(bool extra);
	bool goTo(ObjCoord o);
	//std::map< std::string, std::string > mapMT;// map material to textures
	GLWidget *glWidget;
	Selector *selector;
	QSettings *settings;
	GuiPanel *guiPanel;

	bool guessPaths(QString fn);
	void updatePaths();
	bool loadIni(int lvl);
	QString mabPath;
	QString modName;
	QString lastSkeletonBodiesXmlPath;
	QString modPath() const;
	QString lastSearchString;
	bool usingWarband;

	int curLanguage; // 0 = use default
	QString nextTranlationFilename; // which file to load next time (post mortem msg)
	void setLanguage(int k);

	bool scanBrfForMaterials(const QString fname);
	bool scanIniForMaterials(const QString fname);
	bool scanBrfDataForMaterials(const BrfData &d);
	void tryLoadMaterials();

	void createActions();
	void createMenus();
	void createMiniViewOptions();
	void createConnections();
	bool saveFile(const QString &fileName);
	bool saveReference();
	void setCurrentFile(const QString &fileName);
	void updateRecentFileActions();
	void updateRecentModActions();

	// sets, gets, all bitflags of selected objects
	void getAllRequires(const vector<BrfShader> &v, unsigned int &orr, unsigned int &andd);
	bool setAllRequires(vector<BrfShader> &v, unsigned int toZero, unsigned int toOne);
	template<class BrfType> void getAllFlags(const vector<BrfType> &v, unsigned int &orr, unsigned int &andd);
	template<class BrfType> bool setAllFlags(vector<BrfType> &v, unsigned int toZero, unsigned int toOne);

	bool makeMeshSkinned( BrfMesh &m, bool becauseAddToRef , bool askUserAgain );

	bool mustOverwriteColors(); // if false, must multiply colors instead

	QPair<int, int>  askRefSkel(int nbones, int &method, int &res); // ask user to specify two skel
	int askRefSkin(); //  ask user to specify a skin
	int currentDisplaySkin(); // returns skin currently used as display
	BrfSkeleton* currentDisplaySkeleton(); // returns skeleton currently used to display
	int currentDisplayFrame(); // return v.a. frame currently used to dispaly
	BrfAnimation* currentDisplayAnimation(); // return skel animation
	int currentDisplaySkelAniFrame(); // return skel animation frame currently used as dispaly
	int gimmeASkeleton(int nbones); // returns index of a skeleton with n bones. Maybe asks for one if more than one.
	void cancelNavStack();
	void insert(const BrfMesh &m);
	void insert(const BrfSkeleton &s);
	void insert(const BrfAnimation &a);
	void insert(const BrfTexture &m);
	void insert(const BrfMaterial &m);
	void insert(const BrfShader &s);
	void insert(const BrfBody &s);
	void replace(BrfMesh &m);
	void replace(BrfSkeleton &s);
	void replace(BrfAnimation &a);
	void replace(BrfTexture &m);
	void replace(BrfMaterial &m);
	void replace(BrfShader &s);
	void replace(BrfBody &s);
	void insertOrReplace(const BrfMesh &m);
	void insertOrReplace(const BrfSkeleton &s);
	void insertOrReplace(const BrfAnimation &a);
	void insertOrReplace(const BrfTexture &m);
	void insertOrReplace(const BrfMaterial &m);
	void insertOrReplace(const BrfShader &s);
	void insertOrReplace(const BrfBody &s);
	template<class BrfType> void insert( vector<BrfType> &v, const BrfType &o);
	template<class BrfType> void insertOrReplace( vector<BrfType> &v, const BrfType &o);
	template<class BrfType> void replaceInit(BrfType &o);
	template<class BrfType> BrfType& getSelected(int n=0);
	template<class BrfType> BrfType& getUniqueSelected();
	int getNumSelected() const;
	int getSelectedIndex(int n) const;
	void selectOne(int kind, int i);

	template<class BrfType> void objectMergeSelected(vector<BrfType> &v);

	void loadSystemClipboard();
	void saveSystemClipboard();

	void saveOptions() const;
	void loadOptions();

	void inidataChanged(); // call me when brf content changed
	void updateGl();
	void updateGui();
	void updateSel();
	void findCurFileInIni();
	void addSelectedToClipBoard();
	void completeClipboard(bool andDelete);

	int afterMeshImport() const; // 0:nothing   1:merge   2:normal recompute and merge
	int assembleAniMode() const; // 0:trust vertex order   1:trust vertex coords
	bool usingModReference() const;
	int currAoBrightnessLevel() const;
	int currAoFromAboveLevel() const;
	bool currAoPerFace() const;

	void applyAfterMeshImport(BrfMesh &m);

	QString askExportFilename(QString);
	QString askExportFilename(QString, QString ext );
	QString askImportFilename(QString ext);
	QStringList askImportFilenames(QString ext, bool atMostOne=false);

	QString strippedName(const QString &fullFileName);

	QString curFile;
	QString curFileBackup;
	int curFileIndex; // in module.ini, or -1

	QWidget *comboViewmodeSelector;
	QButtonGroup *comboViewmodeBG;

	QMenu *fileMenu, *importMenu, *editMenu, *optionMenu, *moduleMenu, *selectedMenu, *toolMenu;

	QMenu *recentModsMenu, *autoFemMenu;

	QAction *newAct;
	QAction *registerMime;
	QAction *openAct;
	QAction *saveAct;
	QAction *saveAsAct;
	QAction *saveHitboxAct;
	QAction *createScenePropTextAct;
	QAction *exitAct;
	QAction *sortEntriesAct;
	QAction *enterVertexDataMode;
	QAction *exitVertexDataMode;
	QAction *invertSelectionAct;
	QAction *selectAllAct;
	QAction *aboutCheckboardAct;
	QAction *aboutCurrentShaderAct;
	QAction *aboutColorAssignementAct;
	QAction *aboutAct;
	QAction *editRefAct;
	QAction *separatorAct;
	QAction *editCutAct;
	QAction *editCutCompleteAct;
	QAction *editCopyAct;
	QAction *editCopyCompleteAct;
	QAction *editCopyHitboxAct;
	QAction *editAddToCopyAct;
	QAction *editPasteAct;
	QAction *editPasteMergeMeshAct;
	QAction *editPasteRiggingAct;
	QAction *editPasteTimingsAct;
	QAction *editPasteAniLowerPartsAct;
	QAction *editPasteModificationAct;
	QAction *editPasteHitboxAct;
	QAction *editCutFrameAct;
	QAction *editCopyFrameAct;
	QAction *editPasteFrameAct;
	QAction *editPasteTextcoordsAct;
	QAction *editPasteVertColorsAct;
	QAction *editPasteVertAniAct;

	QAction *undoAct;
	QAction *redoAct;
	QAction *fakeEditAction, *fakeEditFlagAction;
	QAction *openNextInModAct;
	QAction *openPrevInModAct;

	QAction *searchBrfAct;
	QAction *navigateLeftAct;
	QAction *navigateRightAct;
	QAction *navigateUpAct;
	QAction *navigateDownAct;
	QAction *refreshIniAct;
	QAction *computeUsedByAct;
	QAction *selectBrfDataAct;
	QAction *showUnrefTexturesAct;
	QAction *showModuleStatsAct;
	QAction *moduleSelectAct;
	QAction *moduleOpenFolderAct;
	QAction *exportNamesAct;

	QAction *mab2tldHeadAct;
	QAction *tld2mabHeadAct;
	QAction *mab2tldArmorAct;
	QAction *tld2mabArmorAct;
	QAction *tldMakeDwarfSlimAct;
	QAction *tldGrassAlphaPaintAct;
	QAction *tldShrinkAroundBonesAct;

	QAction *checkIniAct;
	QAction *searchIniAct;

	QAction *repeatLastCommandAct;

	QAction *repeatableAction, *lastAction;
	int tokenOfRepeatableAction; // e.g. mesh, texture...
	bool setNextActionAsRepeatable;

	QPair<ObjCoord , QString > navigationStack[2];

    QAction *optionAutoComputeTangents;
	QAction *optionAfterMeshLoadMerge;
	QAction *optionAfterMeshLoadRecompute;
	QAction *optionAfterMeshLoadNothing;
	QAction *optionAssembleAniMatchVert;
	QAction *optionAssembleAniMatchTc;
	QAction *optionAssembleAniQuiverMode;
	QAction *optionAutoFixTextureOn; // unused
	QAction *optionAutoFixTextureOff; // unused
	QAction *optionAutoFixTextureInfo; // unused
	QAction *optionAutoZoomUseGlobal;
	QAction *optionAutoZoomUseSelected;
	QAction *optionLanguage[6];
	QAction *optionLanguageCustom;
	QAction *optionInferMaterialOn;
	QAction *optionInferMaterialOff;
	QAction *optionUseOpenGL2;
	QAction *optionAoBrightness[5];
	QAction *optionAoFromAbove[2];
	QAction *optionAoPerFace[2];
	QAction *optionAoInAlpha;
	QAction *optionBgColor;
	QAction *optionLodSettingsAct;

	QAction *optionFeminizerUseCustom, *optionFeminizerUseDefault;
	QAction *optionLearnFeminization;

	QAction *optionUseModReference;
	QAction *optionUseOwnReference;

	QAction *tldMenuAction;

	QAction *activateFloatingProbeAct;
	QAction *activateRulerAct;

	QAction
	*importStaticMeshAct,
	*importSkinnedMeshAct,
	*importMovingMeshFrameAct,
	*importMovingMeshAct,
	*importSkeletonAct,
	*importAnimationAct,
	*importBodyAct,
	*importBrfAct,
	*addNewMaterialAct,
	*addNewShaderAct,
	*addNewTextureAct,
	*addNewUiPictureAct;

	enum { MaxRecentFiles = 20 };
	QAction *recentFileActs[MaxRecentFiles];
	QAction *recentModActs[MaxRecentFiles];

	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);

	bool maybeSave();
	bool maybeSaveHitboxes();
	void setModified(bool repeateble = true);
	void setModifiedHitboxes(bool mod);
	void setNotModified();
	void updateTitle();
	bool isModified;
	bool isModifiedHitboxes;
	QString lastImpExpFormat;
	// parameters for lodding

	enum {N_LODS = 4};
	bool lodBuild[N_LODS];
	float lodPercent[N_LODS];
	bool lodReplace;
	QColor background;

	QLabel* modStatus; // widget in status bar

	// to tune colors
	void meshTuneColorCancel(bool storeUndo);

	bool easterTLD; // if true, use easteregg
	bool _importStaticMesh(QString s, vector<BrfMesh> &m, vector<bool> &wasMultiple, bool onlyOneFile);

	static QString hitboxExplaination();
	QString senderText() const; // just a hack: returns the text of command being exectued:

	bool loadedModReference;
	QString referenceFilename(bool modSpecific) const;

	// generic importers
	bool _importCollisionBody(bool reimportExisting);
	bool _importAnimation(bool reimportExisting);
	bool _importMesh(bool reimportExisting);

	bool maybeWarnIfVertexAniTooBig(const BrfMesh &m, const BrfAnimation &a);

	std::vector<UndoLevel> undoHistoryRing;
	int undoHistoryRingIndex(int lvl) const;
	UndoLevel* undoHistory(int lvl);
	void undoHistoryClear();
	void markCurrendUndoAsSaved();
	int undoLvlCurr, undoLvlLast;
	int numModifics;
	void updateUndoRedoAct();
};

#endif // MAINWINDOW_H
