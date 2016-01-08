/* OpenBRF -- by marco tarini. Provided under GNU General Public License */



#include <QtGui>
#include "brfData.h"
#include "selector.h"
#include "iniData.h"
#include "tablemodel.h"

const char * tokenTabName[N_TOKEN] = {
  QT_TRANSLATE_NOOP("Selector", "&Mesh"),
  QT_TRANSLATE_NOOP("Selector", "Te&xture"),
  QT_TRANSLATE_NOOP("Selector", "&Shader"),
  QT_TRANSLATE_NOOP("Selector", "Mat&erial"),
  QT_TRANSLATE_NOOP("Selector", "S&keleton"),
  QT_TRANSLATE_NOOP("Selector", "&Animation"),
  QT_TRANSLATE_NOOP("Selector", "&Collision"),
};


void Selector::addToRefMeshA(){ emit(addToRefMesh(0)); }
void Selector::addToRefMeshB(){ emit(addToRefMesh(1)); }
void Selector::addToRefMeshC(){ emit(addToRefMesh(2)); }
void Selector::addToRefMeshD(){ emit(addToRefMesh(3)); }
void Selector::addToRefMeshE(){ emit(addToRefMesh(4)); }
void Selector::addToRefMeshF(){ emit(addToRefMesh(5)); }
void Selector::addToRefMeshG(){ emit(addToRefMesh(6)); }
void Selector::addToRefMeshH(){ emit(addToRefMesh(7)); }
void Selector::addToRefMeshI(){ emit(addToRefMesh(8)); }
void Selector::addToRefMeshJ(){ emit(addToRefMesh(9)); }

void Selector::addShortCuttedAction(QAction *act){
	addAction(act);
	contextMenu->addAction(act);
}

Selector::Selector(QWidget *parent)
  : QTabWidget(parent)
{
	QString keepShiftInfo("(keep shift pressed to multiply)");

	iniData = NULL;
	for (int i=0; i<N_TOKEN; i++) {
		tab[i]=NULL;
		//tabIndex[i]=-1;
	}
	iniDataWaitsSaving = false;


	contextMenu = new QMenu(this);
	connect(contextMenu, SIGNAL(triggered(QAction *)),parent, SLOT(onActionTriggered(QAction *)));
	connect(this, SIGNAL(currentChanged(int)), this, SLOT(onChanged()) );

	//connect(this, SIGNAL(currentChanged(int)), parent, SLOT(undoHistoryAddAction()) );
	connect(contextMenu, SIGNAL(triggered(QAction *)),parent, SLOT(undoHistoryAddAction(QAction *)));

	breakAniWithIniAct = new QAction(tr("Split via action.txt..."), this);
	breakAniWithIniAct ->setStatusTip(tr("Split sequence following the action.txt file. A new \"action [after split].txt\" file is also produced, which use the new animation."));

	breakAniAct = new QAction(tr("Auto-split"), this);
	breakAniAct->setStatusTip(tr("Auto-split sequence into its separated chunks, separating it at lasge gaps in frames."));


	aniExtractIntervalAct = new QAction(tr("Extract interval..."), this);
	aniExtractIntervalAct->setStatusTip(tr("Extract an animation from an interval of times."));

	aniMirrorAct = new QAction(tr("Mirror"), this);
	aniMirrorAct->setStatusTip(tr("Mirror this animation over the X axis."));

	aniRemoveIntervalAct = new QAction(tr("Remove interval..."), this);
	aniRemoveIntervalAct->setStatusTip(tr("Remove an interval of times from the animation."));

	aniMergeAct = new QAction(tr("Merge animations"), this);
	aniMergeAct->setStatusTip(tr("Merge two animations into one -- intervals must be right!"));

	renameAct = new QAction(tr("Rename..."), this);
	renameAct->setShortcut(QString("F2"));
	connect(renameAct, SIGNAL(triggered()), parent, SLOT(renameSel()));
	addShortCuttedAction(renameAct);

	removeAct = new QAction(tr("Remove"), this);
	removeAct->setShortcut(QString("delete"));
	connect(removeAct, SIGNAL(triggered()), parent, SLOT(deleteSel()));
	addShortCuttedAction(removeAct);

	duplicateAct = new QAction(tr("Duplicate"), this);

	goNextTabAct = new QAction(tr("next tab"),this);
	goNextTabAct->setShortcut(QString("right"));
	connect(goNextTabAct, SIGNAL(triggered()),this,SLOT(goNextTab()));
	addShortCuttedAction(goNextTabAct);

	goPrevTabAct = new QAction(tr("left tab"),this);
	goPrevTabAct->setShortcut(QString("left"));
	connect(goPrevTabAct, SIGNAL(triggered()),this,SLOT(goPrevTab()));
	addShortCuttedAction(goPrevTabAct);

	moveUpAct = new QAction(tr("Move up in the list"), this);
	moveUpAct->setShortcut(QString("Alt+up"));
	moveUpAct->setStatusTip(tr("Move this object upward in the list"));
	connect(moveUpAct, SIGNAL(triggered()), parent, SLOT(moveUpSel()));
	addShortCuttedAction(moveUpAct);

	moveDownAct = new QAction(tr("Move down in the list"), this);
	moveDownAct->setShortcut(QString("Alt+down"));
	moveDownAct->setStatusTip(tr("Move this object one step down in the list"));
	connect(moveDownAct, SIGNAL(triggered()), parent, SLOT(moveDownSel()));
	addShortCuttedAction(moveDownAct);


	addToRefAnimAct = new QAction(tr("Add to reference animations"), this);
	addToRefAnimAct->setStatusTip(tr("Add this animation to reference animations (to use it later to display rigged meshes)."));

	addToRefSkelAct = new QAction(tr("Add to reference skeletons"), this);
	addToRefSkelAct->setStatusTip(tr("Add this animation to reference skeletons (to use it later for animations)."));

	for (int i=0; i<10; i++) {
		this->addToRefMeshAct[i]= new QAction(tr("set %1").arg(char('A'+i)), this);
		addToRefMeshAct[i]->setStatusTip(tr("Add this mesh to reference skins (to use it later to display animations)."));

	}

	connect(addToRefMeshAct[0], SIGNAL(triggered()), this, SLOT(addToRefMeshA()));
	connect(addToRefMeshAct[1], SIGNAL(triggered()), this, SLOT(addToRefMeshB()));
	connect(addToRefMeshAct[2], SIGNAL(triggered()), this, SLOT(addToRefMeshC()));
	connect(addToRefMeshAct[3], SIGNAL(triggered()), this, SLOT(addToRefMeshD()));
	connect(addToRefMeshAct[4], SIGNAL(triggered()), this, SLOT(addToRefMeshE()));
	connect(addToRefMeshAct[5], SIGNAL(triggered()), this, SLOT(addToRefMeshF()));
	connect(addToRefMeshAct[6], SIGNAL(triggered()), this, SLOT(addToRefMeshG()));
	connect(addToRefMeshAct[7], SIGNAL(triggered()), this, SLOT(addToRefMeshH()));
	connect(addToRefMeshAct[8], SIGNAL(triggered()), this, SLOT(addToRefMeshI()));
	connect(addToRefMeshAct[9], SIGNAL(triggered()), this, SLOT(addToRefMeshJ()));
	connect(this, SIGNAL(addToRefMesh(int)), parent, SLOT(addToRefMesh(int)));

	for (int i=0; i<MAX_USED_BY; i++){
		usedByAct[i] = new QAction("???",this);
		usedByAct[i]->setProperty("id",i);
		connect(usedByAct[i], SIGNAL(triggered()),  parent, SLOT(goUsedBy()));
	}

	QFont fontIta(QApplication::font());
	fontIta.setItalic(!fontIta.italic());

	usedByNoneAct = new QAction(tr("<none>"),this);
	usedByNoneAct->setFont( fontIta );

	for (int i=0; i<N_TXTFILES; i++){
		usedInAct[i]
		    = new QAction(tr("mod file <%1>").arg(txtFileName[i]),this);
		usedInAct[i+N_TXTFILES]
		    = new QAction(tr("mod file <%1> (indirectly)").arg(txtFileName[i]),this);
		usedInAct[i]->setFont( fontIta );
		usedInAct[i+N_TXTFILES]->setFont( fontIta );
	}
	usedInNoTxtAct = new QAction(tr("<no .txt file>"),this);
	usedInNoTxtAct->setFont(fontIta);
	usedInCoreAct[0] = new QAction(tr("<core engine>"),this);
	usedInCoreAct[0]->setFont(fontIta);
	usedInCoreAct[1] = new QAction(tr("<core engine> (indirectly)"),this);
	usedInCoreAct[1]->setFont(fontIta);

	usedIn_NotInModule = new QAction(tr("<not in module.ini>"),this);
	usedIn_NotInModule->setFont(fontIta);
	usedIn_SaveFirst = new QAction(tr("<save file to find out>"),this);
	usedIn_SaveFirst->setFont(fontIta);

	usedByComputeAct = new QAction(tr("(not computed: compute now)"),this);
	connect(usedByComputeAct, SIGNAL(triggered()),parent, SLOT(computeUsedBy()));


	exportBodyAct = new QAction(tr("Export..."), this);

	exportImportMeshInfoAct = new QAction(tr("Info on mesh import/export"), this);

	exportStaticMeshAct = new QAction(tr("Export static mesh..."), this);
	exportStaticMeshAct->setStatusTip(tr("Export this model (or this frame) as a 3D static mesh."));

	exportMovingMeshAct = new QAction(tr("Export vertex ani..."), this);
	exportMovingMeshAct->setStatusTip(tr("Export this model as a mesh with vertex animation."));

	exportMeshGroupAct = new QAction(tr("Export combined mesh..."), this);
	exportMeshGroupAct->setStatusTip(tr("Export this group of models in a single OBJ."));
	exportMeshGroupManyFilesAct = new QAction(tr("Export all meshes"), this);
	exportMeshGroupManyFilesAct->setStatusTip(tr("Export each of these models as separate OBJs."));

	exportBodyGroupManyFilesAct = new QAction(tr("Export all..."), this);
	exportBodyGroupManyFilesAct->setStatusTip(tr("Export each of these collison bodies as separate files."));

	exportRiggedMeshAct = new QAction(tr("Export rigged mesh..."), this);
	exportRiggedMeshAct->setStatusTip(tr("Export this model (or this frame) as a rigged mesh."));

	exportSkeletonAct = new QAction(tr("Export (nude) skeleton..."), this);
	exportSkeletonAct ->setStatusTip(tr("Export this skeleton (as a set of nude bones)."));
	exportSkinAct = new QAction(tr("Export skeleton with skin..."), this);
	exportSkinAct->setStatusTip(tr("Export this skeleton (as a rigged skin)."));
	exportSkinForAnimationAct     = new QAction(tr("Export a skin for this ani"), this);
	exportSkinForAnimationAct->setStatusTip(tr("Export a rigged skin which can be used for this animation."));

	aniToVertexAniAct = new QAction(tr("Convert into vertex animation"),this);
	aniToVertexAniAct->setStatusTip(tr("Convert skeletal animation into a vertex animation using current skin and skeleton"));

	meshToVertexAniAct = new QAction(tr("Convert into vertex animation"),this);
	meshToVertexAniAct->setStatusTip(tr("Convert rigged mesh into a vertex animation using current animation and skeleton"));

	exportAnimationAct = new QAction(tr("Export animation..."), this);
	exportAnimationAct->setStatusTip(tr("Export this animation."));

	reimportMeshAct = new QAction(tr("Reimport mesh..."), this);
	reimportMeshAct->setStatusTip(tr("Reimport this mesh from file."));
	reimportAniAct = new QAction(tr("Reimport animation..."), this);
	reimportAniAct->setStatusTip(tr("Reimport this mesh from file."));
	reimportBodyAct = new QAction(tr("Reimport collision body..."), this);
	reimportBodyAct->setStatusTip(tr("Reimport this collision mesh from file."));


	reskeletonizeAct = new QAction(tr("Reskeletonize..."), this);
	reskeletonizeAct->setStatusTip(tr("Adapt this rigged mesh to a new skeleton"));

	aniReskeletonizeAct = new QAction(tr("Reskeletonize..."), this);
	aniReskeletonizeAct->setStatusTip(tr("Adapt this animation to a new skeleton"));

	transferRiggingAct = new QAction(tr("Transfer rigging"), this);
	transferRiggingAct->setStatusTip(tr("Copy rigging from one mesh to another"));

	stiffenRiggingAct = new QAction(tr("Make rigging stiffer"), this);
	stiffenRiggingAct->setStatusTip(tr("Make the rigging of selected mesh(es) somewhat rigidier"));

	smoothenRiggingAct = new QAction(tr("Make rigging softer"), this);
	smoothenRiggingAct->setStatusTip(tr("Make the rigging of selected mesh(es) somewhat softer."));

	flipAct = new QAction(tr("Mirror"), this);
	flipAct->setStatusTip(tr("Mirror this object on the X axis."));

	transformAct = new QAction(tr("Roto-translate-rescale..."),this);
	transformAct->setStatusTip(tr("Apply a geometric transform."));

	scaleAct = new QAction(tr("Rescale..."), this);
	scaleAct->setStatusTip(tr("Rescale this object."));

	noSelectionDummyAct = new QAction(tr("(no object selected)"), this);
	QFont f = noSelectionDummyAct->font(); f.setItalic(true); noSelectionDummyAct->setFont( f );
	noSelectionDummyAct->setEnabled(false);

	shiftAniAct = new QAction(tr("Shift time interval..."), this);
	shiftAniAct->setStatusTip(tr("Shift a time interval for this animation"));

	bodyMakeQuadDominantAct = new QAction(tr("Make quad-dominant"), this);
	bodyMakeQuadDominantAct->setStatusTip(tr("Try to merge most triangles into fewer quads (more efficient!)"));
	bodyMerge = new QAction(tr("Combine collision objects"), this);
	bodyMerge->setStatusTip(tr("Make a combined collision obj. unifying these objs."));

	meshRecomputeNormalsAndUnify = new QAction(tr("Recompute normals..."), this);
	meshRecomputeNormalsAndUnify->setStatusTip(tr("Recompute normals for this model, and unify pos and vertices"));

	meshUnify = new QAction(tr("Clean redundant vert/pos"), this);
	meshUnify->setStatusTip(tr("Removes any unused vertices or positions and merge identical ones"));

	meshUvTransformAct = new QAction(tr("Transfrom texture coords"),this);
	meshUvTransformAct->setStatusTip(tr("Translates/Scales/Flips UV coords"));

	meshFixRiggingRigidParts = new QAction(tr("Quick fix rigging of rigid-parts"), this);
	meshFixRiggingRigidParts->setStatusTip(tr("Attempts to fix rigging of small-parts, making them rigid"));

	meshSubdivideIntoComponents = new QAction(tr("Split into connected sub-meshes"), this);
	meshSubdivideIntoComponents->setStatusTip(tr("Create a separate mesh for each connected component of this mesh."));

	meshMerge = new QAction(tr("Combine meshes"), this);
	meshMerge->setStatusTip(tr("Make a combined mesh unifying these meshes."));

	meshToBody = new QAction(tr("Make a collision object"), this);
	meshToBody->setStatusTip(tr("Turn this mesh(es) into a combined Collision object."));

	meshMountOnBone = new QAction(tr("Mount on one bone..."), this);
	meshMountOnBone->setStatusTip(tr("Put this mesh on top of a single skeleton bone."));

	meshRemoveBackfacing = new QAction(tr("remove all"), this);
	meshRemoveBackfacing->setStatusTip(tr("Remove all faces that are backfacing (e.g. in beard meshes)."));
	meshRemoveBackfacing->setData(tr("Back-faces: "));
	meshAddBackfacing = new QAction(tr("add (x2 faces)"), this);
	meshAddBackfacing->setStatusTip(tr("Duplicate all faces: for each current face, add a backfacing face."));
	meshAddBackfacing->setData(tr("Back-faces: "));

	meshComputeAoAct = new QAction(tr("Color with Ambient Occlusion"), this);
	meshComputeAoAct->setStatusTip(tr("Set per vertex color as ambient occlusion (globlal lighting) %1").arg(keepShiftInfo));

	meshColorWithTextureAct = new QAction(tr("Copy colors from texture"), this);
	meshColorWithTextureAct->setStatusTip(tr("Set per vertex color as texture colors %1").arg(keepShiftInfo));

	meshFemininizeAct = new QAction(tr("Add femininized frame"),this);
	meshFemininizeAct->setStatusTip(tr("Build a feminine frame for this armour(s)"));

	meshComputeLodAct = new QAction(tr("Compute LODs"), this);
	meshComputeLodAct->setStatusTip(tr("Tries to compute a LOD pyramid"));

	meshTellBoundingBoxAct = new QAction(tr("Get dimensions..."), this);
	meshTellBoundingBoxAct->setStatusTip(tr("Tell me the dimension of selected object(s)"));

	meshAniSplitAct = new QAction(tr("Separate all frames"), this);
	meshAniSplitAct->setStatusTip(tr("Split all frames, making 1 mesh per frame"));

	meshRecolorAct = new QAction(tr("Color uniform..."), this);
	meshRecolorAct->setStatusTip(tr("Set per vertex color as a uniform color %1").arg(keepShiftInfo));

	meshRecomputeTangentsAct = new QAction(tr("Recompute tangent dirs"), this);
	meshRecomputeTangentsAct->setStatusTip(tr("(Tangent dirs are needed for bump-mapping)"));

	meshTuneColorAct = new QAction(tr("Tune colors HSB..."), this);
	meshTuneColorAct->setStatusTip(tr("Then Hue Saturation and Brightness of per-vertex colors"));

	discardHitboxAct = new QAction(tr("Discard hit-boxes"), this);
	discardHitboxAct->setStatusTip(tr("Discard hit-box set associated to skeletons with this name"));

	meshFreezeFrameAct = new QAction(tr("rigging (freeze current pose)"), this);
	meshFreezeFrameAct->setStatusTip(tr("Discard rigging, but freeze mesh in its current pose"));

	meshUnmountAct = new QAction(tr("rigging (un-mount from bone)"),this);
	meshUnmountAct->setStatusTip(tr("Discard rigging, and move object back at origin."));

	meshAniMergeAct = new QAction(tr("Merge as frames in a vertex ani"), this);
	meshAniMergeAct->setStatusTip(tr("Merge these meshes, in their current order, as frames in a mesh ani"));

	discardColAct = new QAction(tr("per-vertex color"), this);
	discardColAct->setStatusTip(tr("Reset per-vertex coloring (i.e. turn all full-white)"));
	discardRigAct = new QAction(tr("rigging"), this);
	discardRigAct->setStatusTip(tr("Discard rigging (per-verex bone attachments)"));
	discardTanAct = new QAction(tr("tangent directions"), this);
	discardTanAct->setStatusTip(tr("Remove tangent directions (saves space, they are needed mainly for bumbmapping)"));
	discardNorAct = new QAction(tr("normals"), this);
	discardNorAct->setStatusTip(tr("Disregard normals, so to merge more vertices (and use less of them)"));
	discardAniAct = new QAction(tr("vertex animation"), this);
	discardAniAct->setStatusTip(tr("Discard vertex animation (keep only current frame)"));
	//exportAnyBrfAct = new QAction(tr("in a BRF"), this);
	//exportAnyBrfAct->setStatusTip(tr("Export this object in a BRF file."));

	exportSkeletonModAct = new QAction(tr("Make a skeleton-modification mesh..."), this);
	importSkeletonModAct = new QAction(tr("Modify from a skeleton-modification mesh..."), this);

	//hitboxToBodyAct = new QAction(tr("Turn hitboxes to collision-body"), this);
	//bodyToHitboxAct = new QAction(tr("Use this collision for hitboxes of same-named skel"), this);
	//saveSkeletonHitboxAct = new QAction(tr("Save hit-boxes in XML..."), this);

	connect(aniReskeletonizeAct, SIGNAL(triggered()), parent, SLOT(aniReskeletonize()));

	connect(breakAniAct, SIGNAL(triggered()),this,SLOT(onBreakAni()));
	connect(aniExtractIntervalAct, SIGNAL(triggered()),parent,SLOT(aniExtractInterval()));
	connect(aniRemoveIntervalAct, SIGNAL(triggered()),parent,SLOT(aniRemoveInterval()));
	connect(aniMergeAct, SIGNAL(triggered()),parent,SLOT(aniMerge()));
	connect(aniMirrorAct, SIGNAL(triggered()),parent,SLOT(aniMirror()));
	connect(breakAniWithIniAct, SIGNAL(triggered()),this,SLOT(onBreakAniWithIni()));
	connect(meshRecolorAct,SIGNAL(triggered()),parent,SLOT(meshRecolor()));
	connect(meshColorWithTextureAct,SIGNAL(triggered()),parent,SLOT(meshColorWithTexture()));
	connect(meshTellBoundingBoxAct,SIGNAL(triggered()),parent,SLOT(meshTellBoundingBox()));
	connect(meshTuneColorAct,SIGNAL(triggered()),parent,SLOT(meshTuneColor()));
	connect(meshComputeAoAct, SIGNAL(triggered()), parent, SLOT(meshComputeAo()));
	connect(meshComputeLodAct, SIGNAL(triggered()), parent, SLOT(meshComputeLod()));
	connect(meshFixRiggingRigidParts, SIGNAL(triggered()), parent, SLOT(meshFixRiggingRigidParts()));
	connect(meshFemininizeAct,SIGNAL(triggered()),parent,SLOT(meshFemininize()));
	connect(meshRecomputeNormalsAndUnify,  SIGNAL(triggered()),parent,SLOT(meshRecomputeNormalsAndUnify()));
	connect(meshSubdivideIntoComponents,  SIGNAL(triggered()),parent,SLOT(meshSubdivideIntoComponents()));
	connect(meshUnify,  SIGNAL(triggered()),parent,SLOT(meshUnify()));
	connect(meshMerge,  SIGNAL(triggered()),parent,SLOT(meshMerge()));
	connect(bodyMerge,  SIGNAL(triggered()),parent,SLOT(bodyMerge()));
	connect(meshToBody,SIGNAL(triggered()),parent,SLOT(meshToBody()));
	connect(meshMountOnBone,SIGNAL(triggered()),parent,SLOT(meshMountOnBone()));
	connect(meshRemoveBackfacing,SIGNAL(triggered()),parent,SLOT(meshRemoveBack()));
	connect(meshAddBackfacing,SIGNAL(triggered()),parent,SLOT(meshAddBack()));
	connect(meshRecomputeTangentsAct,SIGNAL(triggered()),parent,SLOT(meshRecomputeTangents()));

	connect(reimportMeshAct,SIGNAL(triggered()),parent,SLOT(reimportMesh()));
	connect(reimportAniAct,SIGNAL(triggered()),parent,SLOT(reimportAnimation()));
	connect(reimportBodyAct,SIGNAL(triggered()),parent,SLOT(reimportCollisionBody()));

	//connect(exportAnyBrfAct, SIGNAL(triggered()),parent,SLOT(exportBrf()));
	connect(exportStaticMeshAct, SIGNAL(triggered()),parent,SLOT(exportStaticMesh()));
	connect(exportRiggedMeshAct, SIGNAL(triggered()),parent,SLOT(exportRiggedMesh()));
	connect(exportMovingMeshAct, SIGNAL(triggered()),parent,SLOT(exportMovingMesh()));
	connect(exportMeshGroupAct, SIGNAL(triggered()),parent,SLOT(exportMeshGroup()));
	connect(exportMeshGroupManyFilesAct, SIGNAL(triggered()),parent,SLOT(exportMeshGroupManyFiles()));
	connect(exportBodyGroupManyFilesAct, SIGNAL(triggered()),parent,SLOT(exportBodyGroupManyFiles()));
	connect(exportSkeletonModAct, SIGNAL(triggered()),parent,SLOT(exportSkeletonMod()));
	connect(exportSkeletonAct, SIGNAL(triggered()),parent,SLOT(exportSkeleton()));
	connect(exportAnimationAct, SIGNAL(triggered()),parent,SLOT(exportAnimation()));
	connect(reskeletonizeAct, SIGNAL(triggered()),parent,SLOT(reskeletonize()));
	connect(transferRiggingAct, SIGNAL(triggered()),parent,SLOT(transferRigging()));

	//connect(hitboxToBodyAct, SIGNAL(triggered()),parent,SLOT(hitboxToBody()));
	//connect(bodyToHitboxAct, SIGNAL(triggered()),parent,SLOT(bodyToHitbox()));
	//connect(saveSkeletonHitboxAct, SIGNAL(triggered()),parent,SLOT(saveSkeletonHitbox()));

	connect(flipAct, SIGNAL(triggered()),parent,SLOT(flip()));
	connect(smoothenRiggingAct, SIGNAL(triggered()),parent,SLOT(smoothenRigging()));
	connect(stiffenRiggingAct, SIGNAL(triggered()),parent,SLOT(stiffenRigging()));
	connect(transformAct, SIGNAL(triggered()),parent,SLOT(transform()));
	connect(meshUvTransformAct, SIGNAL(triggered()),parent,SLOT(meshUvTransform()));
	connect(scaleAct, SIGNAL(triggered()),parent,SLOT(scale()));
	connect(exportBodyAct, SIGNAL(triggered()), parent, SLOT(exportCollisionBody()));
	connect(shiftAniAct, SIGNAL(triggered()),parent,SLOT(shiftAni()));

	connect(aniToVertexAniAct, SIGNAL(triggered()), parent, SLOT(aniToVertexAni()));
	connect(meshToVertexAniAct, SIGNAL(triggered()),parent, SLOT(meshToVertexAni()));
	connect(bodyMakeQuadDominantAct, SIGNAL(triggered()),parent,SLOT(bodyMakeQuadDominant()));

	connect(meshAniSplitAct,SIGNAL(triggered()),parent,SLOT(meshAniSplit()));
	connect(meshAniMergeAct,SIGNAL(triggered()),parent,SLOT(meshAniMerge()));

	connect(discardAniAct,SIGNAL(triggered()),parent,SLOT(meshDiscardAni()));
	connect(discardColAct,SIGNAL(triggered()),parent,SLOT(meshDiscardCol()));
	connect(discardNorAct,SIGNAL(triggered()),parent,SLOT(meshDiscardNor()));
	connect(discardRigAct,SIGNAL(triggered()),parent,SLOT(meshDiscardRig()));
	connect(discardTanAct,SIGNAL(triggered()),parent,SLOT(meshDiscardTan()));
	connect(meshFreezeFrameAct,SIGNAL(triggered()),parent,SLOT(meshFreezeFrame()));
	connect(meshUnmountAct,SIGNAL(triggered()),parent,SLOT(meshUnmount()));


	connect(discardHitboxAct, SIGNAL(triggered()), parent, SLOT(skeletonDiscardHitbox()));

	connect(exportSkinAct, SIGNAL(triggered()), parent, SLOT(exportSkeletonAndSkin()));
	connect(exportSkinForAnimationAct, SIGNAL(triggered()), parent, SLOT(exportSkeletonAndSkin()));

	connect(importSkeletonModAct, SIGNAL(triggered()),parent,SLOT(importSkeletonMod()));

	connect(duplicateAct, SIGNAL(triggered()), parent, SLOT(duplicateSel()));
	//


	connect(addToRefAnimAct, SIGNAL(triggered()), parent, SLOT(addToRef()));
	connect(addToRefSkelAct, SIGNAL(triggered()), parent, SLOT(addToRef()));
	this->setMinimumWidth(200);

	// prepare all tabs (w/o attaching them for now)
	for (int ti=0; ti<N_TOKEN; ti++) {

		tab[ti] =// new QListWidget;
		    new QListView;

		tableModel[ti] = new MyTableModel(this);

		tab[ti]->setModel(tableModel[ti]);

		//if (ti==MESH) tab[ti]->setSelectionModel(new MySelectionModel() );

		if (ti==MESH || ti==MATERIAL || ti==BODY || ti==TEXTURE || ti==SKELETON || ti==ANIMATION) {
			tab[ti]->setSelectionMode(QAbstractItemView::ExtendedSelection);
			QString msg =  tr("[Right-Click]: tools for %1. Multiple selections with [Shift] or [Ctrl].").arg(IniData::tokenFullName(ti));
			if (ti==MESH) { if (msg.endsWith('.')) msg.chop(1);  msg.append(" or [Alt]."); }
			tab[ti]->setStatusTip(msg);
		}
		else
			tab[ti]->setStatusTip(QString(tr("[Right-Click]: tools for %1.")).arg(IniData::tokenFullName(ti)));

		if (ti==MESH) connect(tab[ti],
		        SIGNAL(clicked(QModelIndex)),
		        this, SLOT(onClicked(QModelIndex)) );

		connect(tab[ti]->selectionModel(),
		        SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
		        this, SLOT(onChanged()) );


		//connect(tab[ti],SIGNAL(clicked(QModelIndex)),
		//        parent, SLOT(undoHistoryAddAction()) );

		connect(tab[ti]->selectionModel(),
		        SIGNAL(currentChanged(QModelIndex,QModelIndex)),
		        parent, SLOT(undoHistoryAddAction()) );
	}
	/*connect(new QShortcut(QKeySequence("up"),this),SIGNAL(activated()),
	        parent, SLOT(undoHistoryAddAction()) );
	connect(new QShortcut(QKeySequence("down"),this),SIGNAL(activated()),
	        parent, SLOT(undoHistoryAddAction()) );*/
}

void Selector::moveSel(int dx){
	QListView* c=(QListView*)this->currentWidget();

	if (c) {
		QModelIndex li=c->selectionModel()->selectedIndexes()[0];
		QModelIndex lj=li.sibling(li.row()+dx,li.column());
		if (lj.isValid()) {
			c->selectionModel()->blockSignals(true);
			c->clearSelection();
			c->selectionModel()->select(lj,QItemSelectionModel::Select);
			c->selectionModel()->setCurrentIndex(lj,QItemSelectionModel::NoUpdate);
			c->setCurrentIndex( lj );
			c->setFocus();
			c->selectionModel()->blockSignals(false);
		}
	}
}


/*
void Selector::onRenameSel(){
	QListView* tab = (QListView*)currentWidget();
	if (!tab) return;
	QModelIndexList li = tab->selectionModel()->selectedIndexes();
	if (li.size()!=1) return;
	QModelIndex i = li[0];
	//tab->e
	tab->edit(i);
}
*/


int Selector::firstSelected() const{
	if (!this->currentWidget()) return -1;
	unsigned int s =
	    ((QListView*)(this->currentWidget()))
	    ->selectionModel()->selectedIndexes().size();
	if (s==0) return -1;
	return
	    (((QListView*)(this->currentWidget()))
	     ->selectionModel()->selectedIndexes())[0].row();
}

int Selector::lastSelected() const{
	if (!this->currentWidget()) return -1;
	unsigned int s =
	    ((QListView*)(this->currentWidget()))
	    ->selectionModel()->selectedIndexes().size();
	if (s==0) return -1;
	return
	    (((QListView*)(this->currentWidget()))
	     ->selectionModel()->selectedIndexes())[s-1].row();
}

QModelIndexList Selector::selectedList() const{
	if (!this->currentWidget()) return QModelIndexList();
	return
	    ((QListView*)(this->currentWidget()))
	    ->selectionModel()->selectedIndexes();
}

int Selector::onlySelected(int kind) const{
	if (!currentWidget()) return -1;
	if (currentTabName()!=kind) return -1;
	if (numSelected()!=1) return -1;
	int i= ((QListView*)(this->currentWidget()))
	    ->selectionModel()->selectedIndexes()[0].row();
	return i;
}

int Selector::numSelected() const{
	if (!this->currentWidget()) return 0;
	return
	    ((QListView*)(this->currentWidget()))
	    ->selectionModel()->selectedIndexes().size();
}


void Selector::updateData(const BrfData &data){

	setup(data);

}


void Selector::selectAll(){
	int kind = currentTabName();
	assert(kind>=0 && kind<N_TOKEN);
	QListView* c=tab[kind];
	MyTableModel* m = tableModel[kind];

	if (c) {
		QModelIndex li0 = m->pleaseCreateIndex(0,0);
		QModelIndex li1 = m->pleaseCreateIndex(m->size()-1,0);
		c->selectionModel()->select(QItemSelection(li0,li1), QItemSelectionModel::Select);
	}
	onChanged();
}

void Selector::invertSelection(){
	int kind = currentTabName();
	assert(kind>=0 && kind<N_TOKEN);
	QListView* c=tab[kind];
	MyTableModel* m = tableModel[kind];

	if (c) {
		QModelIndex li0 = m->pleaseCreateIndex(0,0);
		QModelIndex li1 = m->pleaseCreateIndex(m->size()-1,0);
		c->selectionModel()->select(QItemSelection(li0,li1), QItemSelectionModel::Toggle);
	}
	onChanged();
}


void Selector::goNextTab(){
	int j=currentIndex();
	if (j<count()-1) setCurrentIndex(j+1);
	else setCurrentIndex(0);
}

void Selector::goPrevTab(){
	int j=currentIndex();
	if (j>0) setCurrentIndex(j-1);
	else setCurrentIndex(count()-1);
}

int Selector::currentTabName() const{
	for (int i=0; i<N_TOKEN; i++)
		if (this->currentWidget()==tab[i]) return i;
	return NONE;
}

void Selector::addDataToAllActions(QMenu *m, QString s){
	QList<QAction*> l = m->actions();
	for (int i=0; i<l.size(); i++) l[i]->setData(s);
}

void Selector::updateContextMenu(){
	contextMenu->clear();

	if (!this->currentWidget()) {
		//contextMenu->setTitle("Selected objects");
		contextMenu->addAction(noSelectionDummyAct);
		return;
	}

	QModelIndexList sel=
	    ((QListView*)(this->currentWidget()))->selectionModel()->selectedIndexes() ;



	bool onesel = sel.size()==1;
	bool mulsel = sel.size()>1;
	bool nosel = sel.size()==0;
	int seli = 0; if (sel.size()>0) seli = sel[0].row();
	int t = currentTabName();


	//contextMenu->setTitle(tr("Selected %1").arg(IniData::tokenPlurName(t)));
	if (nosel) {
		contextMenu->addAction(noSelectionDummyAct);
		return;
	}

	contextMenu->addAction(removeAct);
	if (onesel) renameAct->setText(tr("Rename...")); else renameAct->setText(tr("Group rename..."));
	contextMenu->addAction(renameAct);
	if (onesel) {
		contextMenu->addAction(duplicateAct);
		//contextMenu->addSeparator();
		contextMenu->addAction(moveUpAct);
		contextMenu->addAction(moveDownAct);
	}

	if (onesel){

		//contextMenu->addSeparator();
		QMenu *m = contextMenu->addMenu(tr("Used by:"));
		if (iniDataWaitsSaving)
			m->addAction(usedIn_SaveFirst);
		else
			if (!iniData) {
				m->addAction(usedIn_NotInModule);

			} else {
				if (iniData->updated<4) {
					m->addAction( usedByComputeAct );
				} else {
					ObjCoord c(iniFileIndex,seli,t);
					const std::vector< ObjCoord > &s ( iniData->usedBy(c) );
					IniData::UsedInType ui = iniData->usedIn(c);

					//qDebug("TEST: C=(%d %d %d), size=%d",iniFileIndex,seli,t,s.size());

					for (int i=0; i<(int)N_TXTFILES; i++){
						if (ui.direct & bitMask(i)) m->addAction(usedInAct[i]); else
							if (ui.indirect & bitMask(i)) m->addAction(usedInAct[i+N_TXTFILES]);
					}
					if (ui.direct & bitMask(TXTFILE_CORE)) m->addAction(usedInCoreAct[0]); else
						if (ui.indirect & bitMask(TXTFILE_CORE)) m->addAction(usedInCoreAct[1]);
					if ((m->actions().size()==0) && (s.size()!=0)){
						m->addAction( usedInNoTxtAct );
					}
					for (unsigned int i=0; i<s.size(); i++) if (i<MAX_USED_BY){
						usedByAct[i]->setText(iniData->nameFull( s[i] ));
						m->addAction( usedByAct[i] );
					}
					if (m->actions().size()==0){
						m->addAction( usedByNoneAct );
					}
				}
			}

	}

	if (onesel) {

		contextMenu->addSeparator();

		if (t==MESH) {
			contextMenu->addAction(exportStaticMeshAct);
			if (data->mesh[ seli ].IsRigged())
				contextMenu->addAction(exportRiggedMeshAct);
			if (data->mesh[ seli ].frame.size()>1)
				contextMenu->addAction(exportMovingMeshAct);
			contextMenu->addAction(reimportMeshAct);
		}

		if (t==SKELETON) {
			contextMenu->addAction(exportSkeletonAct);
			contextMenu->addAction(exportSkinAct);
			contextMenu->addSeparator();
			contextMenu->addAction(exportSkeletonModAct);
			contextMenu->addAction(importSkeletonModAct);
			//contextMenu->addAction(saveSkeletonHitboxAct);
			//contextMenu->addAction(hitboxToBodyAct);
			//contextMenu->addAction(importSkeletonHitboxAct);
			contextMenu->addAction(discardHitboxAct);
		}
		if (t==ANIMATION){
			contextMenu->addAction(exportAnimationAct);
			contextMenu->addAction(exportSkinForAnimationAct);
			contextMenu->addAction(reimportAniAct);
		}
		if (t==BODY) {
			contextMenu->addAction(exportBodyAct);
			//if (data->Find( data->body[seli].name, SKELETON) ) // there is a skeleton with same name!
			//  contextMenu->addAction(bodyToHitboxAct);
			contextMenu->addAction(reimportBodyAct);
		}
	} else {
		if (t==MESH) {
			contextMenu->addSeparator();
			contextMenu->addAction(exportMeshGroupAct);
			contextMenu->addAction(exportMeshGroupManyFilesAct);
		}
		if (t==BODY) {
			contextMenu->addAction(exportBodyGroupManyFilesAct);
		}
	}

	// tool section
	bool sep = false;
	if (!nosel) {
		if (t==MESH) {
			const BrfMesh &mesh(data->mesh[ seli ]);
			if (!sep) contextMenu->addSeparator();
			if (mesh.IsRigged()) {
				contextMenu->addAction(reskeletonizeAct);
				contextMenu->addAction(meshFemininizeAct);
				contextMenu->addAction(meshFixRiggingRigidParts);
				contextMenu->addAction(smoothenRiggingAct);
				contextMenu->addAction(stiffenRiggingAct);
				if (onesel) contextMenu->addAction(meshToVertexAniAct);
			}
			contextMenu->addAction(meshMountOnBone);
			//contextMenu->addAction(transferRiggingAct);

			contextMenu->addSeparator(); sep=true;


			contextMenu->addAction(transformAct);
			contextMenu->addAction(meshUvTransformAct);
			contextMenu->addAction(flipAct);
			//contextMenu->addAction(scaleAct);
			contextMenu->addAction(meshRecomputeNormalsAndUnify);
			contextMenu->addAction(meshRecomputeTangentsAct);
			contextMenu->addAction(meshUnify);
			if (onesel)  {
				contextMenu->addAction(meshSubdivideIntoComponents);
				if (mesh.HasVertexAni())
				contextMenu->addAction(meshAniSplitAct);
			}
			if (!onesel) {
				contextMenu->addAction(meshMerge);
				contextMenu->addAction(meshAniMergeAct);
			}
			contextMenu->addAction(meshComputeLodAct);
			contextMenu->addAction(meshToBody);
			contextMenu->addAction(meshTellBoundingBoxAct);

			QMenu *m = contextMenu->addMenu(tr("Backfacing faces"));
			m->addAction(meshRemoveBackfacing);
			m->addAction(meshAddBackfacing);


			m = contextMenu->addMenu(tr("Discard"));
			m->addAction(discardColAct);
			m->addAction(discardTanAct);
			m->addAction(discardAniAct);
			m->addAction(discardNorAct);
			m->addAction(discardRigAct);
			m->addAction(meshFreezeFrameAct);
			m->addAction(meshUnmountAct);
			addDataToAllActions(m,"Discard ");

			discardRigAct->setEnabled(mulsel || mesh.IsRigged());
			discardAniAct->setEnabled(mulsel || mesh.HasVertexAni());
			meshFreezeFrameAct->setEnabled(mulsel || mesh.IsRigged());
			meshUnmountAct->setEnabled(mulsel || mesh.IsRigged());
			discardColAct->setEnabled(mulsel || mesh.hasVertexColor);
			discardNorAct->setEnabled( true );
			discardTanAct->setEnabled(mulsel || mesh.HasTangentField());


			contextMenu->addSeparator();

			contextMenu->addAction(meshRecolorAct);
			contextMenu->addAction(meshColorWithTextureAct);
			contextMenu->addAction(meshComputeAoAct);
			contextMenu->addAction(meshTuneColorAct);

		}

		if (t==BODY) {
			if (!sep) contextMenu->addSeparator(); sep=true;
			contextMenu->addAction(flipAct);
			contextMenu->addAction(transformAct);
			if (!onesel && !nosel) { contextMenu->addAction(bodyMerge); }
			contextMenu->addAction(bodyMakeQuadDominantAct);
		}
		if (t==ANIMATION) {
			if (!sep) contextMenu->addSeparator(); sep=true;
			contextMenu->addAction(aniMirrorAct);
			if (onesel) {
				contextMenu->addAction(breakAniAct);
				contextMenu->addAction(breakAniWithIniAct);
				contextMenu->addAction(shiftAniAct);
				contextMenu->addAction(aniExtractIntervalAct);
				contextMenu->addAction(aniRemoveIntervalAct);
				contextMenu->addAction(aniToVertexAniAct);
			}
			contextMenu->addAction(aniReskeletonizeAct);
			if (mulsel) contextMenu->addAction(aniMergeAct);
		}
	}


	// add to reference
	if (onesel && t==MESH) {
		contextMenu->addSeparator();
		QMenu* refMenu=contextMenu->addMenu(tr("Add to reference skins"));
		int N=reference->GetFirstUnusedLetter();
		for (int i=0; i<N; i++) {
			addToRefMeshAct[i]->setText(tr("to Skin Set %1").arg(char('A'+i)) );
			refMenu->addAction(addToRefMeshAct[i]);
		}
		if (N<10) {
			addToRefMeshAct[N]->setText(tr("to Skin Set %1 [new set]").arg(char('A'+N)) );
			refMenu->addAction(addToRefMeshAct[N]);
		}
	}
	if (onesel && t==ANIMATION) {
		contextMenu->addSeparator();
		contextMenu->addAction(addToRefAnimAct);
	}
	if (onesel && t==SKELETON) {
		contextMenu->addSeparator();
		contextMenu->addAction(addToRefSkelAct);
	}

}

void Selector::contextMenuEvent(QContextMenuEvent *event)
{
	if (!this->currentWidget()) { event->ignore(); return; }
	updateContextMenu();

	contextMenu->exec(event->globalPos());
	event->accept();
}


void Selector::setIniData(const IniData *data, int fi){
	iniData = data;
	iniFileIndex = fi;
}

template<class BrfType>
void Selector::addBrfTab(const vector<BrfType>  &v){

	int ti = BrfType::tokenIndex();

	tableModel[ti]->clear();
	if (v.size()!=0) {

		for (unsigned int k=0; k<v.size(); k++) {
			tableModel[ti]->vec.push_back( QString( v[k].name ) );

		}
		if (iniData && (iniData->updated>=4) && (!iniDataWaitsSaving)) {
			for (unsigned int k=0; k<v.size(); k++) {
				ObjCoord oc(iniFileIndex,k,ti);
				int h=-1;
				if (iniData->usedIn(oc).directOrIndirect()!=0) h=1;
				else if (iniData->usedBy(oc).size()==0) h=-2;
				tableModel[ti]->vecUsed.push_back( h );
			}
		} else {
			tableModel[ti]->vecUsed.resize(v.size(),0);
		}
		//tab[ti]->setModel(tableModel[ti]);

		tableModel[ti]->updateChanges();

		//tab[ti]->viewport()->update();

		if (tab[ti]->selectionModel()->selectedIndexes().size()==0) {
			tab[ti]->selectionModel()->select(
			      tableModel[ti]->pleaseCreateIndex(0,0),
			      QItemSelectionModel::Select
			      );
			tab[ti]->setCurrentIndex(tableModel[ti]->pleaseCreateIndex(0,0));
		}


	} else {
	}

	/*QModelIndexList list = tab[ti]->selectionModel()->selectedIndexes();
	for (int i=0; i<list.size(); i++){
		if (list[i].row()>=tableModel[ti]->size()) tab[ti]->selectionModel()->select(
				tableModel[ti]->pleaseCreateIndex(i,0),QItemSelectionModel::Deselect);
	}*/
	//tab[ti]->selectionModel()->setCurrentIndex(tableModel[ti]->pleaseCreateIndex(0,0),QItemSelectionModel::NoUpdate);

}

void Selector::hideEmpty(){

	int n=0;
	for (int i=0; i<N_TOKEN; i++){
		if (tableModel[i]->size()) n++;
	}

	int reorder[N_TOKEN] = {
	  BODY,
	  MESH,
	  MATERIAL,
	  TEXTURE,
	  SHADER,
	  ANIMATION,
	  SKELETON,
	};

	int prev = 0;
	for (int ii=0; ii<N_TOKEN; ii++) {
		int i = reorder[ii];
		int v = tableModel[i]->size();
		if (v==0) {
			int j = indexOf( tab[i] );
			if (j>=0) removeTab(j);
			//tab[i]->setVisible(false);

			//int j = indexOf( tab[i] );
			//if (j>=0) this->removeTab(  j );
		} else {
			QString title(tr(tokenTabName[i]));
			if (n>3) title.truncate(3);
			else if (n>2) title.truncate(4);
			if (n>3)
				title = QString("%1%2").arg( title ).arg(v);
			else title = QString("%1(%2)").arg( title ).arg(v);
			int j = indexOf( tab[i] );
			if (j<0) insertTab(prev,tab[i], title);
			else setTabText(j,title);
			tab[i]->setVisible(true);
			prev++;

		}
	}
}

void Selector::onBreakAniWithIni(){
	int s = tab[ANIMATION]->selectionModel()->selectedIndexes().constBegin()->row();
	emit breakAni(s,true);
}

void Selector::onBreakAni(){
	int s = tab[ANIMATION]->selectionModel()->selectedIndexes().constBegin()->row();
	emit breakAni(s,false);
}

int Selector::currentIndexOnList() const{
	int k = currentIndex();
	if (k<0) return -1;
	if (k>=N_TOKEN) return -1;
	return tab[k]->currentIndex().row();
}


void Selector::selectOne(int kind, int i){
	assert(kind>=0 && kind<N_TOKEN);
	QListView* c=tab[kind];
	if (c) {
		c->clearSelection();
		QModelIndex li = tableModel[kind]->pleaseCreateIndex(i,0);
		c->selectionModel()->setCurrentIndex(li,QItemSelectionModel::NoUpdate);
		c->selectionModel()->select(li,QItemSelectionModel::Select);
		c->setCurrentIndex(li);
		c->scrollTo(li,QAbstractItemView::PositionAtCenter);

		this->setCurrentWidget(c);
		c->setFocus();

	}
	onChanged();
}

void Selector::selectOneSilent(int kind, int i){

	assert(kind>=0 && kind<N_TOKEN);
	QListView* c=tab[kind];
	if (c) {
		c->selectionModel()->blockSignals(true);
		c->clearSelection();
		QModelIndex li = tableModel[kind]->pleaseCreateIndex(i,0);
		c->selectionModel()->setCurrentIndex(li,QItemSelectionModel::NoUpdate);
		c->selectionModel()->select(li,QItemSelectionModel::Select);
		c->setCurrentIndex(li);
		c->scrollTo(li,QAbstractItemView::PositionAtCenter);

		this->setCurrentWidget(c);
		c->setFocus();
		c->selectionModel()->blockSignals(false);

	}
	onChanged();
}

void Selector::onClicked(const QModelIndex & mi){
	//qDebug("click");

	bool alt = QApplication::keyboardModifiers()&Qt::AltModifier;

	if(!alt) return;


	if (!data) return;

	int ti = currentTabName();
	if (ti==NONE) return;

	MyTableModel *tm = tableModel[ti];
	if (!tm) return;

	QListView* t = tab[ti]; //
	if (!t) return;

	QItemSelectionModel *m = t->selectionModel();
	if (!m) return;

	bool selected = m->isSelected( mi );
	int i = mi.row();
	if (i<0) return;
	if (i>=(int) data->mesh.size()) return;


	QString meshName(data->mesh[i].name);
	int p = meshName.indexOf('.');
	if (p!=-1) meshName.truncate(p);
	QString meshNamePlusDot = meshName;
	meshNamePlusDot.append('.');


	QItemSelection newSel;
	for (uint j=0; j<data->mesh.size(); j++) if (j!=(uint)i){
		QString nameJ(data->mesh.at(j).name);
		if ( (nameJ == meshName) || nameJ.startsWith(meshNamePlusDot) ) {
			QModelIndex mj = tm->pleaseCreateIndex(j,0);
			newSel.select(mj,mj);
		}
	}
	if (!newSel.isEmpty()) {
		//newSel.select(mi,mi); // to let clicked one be the last selected.

		m->select(newSel,QItemSelectionModel::Toggle);

	}
	//t->grabKeyboard();

	qDebug("Alt-click on %d (%s) (%s): selected %d items (%d)",i,
	   meshName.toAscii().data(),
	  (selected)?"select":"deselect",
	  newSel.count(),
	  m->selectedIndexes().size()
	);

}

void Selector::keyPressEvent(QKeyEvent * e){
	QTabWidget::keyPressEvent(e);
	//qDebug("Selector::KeypressEvent!!!");
	//e->text();
}

void Selector::onChanged(){
	//qDebug("OnCHANGED?");
	for(int ti=0; ti<N_TOKEN; ti++) if (tab[ti]) {
		//if (tab[ti]) tab[ti]->clearSelection();
		if (this->currentWidget()==tab[ti]) {
			QItemSelectionModel * tmp = tab[ti]->selectionModel();
			assert(tmp);
			//qDebug("OnCHANGED! (%d)",tmp->selectedIndexes().size());
			emit setSelection(
			      tmp->selectedIndexes()
			      , ti );
			return;
		}
	}
	//static QModelIndexList empty;
	//emit setSelection(empty , NONE );
}

void Selector::setup(const BrfData &_data){
	addBrfTab<BrfMesh> (_data.mesh);
	addBrfTab<BrfShader> (_data.shader);
	addBrfTab<BrfTexture> (_data.texture);
	addBrfTab<BrfMaterial> (_data.material);
	addBrfTab<BrfSkeleton> (_data.skeleton);
	addBrfTab<BrfAnimation> (_data.animation);
	addBrfTab<BrfBody> (_data.body);
	data = &_data;

	hideEmpty();
	onChanged();

	//static QModelIndexList *empty = new QModelIndexList();
	//emit setSelection(*empty , NONE );
}
