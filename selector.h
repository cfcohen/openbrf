/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef SELECTOR_H
#define SELECTOR_H

#include <QTabWidget>
#include <QItemSelection>
#include "brfToken.h"

class BrfData;
class IniData;
class QListView;
class QMenu;
class MyTableModel;

class Selector : public QTabWidget
{
	Q_OBJECT

public:
	Selector(QWidget *parent=0);
	void setup(const BrfData &data);
	void updateData(const BrfData &data);
	void setIniData(const IniData* data, int fileIndex);
	int currentTabName() const;
	int firstSelected() const;
	int lastSelected() const;
	int numSelected() const;
	int onlySelected(int kind) const; // return index of only selected object of given kind or -1
	QModelIndexList selectedList() const;
    std::vector<int> allSelected() const;
	void moveSel(int d);
	BrfData* reference;
	const BrfData* data;
	const IniData* iniData;
	bool iniDataWaitsSaving;
	int iniFileIndex;
	void updateContextMenu();
	void keyPressEvent(QKeyEvent *);
	QMenu *contextMenu;

	static void addDataToAllActions(QMenu* m, QString s);

    void selectOne(int kind, int i);
    void selectOneSilent(int kind, int i);

public slots:
	void selectAll();
	void invertSelection();
    void selectMany( std::vector<int> i);

private slots:
	void onChanged();
    void onDoubleClicked(const QModelIndex & i);
	void onBreakAni();
	void onBreakAniWithIni();
	void addToRefMeshA();
	void addToRefMeshB();
	void addToRefMeshC();
	void addToRefMeshD();
	void addToRefMeshE();
	void addToRefMeshF();
	void addToRefMeshG();
	void addToRefMeshH();
	void addToRefMeshI();
	void addToRefMeshJ();
	void goNextTab();
	void goPrevTab();

signals:
	void setSelection(const QModelIndexList & newSel, int newToken);
	void addToRefMesh(int refIndex);
	void breakAni(int i, bool useIni);

private:
	void hideEmpty();
	template<class BrfType> void addBrfTab(const std::vector<BrfType> &x);
	QListView * tab[N_TOKEN];
	MyTableModel * tableModel[N_TOKEN];
	void contextMenuEvent(QContextMenuEvent *event);
	enum {MAX_USED_BY = 50};
	void addShortCuttedAction(QAction *act);
	int currentIndexOnList() const;
public:
	QAction
	*goNextTabAct,
	*goPrevTabAct,

	// tools
	*aniMirrorAct,
	*aniExtractIntervalAct,
	*aniRemoveIntervalAct,
	*aniMergeAct,
	*aniReskeletonizeAct,
	*aniToVertexAniAct,

	*breakAniAct,
	*breakAniWithIniAct,
	*meshRecomputeNormalsAndUnify,
	*meshUnify,
	*meshFixRiggingRigidParts,
	*meshSubdivideIntoComponents,
	*meshMerge,
	*meshToBody,
	*meshMountOnBone,
	*meshRemoveBackfacing,
	*meshAddBackfacing,
	*meshRecolorAct,
	*meshRecomputeTangentsAct,
	*meshTuneColorAct,
	*meshComputeAoAct,
	*meshColorWithTextureAct,
	*meshFemininizeAct,
	*meshComputeLodAct,
	*meshTellBoundingBoxAct,
	*meshFreezeFrameAct,
	*meshUnmountAct,
	*meshUvTransformAct,
	*meshToVertexAniAct,

	*meshAniMergeAct,
	*meshAniSplitAct,


	*renameAct,
	*removeAct,
	*moveUpAct,
	*moveDownAct,
	*duplicateAct,
	*discardColAct,
	*discardNorAct,
	*discardRigAct,
	*discardTanAct,
	*discardAniAct,


	// exporter acts
	*exportImportMeshInfoAct,
	*exportStaticMeshAct,
	*exportSkinnedMeshAct,
	*exportMovingMeshFrameAct,
	*exportMovingMeshAct,
	*exportMeshGroupAct,
	*exportMeshGroupManyFilesAct,

	*reimportMeshAct,
	*reimportAniAct,
	*reimportBodyAct,

	//*hitboxToBodyAct,
	//*bodyToHitboxAct,
	//*saveSkeletonHitboxAct,
	*discardHitboxAct,

	*exportSkeletonModAct,
	*exportSkeletonAct,
	*exportSkinAct,
	*exportSkinForAnimationAct,
	*exportAnimationAct,
	*exportBodyAct,
	*exportBodyGroupManyFilesAct,

	// importer acts
	*importSkeletonModAct,


	*reskeletonizeAct,
	*transferRiggingAct,
	*flipAct,
	*smoothenRiggingAct,
	*stiffenRiggingAct,
	*transformAct,
	*scaleAct,
	*shiftAniAct,
	*bodyMakeQuadDominantAct,
	*bodyMerge,

    *scaleSkeletonAct,
	*sortEntriesAct,
	*noSelectionDummyAct,

	*addToRefSkelAct,
	*addToRefAnimAct,
	*addToRefMeshAct[10],
	*usedByAct[MAX_USED_BY],
	*usedByComputeAct,
	*usedByNoneAct,
	*usedInCoreAct[2],
	*usedInNoTxtAct,
	*usedInAct[N_TXTFILES*2],
	*usedIn_NotInModule,
	*usedIn_SaveFirst;

};

#endif // SELECTOR_H
