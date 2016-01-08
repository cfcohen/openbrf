/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#include <QtGui>
#include <QDebug>
#include <algorithm>

#include "brfData.h"
#include "glwidgets.h"
#include "selector.h"
#include "mainwindow.h"
#include "tmp/ui_mainwindow.h"
#include "tmp/ui_guipanel.h"
#include "askBoneDialog.h"
#include "askSkelDialog.h"
#include "askTexturenameDialog.h"
#include "askModErrorDialog.h"
#include "askTransformDialog.h"
#include "askUvTransformDialog.h"
#include "askCreaseDialog.h"
#include "askNewUiPictureDialog.h"
#include "askUnrefTextureDialog.h"
#include "askSelectBrfDialog.h"
#include "askSkelPairDialog.h"
#include "askIntervalDialog.h"
#include "askHueSatBriDialog.h"
#include "askLodOptionsDialog.h"
#include "askColorDialog.h"

typedef QPair<int, int> Pair;

int MainWindow::getNumSelected() const{
	return selector->selectedList().size();
}
int MainWindow::getSelectedIndex(int n) const{
	return selector->selectedList()[n].row();
}

int MainWindow::loadModAndDump(QString modpath, QString file){
	modpath = modpath.replace('/','\\');
	if (modpath.endsWith('\\')) modpath.chop(1);
	if (!guessPaths(modpath+"\\Resource")) return -1;
	if (!inidata.loadAll(3)) return -2;
	inidata.updateAllLists();
	if (!inidata.saveLists(file)) return -3;
	return 1;
}


void MainWindow::activateFloatingProbe(bool mode){
	glWidget->setFloatingProbe(mode);
	if (mode) {
		glWidget->setRuler(false);
		if (activateRulerAct->isChecked()) activateRulerAct->setChecked(false);
	}
	if (activateFloatingProbeAct->isChecked()!=mode) activateFloatingProbeAct->setChecked(mode);

	if (activateFloatingProbeAct->isChecked()) guiPanel->setMeasuringTool(1);
	else if (activateRulerAct->isChecked()) guiPanel->setMeasuringTool(0);
	else guiPanel->setMeasuringTool(-1);
}

void MainWindow::activateRuler(bool mode){
	glWidget->setRuler(mode);
	if (mode) {
		glWidget->setFloatingProbe(false);
		if (activateFloatingProbeAct->isChecked()) activateFloatingProbeAct->setChecked(false);
	}
	if (activateRulerAct->isChecked()!=mode) activateRulerAct->setChecked(mode);

	if (activateFloatingProbeAct->isChecked()) guiPanel->setMeasuringTool(1);
	else if (activateRulerAct->isChecked()) guiPanel->setMeasuringTool(0);
	else guiPanel->setMeasuringTool(-1);
}

void MainWindow::repeatLastCommand(){
	executingRepeatedCommand = true;
	if ((repeatableAction) && (tokenOfRepeatableAction == selector->currentTabName()))
		repeatableAction->trigger();
}

void MainWindow::notifyCheckboardChanged(){
	aboutCheckboardAct->setVisible( (glWidget->lastMatErr.type!=0) );
}

void MainWindow::displayInfo(QString st,int howlong){
	statusBar()->showMessage(st,howlong);
}

bool MainWindow::maybeSave()
{
	if (isModified) {

		QMessageBox::StandardButton ret;
		ret = QMessageBox::warning(this, tr("OpenBrf"),
		                           tr("%1 been modified.\n"
		                              "Save changes?").arg((editingRef)?tr("Internal reference objects have"):tr("The dataset has")),
		                           QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		if (ret == QMessageBox::Save) {
			return save();
		}
		else if (ret == QMessageBox::Cancel)
			return false;
	}


	return maybeSaveHitboxes();
}

bool MainWindow::maybeSaveHitboxes()
{
	if (isModifiedHitboxes) {

		QMessageBox::StandardButton ret;
		ret = QMessageBox::warning(this, "OpenBrf",
		                           tr("Skeleton hitboxes have been modified.<br/>"
		                              "Save changes in /Data/skeleton_bodies.xml?")+hitboxExplaination(),
		                           QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		if (ret == QMessageBox::Save) {
			return saveHitboxes();
		}
		if (ret == QMessageBox::Discard) {
			refreshSkeletonBodiesXml();
		}
		else if (ret == QMessageBox::Cancel)
			return false;
	}
	//setModifiedHitboxes(false);
	return true;
}

bool MainWindow::saveHitboxes(){
	QString allnames;
	int nb = (int)hitboxSet.body.size();
	//if (nb==0) QMessageBox::war(this, "OpenBrf", tr("<br>  <br>  <b>There are no hitboxes to save?!?</b>"));
	for (int i=0; i<nb; i++) {
		BrfBody &b(hitboxSet.body[i]);
		allnames= allnames +"<br> -  <b>"+b.name+"</b>";
		if (strcmp(b.name,b.GetOriginalSkeletonName())!=0) {
			allnames += QString(" <font size=-1>(get rest of metadata from '%1' in XML file)</font>").arg(b.GetOriginalSkeletonName());
		}
	}

	if (!QFile(modPath()+"/Data").exists()) {
		QDir q(modPath());
		if (!q.mkdir("/Data")) {
			QMessageBox::warning(this,"OpenBrf",tr("Error: could not make missing folder 'Data' in module folder:\n %1").arg(modPath()));
			return false;
		}
	}
	QString fn = modPath()+"/Data/skeleton_bodies.xml";
	QString backupFn;

	int backupN = 0;
	if (QFile(fn).exists()) do {
		backupN++;
		backupFn = QString(modPath()+"/Data/backup%1_skeleton_bodies_.xml").arg(backupN);
	} while (QFile(backupFn).exists());
	QMessageBox::StandardButton ret;
	ret = QMessageBox::question(this, "OpenBrf",
	                            tr("There are %1 sets of hitboxes, for skeletons:").arg(nb)
	                            +allnames
	                            +tr("<br /><br />Save them inside %2?").arg(fn)
	                            +((backupN)?tr("<br /><br />A backup will be saved in %2").arg(backupFn):QString(""))
	                            +hitboxExplaination(),QMessageBox::Ok|QMessageBox::Cancel

	                            );

	if (ret!=QMessageBox::Ok) return false;
	if (backupN) {
		QFile(fn).copy(backupFn);
	}

	int res = hitboxSet.SaveHitBoxesToXml(lastSkeletonBodiesXmlPath.toStdWString().c_str(), fn.toStdWString().c_str());

	if (res!=1) {
		QMessageBox::warning(this,"OpenBrf",
		                     tr("Error saving hitbox data:\n\n%1\n\n").arg(BrfData::LastHitBoxesLoadSaveError())
		                     );
		return false;
	}
	lastSkeletonBodiesXmlPath = fn;
	setModifiedHitboxes(false);

	QMessageBox::information(this,"OpenBrf",
	                         tr("Saved hitboxes (with the other metadata) for %1 skeletons inside\nfile %2\n\n").arg(nb).arg(fn)
	                         );

	return false;
}

static void setFlags(unsigned int &f, const QString q){
	bool ok;
	f= q.trimmed().replace("0x","").toUInt(&ok,16);
}
static void setFlags(unsigned int &f, QLineEdit *q){
	if (!q->hasFrame() && q->text().isEmpty()) return;
	q->setFrame(true);
	setFlags(f,q->text());
}
static void setUInt(unsigned int &f, const QLineEdit *q){
	f= q->text().trimmed().toUInt();
}
static void setInt(int &f, const QLineEdit *q){
	f= q->text().trimmed().toInt();
}
static void setFloat(float &f, QLineEdit *q){
	if (!q->hasFrame() && q->text().isEmpty()) return;
	q->setFrame(true);
	f= q->text().trimmed().toFloat();
}
/*
static void setInt(int &f, QSpinBox *q){
	if (!q->hasFrame() && q->text().isEmpty()) return;
	q->setFrame(true);
	f= q->value();
}*/
static void setSign(int &s, const QLineEdit *q){
	int i =q->text().trimmed().toInt();
	if (i>=0) s=+1; else s=-1;
}
static void setString(char* st, QString s){
	s.truncate(254);
	sprintf(st,"%s",s.trimmed().toAscii().data());
}
static void setString(char* st, QLineEdit *q){
	if (!q->hasFrame() && q->text().isEmpty()) return;
	q->setFrame(true);
	setString(st, q->text());
}

template<class T>
static void _setFlag(vector<T> &v, int i, QString st){
	assert (i>=0 && i<(int)v.size());
	setFlags(v[i].flags , st);
}

template< class T >
static bool _swap(vector<T> &t, int i, int j){
	if (i<0 || j<0 || i>=(int)t.size() || j>=(int)t.size()) return false;
	T tmp; tmp=t[i]; t[i]=t[j]; t[j]=tmp; return true;
}
template< class T >
static bool _dup(vector<T> &t, int i){
	if (i<0 || i>=(int)t.size()) return false;
	t.insert(t.begin()+i,t.begin()+i,t.begin()+i+1);
	sprintf(t[i+1].name,"copy_%s",t[i].name);
	return true;
}
template< class T >
static bool _dupNN(vector<T> &t, int i){
	if (i<0 || i>=(int)t.size()) return false;
	t.insert(t.begin()+i,t.begin()+i,t.begin()+i+1);
	return true;
}
template< class T >

static unsigned int _del(vector<T> &t, const QModelIndexList &l){
	vector<bool> killme(t.size(),false);

	for (int k=0; k<l.size(); k++) {
		int i=l[k].row();
		if (i<0 || i>=(int)t.size()) continue;
		killme[i] = true;
	}
	uint j=0;
	for (uint i=0; i<t.size(); i++) {
		if (i!=j) t[j]=t[i];
		if (!killme[i]) j++;
	}
	t.resize(j);

	return t.size();
}

template< class T >
static char* _getName(T &t, int i){
	if (i<0 || i>=(int)t.size()) return NULL;
	return t[i].name;
}
template< class T >
void _setName(T &t, QString s){
	s.truncate(254);
	sprintf(t.name, "%s", s.toAscii().data());
}
template<>
void _setName(BrfMesh &t, QString s){
	s.truncate(254);
	sprintf(t.name, "%s", s.toAscii().data());
	t.AnalyzeName();
}
void _setNameOnCharStar(char* st, QString s){
	s.truncate(254);
	sprintf(st, "%s", s.toAscii().data());
}


template< class T >
static bool _copy(vector<T> &t, const QModelIndexList &l, vector<T> &d){
	for (int k=0; k<l.size(); k++) {

		int i=l[k].row();
		if (i<0 || i>=(int)t.size()) continue;
		d.push_back(t[i]);
		/*
		if (T::tokenIndex()==TEXTURE) {
			// remove ".dds"
			QString name(t[i].name);
			name.truncate(name.lastIndexOf("."));
		}*/

	}

	return true;
}

template< class T >
bool _compareName(const T &ta, const T &tb){
	return strcmp(ta.name,tb.name)<0;
}

template< class T >
static bool _sort(vector<T> &t){
	std::sort(t.begin(),t.end(),_compareName<T>);
	return true;
}

void MainWindow::updateTextureAccessDup(){
	int i = selector->firstSelected();
	int j = guiPanel->getCurrentSubpieceIndex(SHADER);
	if (i>=0 && j>=0) {
		_dupNN( brfdata.shader[i].opt, j );
		guiPanel->updateShaderTextaccSize();
		setModified();
		undoHistoryAddEditAction();
	}
}
void MainWindow::updateTextureAccessDel(){
	int i = selector->firstSelected();
	int j = guiPanel->getCurrentSubpieceIndex(SHADER);
	if (i>=0 && j>=0) {
		_del( brfdata.shader[i].opt, selector->selectedList() );
		guiPanel->updateShaderTextaccSize();
		setModified();
		undoHistoryAddEditAction();
	}
}
void MainWindow::updateTextureAccessAdd(){
	int i = selector->firstSelected();
	if (i>=0) {
		BrfShaderOpt o;
		brfdata.shader[i].opt.push_back(o);
		guiPanel->updateShaderTextaccSize();
		setModified();
		undoHistoryAddEditAction();
	}
}

void MainWindow::updateDataShader(){
	int sel=selector->firstSelected();
	if (sel>(int)brfdata.shader.size()) return;
	if (sel<0) return;
	BrfShader &s(brfdata.shader[sel]);
	Ui::GuiPanel* u = guiPanel->ui;
	setFlags(s.flags, u->leShaderFlags);
	setString(s.technique, u->leShaderTechnique);
	setFlags(s.requires, u->leShaderRequires);
	setString(s.fallback, u->leShaderFallback);

	int ta =guiPanel->getCurrentSubpieceIndex(SHADER);

	if (ta>=0 && ta<(int)s.opt.size()) {
		setUInt(s.opt[ta].colorOp, u->leShaderTaColorOp);
		setUInt(s.opt[ta].alphaOp, u->leShaderTaAlphaOp);
		setFlags(s.opt[ta].flags, u->leShaderTaFlags);
		setInt(s.opt[ta].map, u->leShaderTaMap);
	}
	setModified();
	undoHistoryAddEditAction();
}

void MainWindow::updateDataBody(){
	int sel=selector->firstSelected();
	if (sel>(int)brfdata.body.size()) return;
	if (sel<0) return;
	BrfBody &b(brfdata.body[sel]);
	Ui::GuiPanel* ui = guiPanel->ui;

	int ta =guiPanel->getCurrentSubpieceIndex(BODY);
	if (ta>=0 && ta<(int)b.part.size()) {
		BrfBodyPart &p(b.part[ta]);
		bool wasEmptyThen = p.IsEmpty();
		setFlags(p.flags, ui->leBodyFlags);
		setSign(p.ori, ui->leBodySign);
		switch (p.type)
		{
		case BrfBodyPart::CAPSULE:
			setFloat(p.dir.X(),ui->leBodyBX);
			setFloat(p.dir.Y(),ui->leBodyBY);
			setFloat(p.dir.Z(),ui->leBodyBZ);
		case BrfBodyPart::SPHERE:
			// fallthrough
			setFloat(p.center.X(),ui->leBodyAX);
			setFloat(p.center.Y(),ui->leBodyAY);
			setFloat(p.center.Z(),ui->leBodyAZ);
			setFloat(p.radius,ui->leBodyRad);
			break;
		default: break;
		}
		bool isEmptyNow = p.IsEmpty();
		if (wasEmptyThen!=isEmptyNow) {
			guiPanel->updateBodyPartData();
		}
		setModified();
		updateGl();
		undoHistoryAddEditAction();
	}
}

void MainWindow::onChangeMeshMaterial(QString st){
	if (!glWidget) return;
	int n=0;

	QModelIndexList list=selector->selectedList();
	Ui::GuiPanel* u = guiPanel->ui;

	for (int i=0; i<(int)list.size(); i++) {
		int sel = list[i].row();
		if (sel<0 || sel>=(int)brfdata.mesh.size()) continue;
		setString(brfdata.mesh[sel].material, u->boxMaterial);
		n++;
	}
	statusBar()->showMessage( tr("Set %1 mesh materials to \"%2\"")
	                          .arg(n).arg(u->boxMaterial->text()),5000 );
	updateGl();
	setModified();
	undoHistoryAddEditAction();

}


void MainWindow::updateDataMaterial(){
	QModelIndexList list=selector->selectedList();
	Ui::GuiPanel* u = guiPanel->ui;

	for (int i=0; i<(int)list.size(); i++) {
		int sel = list[i].row();
		if (sel<0 || sel>=(int)brfdata.material.size()) continue;

		BrfMaterial &m(brfdata.material[sel]);

		setFlags(m.flags, u->leMatFlags);
		setString(m.bump, u->leMatBump);
		setString(m.diffuseA, u->leMatDifA);
		setString(m.diffuseB, u->leMatDifB);
		setString(m.enviro, u->leMatEnv);
		setFloat(m.r, u->leMatR);
		setFloat(m.g, u->leMatG);
		setFloat(m.b, u->leMatB);
		setFloat(m.specular, u->leMatCoeff);
		setString(m.spec, u->leMatSpec);
		setString(m.shader, u->leMatShader);
		//int ro;
		//setInt(ro, u->leMatRendOrd);
		//m.SetRenderOrder(ro);

		//mapMT[m.name] = m.diffuseA;
	}
	updateGl();
	setModified();
	undoHistoryAddEditAction();
}

void MainWindow::onChangeTimeOfFrame(QString time){
	if (!glWidget) return;
	if (!selector) return;
	int i=selector->firstSelected();
	int j=currentDisplayFrame();
	int fl  =  time.toInt();

	if (selector->currentTabName()==MESH) {
		if (i>=0 && j>=0 && i<(int)brfdata.mesh.size() && j<(int)brfdata.mesh[i].frame.size()) {
			brfdata.mesh[i].frame[j].time=fl;
		}
	} else if (selector->currentTabName()==ANIMATION) {
		if (i>=0 && j>=0 && i<(int)brfdata.animation.size() && j<(int)brfdata.animation[i].frame.size()) {
			brfdata.animation[i].frame[j].index=fl;
			guiPanel->ui->boxAniMinFrame->display( brfdata.animation[i].FirstIndex() );
			guiPanel->ui->boxAniMaxFrame->display( brfdata.animation[i].LastIndex() );
		}
	} else assert(0);
	statusBar()->showMessage( tr("Set time of frame %1 to %2").arg(j).arg(fl),3000 );
	guiPanel->frameTime[j]=fl;
	setModified();

	undoHistoryAddEditAction();
}

void MainWindow::tld2mabArmor(){
}

void MainWindow::tldMakeDwarfBoots(){
	int sdi=reference.Find("skel_dwarf",SKELETON);
	int shi=reference.Find("skel_human",SKELETON);
	if (sdi==-1 || shi==-1) {
		QMessageBox::information(this,
		                         "OpenBRF","Cannot find skel_human, skel_dwarf and skel_orc in reference data.\n"
		                         );
		return;
	}
	BrfSkeleton &sd (reference.skeleton[sdi]);
	BrfSkeleton &sh (reference.skeleton[shi]);
	//vector<BrfMesh> res;
	for (int ii=0; ii<selector->selectedList().size(); ii++) {
		int i = selector->selectedList()[ii].row();
		if (i<0) continue;
		if (i>(int)brfdata.mesh.size()) continue;

		BrfMesh &m (brfdata.mesh[i]);

		m.ReskeletonizeHuman( sh, sd , 0.05);
		m.Scale(1.00,1.00,1,1,0.9,0.95);
		//m.TowardZero(0.008,0,0);

		QString tmp= QString("%1").arg(m.name);

		int indof = tmp.indexOf(".",0); if (indof == -1) indof = tmp.length();
		QString tmp2 = tmp.left(indof)+"_dwarf"+tmp.right(tmp.length()-indof);
		m.SetName(tmp2.toAscii().data());

		//res.push_back(m);
		setModified();
	}
	/*  for (unsigned int k=0; k<res.size(); k++) {
		insert(res[k]);
	}*/
	updateGui();
	updateSel();
}

void MainWindow::tldMakeDwarfSlim(){
	int sdi=reference.Find("skel_dwarf",SKELETON);
	if (sdi==-1) {
		QMessageBox::information(this,
		                         "OpenBRF",tr("CAnnot find skel_human, skel_dwarf and skel_orc in reference data.\n")
		                         );
		return;
	}
	BrfSkeleton &sd (reference.skeleton[sdi]);
	for (int ii=0; ii<selector->selectedList().size(); ii++) {
		int i = selector->selectedList()[ii].row();
		if (i<0) continue;
		if (i>(int)brfdata.mesh.size()) continue;
		BrfMesh &m(brfdata.mesh[i]);
		if (m.frame.size()<4) return;

		m.frame[1].MakeSlim(0.95,0.95,&sd);
		setModified();
		updateGl();
	}

}

void MainWindow::tldGrassAlphaPaint(){
	if (selector->currentTabName()!=MESH) return;
	bool done = false;
	for (int ii=0; ii<selector->selectedList().size(); ii++) {
		int i = selector->selectedList()[ii].row();
		if (i<0) continue;
		if (i>(int)brfdata.mesh.size()) continue;
		brfdata.mesh[i].paintAlphaAsZ(0,0.5);
		done = true;
	}
	if (done) {
		setModified();
		updateGl();
		updateGui();
	}
}

void MainWindow::tldShrinkAroundBones(){
	BrfSkeleton *skel  = currentDisplaySkeleton();
	if (!skel) return;

	int done =0;
	for (int ii=0; ii<selector->selectedList().size(); ii++) {
		int i = selector->selectedList()[ii].row();
		if (i<0) continue;
		if (i>(int)brfdata.mesh.size()) continue;
		BrfMesh &m(brfdata.mesh[i]);
		int frame = guiPanel->getCurrentSubpieceIndex(MESH);
		if (frame<0 || frame>=(int)m.frame.size()) {
			statusBar()->showMessage(tr("Invalid frame %1").arg(frame));
			continue;
		}

		m.ShrinkAroundBones(*skel,frame);
		setModified();
		updateGl();
		done++;
	}
	statusBar()->showMessage(tr("%1 meshes shrunk around bones").arg(done));

}

void MainWindow::mab2tldArmor(){
	int k=0;
	//int shi=reference.Find("skel_orc_tall",SKELETON);
	int shi=reference.Find("skel_human",SKELETON);
	int sdi=reference.Find("skel_dwarf",SKELETON);
	int soi=reference.Find("skel_orc",SKELETON);
	if (shi==-1 || sdi==-1  || soi==-1) {
		QMessageBox::information(this,
		                         "OpenBRF",tr("Cannot find skel_human, skel_dwarf and skel_orc in reference data.\n")
		                         );
		return;
	}
	BrfSkeleton &sh (reference.skeleton[shi]);
	BrfSkeleton &sd (reference.skeleton[sdi]);
	BrfSkeleton &so (reference.skeleton[soi]);

	for (int ii=0; ii<selector->selectedList().size(); ii++) {
		int i = selector->selectedList()[ii].row()+k;
		if (i<0) continue;
		if (i>(int)brfdata.mesh.size()) continue;

		BrfMesh &m(brfdata.mesh[i]);
		BrfMesh fem;
		int lst = m.frame.size()-1;
		bool usefem = false;
		if (m.frame.size()==3) {
			usefem = true;
			fem=m;
			fem.KeepOnlyFrame(1);
			lst = 2;
		}
		if (m.frame.size()==5) {
			usefem = true;
			fem=m;
			fem.KeepOnlyFrame(3);
			lst = 4;
		}

		BrfMesh ml = m;  // last frame
		ml.KeepOnlyFrame(lst);// or 0

		m.KeepOnlyFrame( 0 );

		BrfMesh md = m; // dwarf mesh
		md.ReskeletonizeHuman( sh, sd , 0.05); // 0.05 = big arms!
		float t[16]={1,0,0,0, 0,1,0,0, 0,0,1.25,0, 0,0,0,1};
		md.Transform(t); // fat dwarf!
		m.AddFrameDirect(md);

		BrfMesh mo = m; // orc mesh
		mo.ReskeletonizeHuman( sh, so , 0.00);
		m.AddFrameDirect(mo);

		if (usefem) m.AddFrameDirect(fem); // feminine mesh

		m.AddFrameDirect(ml); // last frame



		m.AdjustNormDuplicates();
		setModified();
	}
	selector->updateData(brfdata);

}

void  MainWindow::mab2tldHead(){
	tldHead(1);
}
void  MainWindow::tld2mabHead(){
	tldHead(-1);
}
void  MainWindow::tldHead(float verse){
	bool changed = false;
	//vcg::Point3f tldHead(0,0,0);//0.0370, -0.01);
	//vcg::Point3f tldOrcHead(0,0.054033, -0.064549); // from horiz neck to straing neck
	vcg::Point3f tldOrcHead(0,-0.0390, 0.01); // from human to orc... almost halfway OLD


	//vcg::Point3f tldOrcHead(0,-0.0490, 0.01); // from human to orc... almost halfway
	vcg::Point3f tldOrcHeadS(0,tldOrcHead[2], -tldOrcHead[1]);
	vcg::Point3f zero(0,0,0);
	//int torax = 8;
	int head = 9;
	if (selector->currentTabName()==SKELETON) {
		int j = selector->firstSelected();
		if (j<0) return;


		//BrfAnimation &a(reference.animation[0]);
		BrfSkeleton &s(brfdata.skeleton[j]);

		//VcgMesh::add(s);
		//VcgMesh::moveBoneInSkelMesh(head,vcg::Point3f(0,tldOrcHead[2],tldOrcHead[1]));
		//VcgMesh::modifyBrfSkeleton(s);

		//Point3f v = tldHead*verse;
		//if (QString(s.name).contains("orc"));
		Point3f v= tldOrcHead*verse;
		//v = BrfSkeleton::adjustCoordSyst(v);

		//Matrix44f mat = (
		//  BrfSkeleton::adjustCoordSystHalf(
		//    s.GetBoneMatrices()[torax].transpose()
		//  ).transpose()
		//);

		//mat = vcg::Inverse(mat);
		//mat.transposeInPlace();

		//v = mat*v - mat*zero;
		v = BrfSkeleton::adjustCoordSyst(v);
		s.bone[head].t += v;
		changed=true;
	}
	if (selector->currentTabName()==MESH) {
		for (int j=0; j<selector->selectedList().size(); j++) {
			int i  =(selector->selectedList()[j]).row();
			BrfMesh &m(brfdata.mesh[i]);
			m.Translate(-tldOrcHead*verse);
			changed=true;
		}
	}

	if (changed) {
		setModified();
		updateGl();
	}

}

void MainWindow::onChangeFlags(QString st){
	if (!glWidget) return;
	//int n=1;
	//unsigned int fl  =  st.toUInt();
	st.truncate(254);
	int i=selector->firstSelected();

	switch(selector->currentTabName()) {
	case MESH:
		for (int j=0; j<selector->selectedList().size(); j++) {
			_setFlag(brfdata.mesh, (selector->selectedList()[j]).row(), st);
		}
		break;
	case TEXTURE:
		//_setFlag(brfdata.texture,i,st);
		for (int j=0; j<selector->selectedList().size(); j++) {
			_setFlag(brfdata.texture, (selector->selectedList()[j]).row(), st);
		}
		break;
	case SKELETON: _setFlag(brfdata.skeleton,i,st); break;
		//case ANIMATION: _setFlag(brfdata.animation,i,fl); break;
	default: return; //assert(0);
	}
	statusBar()->showMessage( tr("Set flag(s) to \"%1\"").arg(st) );
	setModified();
	undoHistoryAddEditAction();
}

int MainWindow::getLanguageOption(){
	QSettings *settings;
	settings = new QSettings("mtarini", "OpenBRF");

	QVariant s =settings->value("curLanguage");
	if (s.isValid()) return  s.toInt(); return  0;
}

void MainWindow::openModuleIniFile(){
	QDesktopServices::openUrl(QUrl::fromLocalFile(modPath()+"/module.ini"));
}

QString MainWindow::referenceFilename(bool modSpecific) const
{
	if (!modSpecific) return QCoreApplication::applicationDirPath()+"/reference.brf";
	else return  modPath()+"/reference.brf";
}

// just a replacement for reference data: from "skinA_(...)" to "skinA.(...)
static void quickHackFixName( BrfData &ref ){
	for (uint i=0; i<ref.mesh.size(); i++) {
		BrfMesh &m(ref.mesh[i]);
		if ((m.name[5]=='_') && (m.name[0]=='S')) {
			m.name[5]='.';
			m.name[0]='s';
			m.AnalyzeName();
		}
	}
}

void MainWindow::refreshReference(){
	bool loaded = false;
	if (usingModReference()) {
		// attempt to use module spcific folder
		QString fn = referenceFilename(1);
		//qDebug("Trying to load '%s'",fn.toAscii().data());
		if (reference.Load(fn.toStdWString().c_str())) {
			loadedModReference = true;
			loaded = true;
			quickHackFixName( reference );
		}
	}
	if (!loaded) {
		loadedModReference = false;
		QString fn = referenceFilename(0);
		//qDebug("Trying to standard load '%s'",fn.toAscii().data());
		if (reference.Load(fn.toStdWString().c_str()))  loaded = true;
		quickHackFixName( reference );( reference );
	}
	if (loaded) {
		guiPanel->setReference(&reference);
		updateGui();
	}
}

MainWindow::MainWindow(QWidget *parent):QMainWindow(parent),inidata(brfdata)
{
	setWindowIcon(QIcon(":/openBrf.ico"));

	usingWarband = true; // until proven otherwise
	useAlphaCommands = false;

	settings = new QSettings("mtarini", "OpenBRF");

	repeatableAction = 0;
	setNextActionAsRepeatable = false;

	glWidget = new GLWidget(this,inidata);
	selector = new Selector(this);
	selector->reference=&reference;

	isModified=false;
	isModifiedHitboxes=false;
	executingRepeatedCommand = false;
	createMiniViewOptions();
	guiPanel = new GuiPanel( this, inidata);

	connect(menuBar(), SIGNAL(triggered(QAction*)), this, SLOT(undoHistoryAddAction(QAction*)) );

	createActions();
	createMenus();

	QSplitter* main = new QSplitter();

	loadOptions();

	undoHistoryRing.resize(25); // that many undo levels

	setEditingRef(false);

	setEditingVertexData( false);

	main->addWidget(selector);
	main->addWidget(guiPanel);
	main->addWidget(glWidget);

	cancelNavStack();

	createConnections();

	setCentralWidget(main);

	updateTitle();

	glWidget->selected=1;
	glWidget->data = guiPanel->data = &brfdata;
	glWidget->reference = &reference;
	glWidget->hitBoxes = &hitboxSet;

	refreshReference();

	guiPanel->hitBoxes = &hitboxSet;

	//tryLoadMaterials();

	this->setAcceptDrops(true);


	updatePaths();
	setLocale(QLocale::system());


	if (optionFeminizerUseDefault->isChecked()) optionFemininzationUseDefault();
	else optionFemininzationUseCustom();

	loadCarryPositions();

	setLanguage( curLanguage );

	glWidget->setDefaultBgColor(background,true);

	undoHistoryClear();

	connect(this->menuBar(), SIGNAL(triggered(QAction*)),this, SLOT(onActionTriggered(QAction *)));


	// create askTransofrDialog windows
	askTransformDialog = new AskTransformDialog(this );
	askTransformDialog->matrix = glWidget->extraMatrix;
	askTransformDialog->setApplyToAllLoc( &( glWidget->applyExtraMatrixToAll ) );
	connect(askTransformDialog,SIGNAL(changed()),glWidget,SLOT(update()));

	askUvTransformDialog = new AskUvTransformDialog(this);
	connect(askUvTransformDialog,SIGNAL(changed()),this,SLOT(meshUvTransformDoIt()));

}


bool MainWindow::setEditingVertexData(bool mode){
	editingVertexData = mode;
/*	enterVertexDataMode->setVisible( !mode );
	exitVertexDataMode->setVisible( mode );
	guiPanel->setEditingVertexData( mode );*/
	return mode;
}


bool MainWindow::setEditingRef(bool mode){
	if (editingRef!=mode) {
		undoHistoryClear();
		setNotModified();
	}
	editingRef = mode;
	if (editingRef) {
		editRefAct->setText(tr("Stop editing reference data"));
		editRefAct->setStatusTip(tr("Stop editing \"reference\" skeletons, animations & meshes, that OpenBrf uses to display data."));
	} else {
		editRefAct->setText(tr("Edit reference data"));
		editRefAct->setStatusTip(tr("Edit \"reference\" skeletons, animations & meshes, that OpenBrf uses to display data."));
	}
	glWidget->setEditingRef(mode);
	return true;
}

void MainWindow::insert(const BrfMesh &o){ insert( brfdata.mesh, o); }
void MainWindow::insert(const BrfSkeleton &o){ insert( brfdata.skeleton, o); }
void MainWindow::insert(const BrfAnimation &o){ insert( brfdata.animation, o); }
void MainWindow::insert(const BrfTexture &o){   insert( brfdata.texture, o); }
void MainWindow::insert(const BrfMaterial &o){  insert( brfdata.material, o); }
void MainWindow::insert(const BrfShader &o){   insert( brfdata.shader, o); }
void MainWindow::insert(const BrfBody &o){   insert( brfdata.body, o); }

void MainWindow::insertOrReplace(const BrfMesh &o){ insertOrReplace( brfdata.mesh, o); }
void MainWindow::insertOrReplace(const BrfSkeleton &o){ insertOrReplace( brfdata.skeleton, o); }
void MainWindow::insertOrReplace(const BrfAnimation &o){ insertOrReplace( brfdata.animation, o); }
void MainWindow::insertOrReplace(const BrfTexture &o){   insertOrReplace( brfdata.texture, o); }
void MainWindow::insertOrReplace(const BrfMaterial &o){  insertOrReplace( brfdata.material, o); }
void MainWindow::insertOrReplace(const BrfShader &o){   insertOrReplace( brfdata.shader, o); }
void MainWindow::insertOrReplace(const BrfBody &o){   insertOrReplace( brfdata.body, o); }

template <class BrfType>
void MainWindow::replaceInit(BrfType &o){
	BrfType& curr = getUniqueSelected<BrfType>();
	if (&curr) {
		sprintf( o.name, curr.name );
		curr = o;
	}
	setModified();
	updateGl();
	updateGui();
}

void MainWindow::replace(BrfMesh &o     ){ replaceInit(o); }
void MainWindow::replace(BrfSkeleton &o ){ replaceInit(o); }
void MainWindow::replace(BrfAnimation &o){ replaceInit(o); }
void MainWindow::replace(BrfTexture &o  ){ replaceInit(o); }
void MainWindow::replace(BrfMaterial &o ){ replaceInit(o); }
void MainWindow::replace(BrfShader &o   ){ replaceInit(o); }
void MainWindow::replace(BrfBody &o     ){ replaceInit(o); }


template<class BrfType> void MainWindow::insert( vector<BrfType> &v, const BrfType &o){
	int newpos;
	if (selector->currentTabName()!=BrfType::tokenIndex() ) {
		v.push_back(o);
		newpos=v.size()-1;
	} else {
		int i = selector->lastSelected()+1;
		if (i<0 || i>=(int)v.size()) i=v.size();
		if (i==(int)v.size()) v.push_back(o); else
			v.insert( v.begin()+i, o);
		newpos=i;
	}
	inidataChanged();
	updateSel();
	selectOne(BrfType::tokenIndex(), newpos);
}

template <class T> int _findByName( const vector<T> &v, const QString &s){
	for (unsigned int i=0; i<v.size(); i++)
		if (QString::compare(v[i].name,s,Qt::CaseInsensitive)==0) return i;
	return -1;
}

template<class BrfType> void MainWindow::insertOrReplace( vector<BrfType> &v, const BrfType &o){
	QString st(o.name);
	int i = _findByName(v,st);
	if (i>=0) {
		v[i]=o;
		inidataChanged();
		updateSel();
		selectOne(BrfType::tokenIndex(), i);
	}
	else insert(v,o);
}

bool MainWindow::addNewUiPicture(){
	AskNewUiPictureDialog d(this);
	d.setBrowsable(mabPath+"/Modules/"+modName+"/Textures");
	int ok=d.exec();
	if (ok){
		if (AskNewUiPictureDialog::name[0]==0) return false;
		BrfMaterial mat;
		BrfTexture tex;
		BrfMesh mes;

		mat.SetDefault();
		tex.SetDefault();
		mes.SetDefault();

		mes.MakeSingleQuad(
		      AskNewUiPictureDialog::px*0.01,
		      AskNewUiPictureDialog::py*0.01*0.75,
		      AskNewUiPictureDialog::sx*0.01,
		      AskNewUiPictureDialog::sy*0.01*0.75);

		switch (AskNewUiPictureDialog::overlayMode){
		case 0: mat.flags = 0x301; break; // darken
		case 1: mat.flags = 0x101; break;
		case 2: mat.flags = 0x0; break; // solild: only no fog
			// add 0x10 for no depth
		}

		sprintf(mat.name,"%s",AskNewUiPictureDialog::name);
		sprintf(tex.name,"%s.%s",
		        AskNewUiPictureDialog::name,
		        d.ext.toAscii().data());
		sprintf(mes.name,"%s",AskNewUiPictureDialog::name);
		sprintf(mat.diffuseA,"%s",AskNewUiPictureDialog::name);
		sprintf(mes.material,"%s",AskNewUiPictureDialog::name);
		if (AskNewUiPictureDialog::replace) {
			insertOrReplace(mat);
			insertOrReplace(tex);
			insertOrReplace(mes);
		} else {
			insert(mat);
			insert(tex);
			insert(mes);
		}
		setModified();
		return true;
	}
	return false;
}



template<class TT>
bool MainWindow::addNewGeneral(QStringList defaultst ){


	int tok = TT::tokenIndex();

	QString alsoAdd;
	if (tok==MATERIAL) alsoAdd = QString(tokenBrfName[TEXTURE ]).toLower();
	if (tok==TEXTURE ) alsoAdd = QString(tokenBrfName[MATERIAL]).toLower();

	AskTexturenameDialog d(this, alsoAdd);
	if (tok==MATERIAL || tok==TEXTURE)
		d.setBrowsable(mabPath+"/Modules/"+modName+"/Textures");
	//d.setLabel( tr("Name:") );
	if (defaultst.empty())
		d.setDef(tr("new_%1").arg(QString(tokenBrfName[tok]).toLower()) );
	else
		d.setRes(defaultst);

	d.setWindowTitle( tr("New %1").arg( IniData::tokenFullName(tok)  ) );


	int ok=d.exec();
	if (ok) {
		QStringList newName=d.getRes();

		for (int i=0; i<newName.size(); i++) {
			TT m;
			_setName(m,newName[i]);
			m.SetDefault();
			insert(m);
			if (d.alsoAdd()) {
				if (tok==MATERIAL) {
					BrfTexture t;
					_setName(t,newName[i]);
					t.SetDefault();
					insert(t);
				}
				if (tok==TEXTURE) {
					BrfMaterial t;
					_setName(t,newName[i]);
					t.SetDefault();
					insert(t);
				}
			}
			setModified();
		}
		return true;

	}

	return false;
}
bool MainWindow::addNewMaterial(){

	if (addNewGeneral<BrfMaterial>( QStringList() )){
		/*
		int i=selector->currentIndex();
		if (i>=0 && i<(int)brfdata.material.size()) {
			BrfMaterial &m(brfdata.material[i]);
			mapMT[m.name] = m.diffuseA;
		}*/
		glWidget->showMaterialDiffuseA();
		return true;
	}
	return false;
}
bool MainWindow::addNewTexture(){
	return addNewGeneral<BrfTexture>( QStringList() );
}
bool MainWindow::addNewShader(){
	return addNewGeneral<BrfShader>( QStringList() );
}

void MainWindow::applyAfterMeshImport(BrfMesh &m){
	m.AnalyzeName();
	switch (this->afterMeshImport()) {
	case 1:
		m.UnifyPos();
		m.UnifyVert(true,0.995); // cazz
		break;
	case 2:
		m.UnifyPos();
		m.UnifyVert(false);
		m.ComputeNormals();
		break;
	}
}

int MainWindow::gimmeASkeleton(int howManyBones){
	return reference.getOneSkeleton(howManyBones, glWidget->getRefSkeleton() );
}

int MainWindow::askRefSkin(){
	QStringList items;
	int n = reference.GetFirstUnusedLetter();
	if (n==0) {
		QMessageBox::information(this,
		                         "OpenBRF",
		                         tr("Oops... no skin is currently available...\n")
		                         );
		return -1;
	}
	for (int i=0; i<n; i++)
		items.append(tr("Skin %1").arg( char('A'+i) ));

	bool ok;
	QString resSt = QInputDialog::getItem(
	      this, tr("Select a skin"),
	      tr("Select a skin:"), items, 0, false, &ok
	      );

	if (ok && !resSt.isEmpty())
		return resSt.toAscii().data()[5]-'A';
	else return -1;
}

int MainWindow::currentDisplaySkin(){
	return glWidget->getRefSkin();
}

BrfSkeleton* MainWindow::currentDisplaySkeleton(){
	int i = glWidget->getRefSkeleton();
	if ( (i<0) || (i>=(int)reference.skeleton.size()) ) {
		QMessageBox::warning(this,"OpenBRF",tr("Select a skeleton\nin the view panel first"));
		return NULL;
	}
	return &(reference.skeleton[i]);
}

BrfAnimation* MainWindow::currentDisplayAnimation(){
	int i = glWidget->getRefSkelAni();
	if ( (i<0) || (i>=(int)reference.animation.size()) ) {
		QMessageBox::warning(this,"OpenBRF",tr("Select an animation\nin the view panel first"));
		return NULL;
	}
	return &(reference.animation[i]);
}
int MainWindow::currentDisplayFrame(){
	if (selector->currentTabName()==MESH)
		return guiPanel->ui->frameNumber->value(); //glWidget->getFrameNumber();
	if (selector->currentTabName()==ANIMATION)
		return guiPanel->ui->frameNumberAni->value()-1;
	assert(0);
	return 0;
}

int MainWindow::currentDisplaySkelAniFrame(){
	assert ((selector->currentTabName()==MESH));
	return glWidget->lastSkelAniFrameUsed;
}


void MainWindow::meshUnify(){

	bool mod = false;
	for (int k=0; k<selector->selectedList().size(); k++) {
		int i= selector->selectedList()[k].row();
		if (i<0) continue;
		if (i>(int)brfdata.mesh.size()) continue;


		BrfMesh &m (brfdata.mesh[i]);
		if (m.RemoveUnreferenced()) mod=true;
		if (m.UnifyPos()) mod=true;
		if (m.UnifyVert(true,0.995)) mod=true;

	}
	updateGui();
	updateGl();


	if (mod) setModified();
	statusBar()->showMessage(tr("Vertex unified."), 2000);
}


void MainWindow::meshTellBoundingBox(){
	vcg::Box3f bbox;
	bbox.SetNull();
	QString objname;
	int objnum = 0;
	for (int k=0; k<selector->selectedList().size(); k++) {
		int i= selector->selectedList()[k].row();
		if (i<0) continue;
		if (i>(int)brfdata.mesh.size()) continue;
		BrfMesh &m (brfdata.mesh[i]);
		m.UpdateBBox();
		bbox.Add(m.bbox);
		objnum++;
		if (objnum==1)
			objname = QString(tr("object '%1'")).arg(m.name);
		else
			objname = QString(tr("%1 objects")).arg(objnum);
	}
	QString s = QString("(%1,%2),(%3,%4),(%5,%6)")
	    .arg(bbox.min[0]).arg(bbox.max[0])
	    .arg(bbox.min[2]).arg(bbox.max[2])
	    .arg(bbox.min[1]).arg(bbox.max[1])
	    ;
	QApplication::clipboard()->setText(s);
	QString msg = QString(tr("Spatial extension of %7:\n\nin X=%1 to %2\nin Y=%3 to %4\nin Z=%5 to %6\n\n(data copied to clipboard)"))
	    .arg(bbox.min[0]).arg(bbox.max[0])
	    .arg(bbox.min[2]).arg(bbox.max[2])
	    .arg(bbox.min[1]).arg(bbox.max[1])
	    .arg(objname)
	    ;
	QMessageBox::information(this,"OpenBRF",msg);
}

void MainWindow::meshAniSplit(){
	for (int k=0; k<selector->selectedList().size(); k++) {
		int i= selector->selectedList()[k].row();
		if (i<0) continue;
		if (i>(int)brfdata.mesh.size()) continue;

		BrfMesh &m (brfdata.mesh[i]);


		std::vector<BrfMesh> res;
		for (uint i=0; i<m.frame.size(); i++) {
			BrfMesh m2 = m;
			m2.frame[0]=m.frame[i];
			m2.frame.resize(1);
			char newName[1024];
			sprintf( newName, "%s_frame%d", m.name, i);
			m2.SetName(newName);
			res.push_back( m2 );

		}

		for (uint i=0; i<res.size(); i++) insert(res[i]);

		if (!res.size())
			statusBar()->showMessage(tr("Only one component found"), 2000);
		else
			statusBar()->showMessage(tr("Mesh separated into %1 pieces.").arg(res.size()), 2000);

		updateGui();
		updateGl();

		setModified();

		break;

	}
}


void MainWindow::meshSubdivideIntoComponents(){
	for (int k=0; k<selector->selectedList().size(); k++) {
		int i= selector->selectedList()[k].row();
		if (i<0) continue;
		if (i>(int)brfdata.mesh.size()) continue;

		BrfMesh &m (brfdata.mesh[i]);
		//m.FixRigidObjectsInRigging();

		std::vector<BrfMesh> res;
		m.SubdivideIntoConnectedComponents(res);
		if (res.size()>10) {
			if (QMessageBox::warning(
			      this, tr("OpenBrf"),
			      tr("Mesh %1 will be \nsplit in %2 sub-meshes!.\n\nProceed?").arg(m.name).arg(res.size()),
			      QMessageBox::Yes | QMessageBox::No ) == QMessageBox::No )
				break;
		}

		for (uint i=0; i<res.size(); i++) insert(res[i]);

		if (!res.size())
			statusBar()->showMessage(tr("Only one component found"), 2000);
		else
			statusBar()->showMessage(tr("Mesh separated into %1 pieces.").arg(res.size()), 2000);

		updateGui();
		updateGl();

		setModified();

		break;

	}
}

void MainWindow::meshFixRiggingRigidParts(){

	for (int k=0; k<selector->selectedList().size(); k++) {
		int i= selector->selectedList()[k].row();
		if (i<0) continue;
		if (i>(int)brfdata.mesh.size()) continue;

		BrfMesh &m (brfdata.mesh[i]);
		m.FixRigidObjectsInRigging();
	}
	updateGui();
	updateGl();


	setModified();
	statusBar()->showMessage(tr("Autofixed rigid parts."), 2000);
}


template<class BrfType>
void MainWindow::objectMergeSelected(vector<BrfType> &v){
	BrfType res;
	bool first=true;
	QString commonPrefix;
	for (int i=0; i<selector->selectedList().size(); i++)
	{
		int j=selector->selectedList()[i].row();
		if (j<0 || j>=(int)v.size()) continue;
		if (first) {
			res = v[j];
			commonPrefix = QString(v[j].name);
		} else {
			if (!res.Merge( v[j] )) {
				QMessageBox::information(this,
				                         "OpenBrf",
				                         tr("Cannot merge these meshes\n (different number of frames,\n or rigged VS not rigged).\n")
				                         );
				return;
			}
			_findCommonPrefix( commonPrefix, v[j].name );
		}
		first=false;
	}
	if (commonPrefix.endsWith(".")) commonPrefix.chop(1);
	else commonPrefix+="_combined";
	_setName(res,commonPrefix.toAscii().data());
	insert(res);
	setModified();
}

/*template<> BrfMesh& MainWindow::getSelected<BrfMesh>(int n){
		return *((BrfMesh*)NULL); //brfdata.mesh[n];
}*/

template<class BrfType> BrfType& MainWindow::getSelected(int n){

	if (BrfType::tokenIndex()!=selector->currentTabName()) return *((BrfType*)NULL);

	std::vector<BrfType> &v ( VectorOf<BrfType>(brfdata) );
	int i= selector->selectedList()[n].row();
	if (i<0) return *((BrfType*)NULL);
	if (i>=(int)v.size()) return *((BrfType*)NULL);
	return v[i];
}

template<class BrfType> BrfType& MainWindow::getUniqueSelected(){
	if (BrfType::tokenIndex()!=selector->currentTabName()) return *((BrfType*)NULL);

	std::vector<BrfType> &v ( VectorOf<BrfType>(brfdata) );
	if (selector->selectedList().size()!=1) return *((BrfType*)NULL);
	int i= selector->selectedList()[0].row();
	if (i<0) return *((BrfType*)NULL);
	if (i>(int)v.size()) return *((BrfType*)NULL);
	return v[i];
}

/*
bool MainWindow::hitboxToBody(){
	BrfSkeleton &s = getSelected<BrfSkeleton>();
	if (!&s) return false;
	int bi = hitboxSet.Find(s.name,BODY);

	if (bi<0) {
		QMessageBox::warning(this,"OpenBRF",
			tr("Unfortunately, no hit-box is known for skeleton '%1', from 'data/skeleton_bodies.xml'. Operation canceled.").arg(s.name)
		);
		return false;
	}
	BrfBody &b ( hitboxSet.body[bi] );
	BrfBody res;

	if (!s.LayoutHitboxes(b,res,false)) {
			QMessageBox::warning(this,"OpenBRF",
				tr("Num of bones and num of body-parts mismatch!")
			);
			return false;
	}
	insert(res);

	return true;
}

bool MainWindow::bodyToHitbox(){
	BrfBody &b = getSelected<BrfBody>();
	if (!&b) return false;
	int si = brfdata.Find(b.name,SKELETON);
	if (si<0) {
		QMessageBox::warning(this,"OpenBRF",
			tr("Cannot find a skeleton named '%1' in current file. Operation canceled.").arg(b.name)
		);
		return false;
	}
	BrfSkeleton &s ( brfdata.skeleton[si] );

	BrfBody res;
	if (!s.LayoutHitboxes(b,res,true)) {
			QMessageBox::warning(this,"OpenBRF",
				tr("Num of bones and num of body-parts mismatch!")
			);
			return false;
	}

	int bi = hitboxSet.Find(b.name, BODY);
	if (bi>=0) hitboxSet.body[bi] = res; // substitute hitboxes in set
	else hitboxSet.body.push_back(res); // add hitbox to set
	return true;
}

bool MainWindow::saveSkeletonHitbox(){
	return true;
}
*/

void MainWindow::hitboxEdit(int whichAttrib, int dir){

	BrfSkeleton &s = getSelected<BrfSkeleton>();
	if (!&s) return;
	int b = guiPanel->getCurrentSubpieceIndex(SKELETON);
	BrfBody *hit = hitboxSet.FindBody(s.name);
	if (b<0) return;
	if (!hit) return;
	if (b>=(int)hit->part.size()) return;

	//qDebug("Skel = %s, piece = %d, attr = %d, dir = %d!",s.name,b,whichAttrib,dir);

	float delta = 0.01;
	if (QApplication::keyboardModifiers()!=Qt::NoModifier) delta*=0.1;
	hit->part[b].ChangeAttribute(whichAttrib,delta*dir);

	setModifiedHitboxes(true);

	updateGl();
}

void MainWindow::hitboxSetRagdollOnly(bool v){
	BrfSkeleton &s = getSelected<BrfSkeleton>();
	if (!&s) return;
	int b = guiPanel->getCurrentSubpieceIndex(SKELETON);
	BrfBody *hit = hitboxSet.FindBody(s.name);
	if (b<0) return;
	if (!hit) return;
	if (b>=(int)hit->part.size()) return;
	hit->part[b].SetHitboxFlags((v)?1:0);
	setModifiedHitboxes(true);
}

void MainWindow::hitboxSymmetrize(){
	BrfSkeleton &s = getSelected<BrfSkeleton>();
	if (!&s) return;
	int b = guiPanel->getCurrentSubpieceIndex(SKELETON);
	BrfBody *hit = hitboxSet.FindBody(s.name);
	if (b<0) return;
	if (!hit) return;
	if (b>=(int)hit->part.size()) return;

	int b1 = s.FindSpecularBoneOf(b);
	if (b1!=b) {
		// make other capsule the simmetric of this one
		hit->part[b1].SymmetrizeCapsule(hit->part[b]);
	} else {
		hit->part[b].SymmetrizeCapsule();
	}

	setModifiedHitboxes(true);
	updateGl();
}

void MainWindow::hitboxActivate(bool a){
	BrfSkeleton &s = getSelected<BrfSkeleton>();
	if (!&s) return;
	int b = guiPanel->getCurrentSubpieceIndex(SKELETON);
	BrfBody *hit = hitboxSet.FindBody(s.name);
	if (b<0) return;
	if (!hit) return;
	if (b>=(int)hit->part.size()) return;

	if (!a) hit->part[b].SetEmpty();
	else hit->part[b].type = BrfBodyPart::CAPSULE;

	setModifiedHitboxes(true);
	updateGl();

}

void MainWindow::hitboxReset(){

	BrfSkeleton &s = getSelected<BrfSkeleton>();
	if (!&s) return;
	int b = guiPanel->getCurrentSubpieceIndex(SKELETON);
	BrfBody *hit = hitboxSet.FindBody(s.name);
	if (b<0) return;
	if (!hit) return;
	if (b>=(int)hit->part.size()) return;

	hit->part[b].SetAsDefaultHitbox();

	setModifiedHitboxes(true);
	updateGl();

}

bool MainWindow::loadCarryPositions(QString filename){
	QFile f(filename);
	if (!f.open(QIODevice::ReadOnly)) return false;

	carryPositionSet.clear();

	char line[1024];
	while (f.readLine(line,1023)>=0) {

		//qDebug("adding line \"%s\"...\n",line);
		if (!QString(line).trimmed().isEmpty())
		if (!(line[0]=='#')) {
			CarryPosition cp;
			if (cp.Load(line)) {
				carryPositionSet.push_back(cp);
				//qDebug("Added position %s",cp.name);
			} else {
				QMessageBox::warning(this,"OpenBRF",tr("Error loading line of file %2:\n\n%1").
				                     arg(line).arg(filename));
				return false;
			}
		}
	}
	//qDebug("done adding");
	return true;
}

bool MainWindow::loadCarryPositions(){
	const char* filename = "carry_positions.txt";
	QString fn1 = QString("%1/%2").arg(QCoreApplication::applicationDirPath()).arg(filename);
	QString fn2 = QString(":/%1").arg(filename);

	//qDebug("%s",fn1.toAscii().data());
	if (loadCarryPositions(fn1)) return true;
  if (loadCarryPositions(fn2)) return true;
	QMessageBox::warning(this,"OpenBRF",tr("Failed loading carry positions"));
	return false;
}



void MainWindow::optionFemininzationUseCustom(){
	QFile f(QCoreApplication::applicationDirPath()+"customFemininizer.morpher");
	QByteArray r;
	bool ok = false;
	if (f.open(QIODevice::ReadOnly)){
		r = f.readAll();
		if (femininizer.Load(r.data())) ok = true;
	}
	if (!ok){
		QMessageBox::warning(this, tr("OpenBrf"),
		                     QString(
		                       "Failed to load a custom feminizer mesh-morpher!\n"
		                       "using built-in feminizer morpher instead\n\n"
		                       "INFO: in order to build your user-defined feminizer mesh-morpher,"
		                       "you must first select examples of feminized armours, and then use:"
		                       "\n  [%1] -->\n  [%2] -->\n  [%3]"
		                       ).arg(optionMenu->title().replace("&",""))
		                     .arg(autoFemMenu->title())
		                     .arg(optionLearnFeminization->text())
		                     );
		optionFeminizerUseDefault->trigger();
	}
}

void MainWindow::optionFemininzationUseDefault(){
	QFile f(":/femininizer.morpher");
	QByteArray r;
	bool ok = false;
	if (f.open(QIODevice::ReadOnly)){
		r = f.readAll();
		if (femininizer.Load(r.data())) ok = true;
	}
	if (!ok){
		QMessageBox::warning(this, tr("OpenBrf"),
		                     QString("Internal mysterious error on loading built-in femininizer morpher")
		                     );
	}
}


void MainWindow::learnFemininzation(){
	int ndone = 0;

	int feminineFrame = (usingWarband)?2:1;
	int masculineFrame = (usingWarband)?1:2;

	if (selector->currentTabName()==MESH) {
		for (int k=0; k<selector->selectedList().size(); k++) {
			int i= selector->selectedList()[k].row();
			if (i<0) continue;
			if (i>(int)brfdata.mesh.size()) continue;
			BrfMesh& m (brfdata.mesh[i]);
			if (m.frame.size()!=3) continue;
			if (!ndone) femininizer.ResetLearning();
			femininizer.LearnFrom(m,feminineFrame,masculineFrame);
			ndone++;
		}
	}

	if (!ndone) {
		QMessageBox::warning(this,tr("OpenBRF"),tr("No mesh found to learn how to femininize an armour.\n\nYou must select meshes with feminine frame, and I'll try to learn the way to build a femenine frame from a given armour"));
		return;
	}

	bool accept = false;

	int breast=0, emp=0; bool ok=true;

	emp = QInputDialog::getInteger(this,"OpenBRF",tr(
	                                 "Select a emphasis factor between -100% and +100%\n\n"
	                                 "zero => normal.\n""positive => stronger effect.\n""negative => milder effect \n\n"
	                                 "(default: +15%)"),15,-300,+300,5,&ok);
	if (ok) {
		breast = QInputDialog::getInteger(this,"OpenBRF",tr("Select amount of extra breastification in mm\n(default: 13mm)"),13,0,100,1,&ok);
		if (ok)  accept = true;
	}

	if (accept) {
		femininizer.FinishLearning();
		femininizer.Emphatize(emp/100.0);
		femininizer.extraBreast = breast / 1000.0;

		femininizer.Save("customFemininizer.morpher");
		QMessageBox::information(this,tr("OpenBRF"),tr("Learnt a custom way to femininize an armour\nfrom %1 examples!").arg(ndone));
		optionFeminizerUseCustom->trigger();
	} else {
		QMessageBox::information(this,tr("OpenBRF"),tr("Canceled"));
	}

}


void MainWindow::meshFemininize(){
	if (selector->currentTabName()!=MESH) return;
	int ndone = 0;

	bool yesToAll = false;
	for (int k=0; k<selector->selectedList().size(); k++) {
		int i= selector->selectedList()[k].row();
		if (i<0) continue;
		if (i>(int)brfdata.mesh.size()) continue;
		BrfMesh& m (brfdata.mesh[i]);


		if (!yesToAll) {
			if (m.frame.size()>1) {
				int res = QMessageBox::warning(this,"OpenBRF",tr("Warning: mesh %1 has already a feminine frame %2.\n\nOverwrite it?").arg(m.name).arg(usingWarband),
				                               QMessageBox::Yes|QMessageBox::YesToAll|QMessageBox::No
				                               );
				if (res==QMessageBox::No) continue;
				if (res==QMessageBox::YesToAll) yesToAll = true;
			}
		}
		while (m.frame.size()<3) m.frame.push_back(m.frame[0]);
		int feminineFrame = (usingWarband)?2:1;
		int masculineFrame = (usingWarband)?1:2;
		m.MorphFrame(masculineFrame,feminineFrame,femininizer);

		m.ComputeNormals(feminineFrame);
		m.frame[0].time = 0;
		m.frame[1].time = (usingWarband)?0:10;
		m.frame[2].time = (usingWarband)?10:20;
		ndone++;
	}
	if (ndone){
		setModified();
		updateGui();
		updateGl();
	}

}



void MainWindow::exportNames(){
	// TODO!!!
}

void MainWindow::meshMerge(){
	objectMergeSelected(brfdata.mesh);
}

void MainWindow::bodyMerge(){
	objectMergeSelected(brfdata.body);
}

void MainWindow::updateGl(){
	glWidget->update();
}
void MainWindow::updateGui(){
	guiPanel->setSelection( selector->selectedList(),selector->currentTabName());
}
void MainWindow::updateSel(){
	selector->updateData(brfdata);

}

// these should go as private members but I'm lazy right now
static double _crease = 0.5;
static bool _keepSeams = true;

void MainWindow::meshUvTransform(){

	brfdataTmp.mesh = brfdata.mesh;
	int res = askUvTransformDialog->exec();

	if (res==QDialog::Accepted) {
		setModified();
	} else {
		brfdata.mesh = brfdataTmp.mesh;
		updateGl();
	}
}


void MainWindow::meshUvTransformDoIt(){
	float su,sv,tu,tv;
	askUvTransformDialog->getData(su,sv,tu,tv);
	for (int i=0; i<getNumSelected(); i++) {
		BrfMesh &m( getSelected<BrfMesh>(i));
		if (!&m) continue;
		m = brfdataTmp.mesh[ getSelectedIndex(i)];
		m.TransformUv(su,sv,tu,tv);
	}
	updateGl();
}

void MainWindow::meshRecomputeNormalsAndUnify_onSlider(int i){
	_crease = 1-i/100.0f*2;
	meshRecomputeNormalsAndUnifyDoIt();
}




void MainWindow::meshRecomputeNormalsAndUnify_onCheckbox(bool i){
	_keepSeams = i;
	meshRecomputeNormalsAndUnifyDoIt();
}

void MainWindow::meshRecomputeNormalsAndUnifyDoIt(){

	//int i = selector->firstSelected();

	for (int k=0; k<selector->selectedList().size(); k++) {
		int i= selector->selectedList()[k].row();
		if (i<0) continue;
		if (i>(int)brfdata.mesh.size()) return;

		BrfMesh &m (brfdata.mesh[i]);


		m.UnifyPos();

		m.DivideVert();
		m.ComputeNormals();
		m.UnifyVert(true,_crease);

		m.ComputeNormals();
		if (!_keepSeams)
			m.RemoveSeamsFromNormals(_crease);
	}
	updateGui();
	updateGl();
	statusBar()->showMessage(tr("Normals recomputed with %1% hard edges.").arg(int(100-_crease*50)), 2000);

}

void MainWindow::meshRecomputeNormalsAndUnify(){
	int i = selector->firstSelected();
	if (i<0) return;
	if (i>(int)brfdata.mesh.size()) return;

	AskCreaseDialog d(this);

	d.slider()->setValue(100-int(_crease*100));
	d.checkbox()->setChecked(_keepSeams);

	connect(d.slider(),SIGNAL(valueChanged(int)),
			this,SLOT(meshRecomputeNormalsAndUnify_onSlider(int)));
	connect(d.checkbox(),SIGNAL(clicked(bool)),
	        this,SLOT(meshRecomputeNormalsAndUnify_onCheckbox(bool)));

	//d.exec();
	std::vector<BrfMesh> backup = brfdata.mesh;
	meshRecomputeNormalsAndUnifyDoIt();

	int res = d.exec();
	//d.setModal();
	disconnect(d.checkbox(),SIGNAL(clicked(bool)),
	           this,SLOT(meshRecomputeNormalsAndUnify_onCheckbox(bool)));
	disconnect(d.slider(),SIGNAL(valueChanged(int)),
	           this,SLOT(meshRecomputeNormalsAndUnify_onSlider(int)));


	if (res==QDialog::Accepted) {
		setModified();
	} else {
		brfdata.mesh = backup;
		updateGl();
	}
}

void MainWindow::smoothenRigging(){
	QModelIndexList list= selector->selectedList();
	if (selector->currentTabName()!=MESH) return;

	int k=0;
	for (int j=0; j<list.size(); j++){
		BrfMesh &m(brfdata.mesh[list[j].row()]);
		if (m.IsRigged()) {
			m.SmoothRigging();
			k++;
		}
	}
	if (k) {
		updateGl();
		setModified();
	}
	statusBar()->showMessage(tr("Softened %1 rigged meshes!").arg(k), 2000);
}


void MainWindow::stiffenRigging(){
	QModelIndexList list= selector->selectedList();
	if (selector->currentTabName()!=MESH) return;

	int k=0;
	for (int j=0; j<list.size(); j++){
		BrfMesh &m(brfdata.mesh[list[j].row()]);
		if (m.IsRigged()) {
			m.StiffenRigging(1.2);
			k++;
		}
	}
	if (k) {
		updateGl();
		setModified();
	}
	statusBar()->showMessage(tr("Stiffened %1 rigged meshes!").arg(k), 2000);
}

void MainWindow::flip(){
	QModelIndexList list= selector->selectedList();
	switch (selector->currentTabName()) {
	case MESH:
		for (int j=0; j<list.size(); j++){
			brfdata.mesh[list[j].row()].Flip();
		}
		// test
		/*{
		std::vector<BrfRigging> &r( brfdata.mesh[list[0].row()].rigging);
		for (int i=0; i<r.size(); i++) if (r[i].boneIndex[1]==-1) {
				r[i].boneIndex[1] = r[i].boneIndex[2] = r[i].boneIndex[3] = r[i].boneIndex[0];
				r[i].boneWeight[0] = r[i].boneWeight[1] = r[i].boneWeight[2] = r[i].boneWeight[3]= 0.25;
		}

		for (int i=0; i<r.size(); i++) {
			qDebug("%d %d %d %d %f %f %f %f",
						 r[i].boneIndex[0], r[i].boneIndex[1], r[i].boneIndex[2], r[i].boneIndex[3],
						 r[i].boneWeight[0],r[i].boneWeight[1],r[i].boneWeight[2],r[i].boneWeight[3]);
		}
	}*/
		break;
	case BODY:
		for (int j=0; j<list.size(); j++){
			brfdata.body[list[j].row()].Flip();
		}
		break;
	default: return;
	}
	updateGl();
	setModified();
}

void MainWindow::scale(){
	QModelIndexList list= selector->selectedList();
	if (selector->currentTabName()==MESH && list.size()>0) {
		float sca;
		bool ok;
		sca =  QInputDialog::getInteger(this,"Set Scale Factor","[in %, >100 means larger]",100,1,1000,5,&ok);
		if (ok) {
			for (int j=0; j<list.size(); j++){
				brfdata.mesh[list[j].row()].Scale(sca/100.0);
			}
			setModified();
		}
	}
	updateGl();
}

void MainWindow::meshToBody(){
	QModelIndexList list= selector->selectedList();
	if (selector->currentTabName()==MESH && list.size()>0) {
		BrfBody b;
		BrfBodyPart bp;
		for (int j=0; j<list.size(); j++){
			BrfMesh &m (brfdata.mesh[list[j].row()]);
			m.AddToBody(bp);
			if (j==0) sprintf(b.name, m.GetLikelyCollisonBodyName() );
		}
		b.part.push_back(bp);
		b.MakeQuadDominant();
		b.UpdateBBox();

		insert(b);
		setModified();
	}

}

void MainWindow::bodyMakeQuadDominant(){
	getUniqueSelected<BrfBody>();
	QModelIndexList list= selector->selectedList();
	if (selector->currentTabName()==BODY && list.size()>0) {
		for (int j=0; j<list.size(); j++){
			brfdata.body[list[j].row()].MakeQuadDominant();
		}
		setModified();
	}
	updateGui();
	updateGl();
}

void MainWindow::shiftAni(){
	if (selector->currentTabName()!=ANIMATION) return;
	QModelIndexList list= selector->selectedList();
	for (int i=0; i<list.size(); i++){
		int j = list[i].row();
		bool ok;
		int a=brfdata.animation[j].FirstIndex();
		int b=brfdata.animation[j].LastIndex();
		int k =  QInputDialog::getInteger(this,
		                                  tr("Shift animation timings"),
		                                  tr("Current Interval: [%1 %2]\nNew interval: [%1+k %2+k]\n\nSelect k:")
		                                  .arg(a).arg(b),
		                                  0,-a,1000,1,&ok);
		if (ok) {
			brfdata.animation[j].ShiftIndexInterval(k);
			setModified();
		}
	}
	updateGui();
	updateGl();
}

void MainWindow::aniMirror(){
	if (selector->currentTabName()!=ANIMATION) return;
	QModelIndexList list= selector->selectedList();
	for (int i=0; i<list.size(); i++){
		int j = list[i].row();
		BrfAnimation &a (brfdata.animation[j]);

		// MEGAHACK
		//a.AddBoneHack(16);
		//a.AddBoneHack(17);
		//setModified();
		//continue;

		int si = glWidget->getRefSkeleton();
		if (si<0) continue;
		BrfSkeleton s =reference.skeleton[si];


		BrfAnimation b = a;
		if (a.Mirror(b,s))	setModified();
	}
	updateGl();

}

void MainWindow::aniExtractInterval(){
	if (selector->currentTabName()!=ANIMATION) return;
	int j = selector->firstSelected();
	BrfAnimation newani;

	BrfAnimation &ani(brfdata.animation[j]);
	AskIntervalDialog d(this, tr("Extract Interval"), ani.FirstIndex(), ani.LastIndex());
	if (d.exec()!=QDialog::Accepted) return;
	int res = ani.ExtractIndexInterval(newani, d.getA(),d.getB());
	if (res>0){
		insert(newani);
	}
}


void MainWindow::aniRemoveInterval(){
	if (selector->currentTabName()!=ANIMATION) return;
	int j = selector->firstSelected();
	BrfAnimation &ani(brfdata.animation[j]);
	AskIntervalDialog d(this, tr("Remove Interval"), ani.FirstIndex(), ani.LastIndex());
	if (d.exec()!=QDialog::Accepted) return;
	int res = ani.RemoveIndexInterval(d.getA(),d.getB());
	if (res>0) {
		setModified();
		updateGui();
		updateGl();
	}
}

void MainWindow::aniMerge(){
	if (selector->currentTabName()!=ANIMATION) return;
	BrfAnimation res;
	QModelIndexList list= selector->selectedList();
	for (int i=0; i<list.size(); i++){
		int j = list[i].row();
		if (i==0) res = brfdata.animation[j];
		else {
			BrfAnimation tmp = res;
			if (!res.Merge(tmp,brfdata.animation[j])) {
				QMessageBox::information(this,"OpenBrf",


										 tr("Cannot merge these animations\n (different number of bones).\n")
				                         );
				return;
			}
		}
	}
	insert(res);
}

void MainWindow::meshAniMerge(){
	if (selector->currentTabName()!=MESH) return;

	QModelIndexList list = selector->selectedList();
	std::vector<bool> sel(brfdata.mesh.size(),false);

	for (int i=0; i<list.size(); i++){
		int j = list[i].row();
		sel[j] = true;
	}

	BrfMesh res;
	for (uint i=0, first=1; i<sel.size(); i++) if (sel[i]) {
		if (first) {
			res = brfdata.mesh[i]; first = 0;
		} else {
			if (!res.AddAllFrames(brfdata.mesh[i])) {
				QMessageBox::information(this,"OpenBrf",
				                         tr("Cannot merge these meshes\n (different number of vertices, faces, points...).\n")
				                         );
				return;
			}
			QString n0 = QString("%1").arg(res.name);
			QString n1 = QString("%2").arg(res.name);
		}
	}
	int j=0;
	for (uint i=0; i<sel.size(); i++) {
		if (!sel[i]) {
			brfdata.mesh[j++] = brfdata.mesh[i];
		}
	}
	brfdata.mesh.resize(j);


	insert(res);
}

void MainWindow::aniReskeletonize(){

	int n = getNumSelected();
	int nb = -1;
	bool fail = false;
	for (int j=0; j<n; j++){
		BrfAnimation &a( getSelected<BrfAnimation>(j) );
		if (!&a) continue;
		if (nb==-1) nb = a.nbones;
		if (nb!=a.nbones) fail = true;
	}
	if (nb==-1) fail = true;
	if (fail) QMessageBox::warning(this,"OpenBRF",tr("Select one or more animation using same number of bones first"));
	if (fail) return;

	AskSkelPairDialog *d = new AskSkelPairDialog;
	d->numBoneInAni = nb;

	fail = true;
	for (int i=0; i<(int)reference.skeleton.size();i++) {
		BrfSkeleton &s( reference.skeleton[i] );
		if ((int)s.bone.size() == nb) fail = false;
		d->addSkeleton(s.name,s.bone.size(),i);
	}
	if (fail) QMessageBox::warning(this,"OpenBRF",tr("Select one or more animation using same number of bones first"));
	if (fail) return;
	bool done = false;
	if (d->exec()==QDialog::Accepted) {

		BrfSkeleton &s1( reference.skeleton[d->skelFrom()] );
		BrfSkeleton &s2( reference.skeleton[d->skelTo()] );
		std::vector<int> map = s2.Bone2BoneMap(s1);
		std::vector<vcg::Point4<float> > boneRot = s2.BoneRotations();
		qDebug("sizes = %d %d %d",s2.bone.size(),map.size(),boneRot.size());
		for (int j=0; j<n; j++){
			BrfAnimation &a( getSelected<BrfAnimation>(j) );
			if (!&a) continue;
			a.Shuffle(map,boneRot);
			done = true;
		}
	}
	if (done) {
		updateGui();
		updateGl();
		setModified();
	}


}

void MainWindow::meshFreezeFrame(){
	bool mod = false;
	for (int j=0; j<getNumSelected(); j++){
		BrfMesh &m( getSelected<BrfMesh>(j) );
		if (!&m) continue;
		//m.Unskeletonize(reference.skeleton[9]);//gimmeASkeleton(20)]);
		if (m.IsRigged()) {

			BrfSkeleton *s = currentDisplaySkeleton();
			if (!s) break;
			BrfAnimation *a = currentDisplayAnimation();
			if (!a) break;
			int f = currentDisplaySkelAniFrame();
			if (f<0) break;
			m.FreezeFrame(*s,*a,f);
			m.DiscardRigging();
			mod = true;
		}

	}
	if (mod) {
		setModified();
		updateGui();
		updateGl();
	}

}

void MainWindow::skeletonDiscardHitbox(){
	BrfSkeleton &s = getSelected<BrfSkeleton>();
	if (&s == NULL) return;
	int i = hitboxSet.Find(s.name,BODY);
	if (i<0) {
		QMessageBox::warning(this,"OpenBRF",tr("Skeleton %1 has no associated hit-box set. Canceled").arg(s.name));
		return;
	}


	if (QMessageBox::question(this,"OpenBRF",
	                          tr("Remove the hit-box associated to skeleton name %1?<br /><br />"
	                             "(this means that no skeleton named '%1' will have a hitbox, in this Module)").arg(s.name)
	                          + hitboxExplaination(),QMessageBox::Ok|QMessageBox::Cancel
	                          ) == QMessageBox::Ok) {
		hitboxSet.body.erase( hitboxSet.body.begin()+ i);
		setModifiedHitboxes(true);

		updateGui();
		updateGl();
	}

}

void MainWindow::meshDiscardRig(){
	QModelIndexList list= selector->selectedList();
	for (int j=0; j<list.size(); j++){
		BrfMesh &m(brfdata.mesh[list[j].row()]);
		//m.Unskeletonize(reference.skeleton[9]);//gimmeASkeleton(20)]);

		m.DiscardRigging();
		setModified();
	}
	updateGui();
	updateGl();
}

void MainWindow::meshRecomputeTangents(){
	QModelIndexList list= selector->selectedList();
	for (int j=0; j<list.size(); j++){
		BrfMesh &m(brfdata.mesh[list[j].row()]);
		m.ComputeTangents();
		setModified();
	}
	updateGui();
	updateGl();
}

void MainWindow::meshRecolor(){

	QColor oldColor = Qt::white;

	BrfMesh &m(getSelected<BrfMesh>(0));
	if (&m) oldColor = QColor( QRgb( m.GetAverageColor() ) );
	QString title = tr("Uniform color for mesh");

	QColor color = AskColorDialog::myGetColor(
		oldColor,
		this,
		title
		//,QColorDialog::ShowAlphaChannel
	);



	if (!color.isValid()) return;
	uint r = color.red();
	uint g = color.green();
	uint b = color.blue();
	uint a = color.alpha();

	uint col = b | (g<<8) | (r<<16) | (a<<24);
	bool overwrite = mustOverwriteColors();
	QModelIndexList list= selector->selectedList();
	for (int j=0; j<list.size(); j++){
		BrfMesh &m(brfdata.mesh[list[j].row()]);
		if (overwrite) m.ColorAll(col);
		else m.MultColorAll(col);
		setModified();
	}
	guiPanel->ui->rbVertexcolor->click();
	updateGui();
	updateGl();
}


void MainWindow::meshColorWithTexture(){

	glWidget->renderTextureColorOnMeshes( mustOverwriteColors() );
	setModified();
	if (guiPanel->ui->cbTexture->isChecked()) guiPanel->ui->cbTexture->click();
	if (!guiPanel->ui->rbVertexcolor->isChecked()) guiPanel->ui->rbVertexcolor->click();

	updateGui();
	updateGl();
}

bool MainWindow::mustOverwriteColors(){
	return !(QApplication::keyboardModifiers() & Qt::ShiftModifier);
}

void MainWindow::meshComputeAo(){
	bool inAlpha = optionAoInAlpha->isChecked() && useAlphaCommands;
	glWidget->renderAoOnMeshes(
	      0.33f * currAoBrightnessLevel()/4.0f,
	      0.4+0.6*(1-currAoFromAboveLevel()),
	      currAoPerFace(),
	      inAlpha,
	      mustOverwriteColors()
	);
	setModified();
	statusBar()->showMessage(tr("Computed AO%1").arg(inAlpha?tr("(in alpha channel)"):""), 2000);

	updateGui();
	updateGl();
	//guiPanel->ui->rbVertexcolor->setEnabled(true);
	guiPanel->ui->rbVertexcolor->click();

}

unsigned int tuneColor(unsigned int col, int contr, int dh, int ds, int db){
	QColor c(col&0xFF,(col>>8)&0xFF,(col>>16)&0xFF,(col>>24)&0xFF);
	c.convertTo(QColor::Hsv);
	qreal h,s,b,a;
	c.getHsvF(&h,&s,&b,&a);
	h = c.hueF();
	if (h<0) h=0;
	h+=dh/200.0;

	h = h+100 - floor(h+100);
	//h = dh;
	b+= 0.5 * contr/50.0 * (0.5*(sin(3.1415*(b-0.5))+1.0)-b) + db/100.0;
	s+=ds/100.0;
	if (h<0) h=0;
	if (s<0) s=0;
	if (b<0) b=0;
	if (h>1) h=1;
	if (s>1) s=1;
	if (b>1) b=1;
	c.setHsvF(h,s,b,a);

	c.convertTo(QColor::Rgb);
	unsigned int alpha = c.alpha();
	return (c.red()&0xff) | ((c.green()&0xff)<<8) | ((c.blue()&0xff)<<16) | (alpha<<24);
}


void MainWindow::meshTuneColorCancel(bool reallyCancel){
	static std::vector<unsigned int> stored;
	QModelIndexList list= selector->selectedList();
	if (reallyCancel) {
		// store original colors
		stored.clear();
		for (int j=0,h=0; j<list.size(); j++){
			BrfMesh &m(brfdata.mesh[list[j].row()]);
			for (uint i=0; i<m.vert.size(); i++,h++) {
				stored.push_back(m.vert[i].col);
			}
		}
	} else {
		// recover original colors
		for (int j=0,h=0; j<list.size(); j++){
			BrfMesh &m(brfdata.mesh[list[j].row()]);
			for (uint i=0; i<m.vert.size(); i++,h++) {
				m.vert[i].col = stored[h];
			}
		}
	}

}

void MainWindow::meshTuneColorDo(int c,int h,int s,int b){
	meshTuneColorCancel(false);
	QModelIndexList list= selector->selectedList();
	for (int j=0; j<list.size(); j++){
		BrfMesh &m(brfdata.mesh[list[j].row()]);
		m.TuneColors(c,h,s,b);
	}
	updateGl();
}

void MainWindow::meshTuneColor(){
	meshTuneColorCancel(true);
	AskHueSatBriDialog *d = new AskHueSatBriDialog(this);
	connect(d, SIGNAL(anySliderMoved(int,int,int,int)), this, SLOT(meshTuneColorDo(int,int,int,int)));
	int res = d->exec();
	if (res!=QDialog::Accepted) meshTuneColorCancel(false); else {
		setModified();
		guiPanel->ui->rbVertexcolor->click();
	}
	updateGui();
	updateGl();
	delete d;
}

void MainWindow::meshDiscardCol(){
	QModelIndexList list= selector->selectedList();
	for (int j=0; j<list.size(); j++){
		BrfMesh &m(brfdata.mesh[list[j].row()]);
		m.ColorAll(0xFFFFFFFF);
		setModified();
	}
	updateGui();
	updateGl();
}
void MainWindow::meshDiscardTan(){
	QModelIndexList list= selector->selectedList();
	for (int j=0; j<list.size(); j++){
		BrfMesh &m(brfdata.mesh[list[j].row()]);
		m.DiscardTangentField();
		setModified();
	}
	updateGui();
	updateGl();
}


void MainWindow::meshDiscardNor(){
	QModelIndexList list= selector->selectedList();
	for (int j=0; j<list.size(); j++){
		BrfMesh &m(brfdata.mesh[list[j].row()]);
		m.UnifyPos();
		m.UnifyVert(false);
		setModified();
	}
	updateGui();
	updateGl();
}

void MainWindow::meshDiscardAni(){
	QModelIndexList list= selector->selectedList();
	for (int j=0; j<list.size(); j++){
		BrfMesh &m(brfdata.mesh[list[j].row()]);
		m.KeepOnlyFrame(guiPanel->getCurrentSubpieceIndex(MESH));
		setModified();
	}
	updateGui();
	updateGl();
}

double myRound(double d){
	if (d<=0) return 0.1;
	if (d>=1e10) return 1e10;
	double base = 1;
	while (d>1.0) { d/=10; base*=10; }
	while (d<0.1) { d*=10; base/=10; }
	// now d in 0.1 .. 1.0
	if (d<0.15) d = 0.1; else
		if (d<0.23) d = 0.2; else
			if (d<0.42) d = 0.25; else
				if (d<0.75) d = 0.5; else
					d=1.0;
	return d*base;
}


void MainWindow::transform(){
	QModelIndexList list= selector->selectedList();
	if (list.size()>0) {
		glWidget->lastSelected = list[ list.size()-1 ].row();

		// find bbox
		Box3f bboxAll, bboxOne; bboxAll.SetNull();
		if (selector->currentTabName()==MESH)
			for (int j=0; j<list.size(); j++){
				bboxOne = brfdata.mesh[list[j].row()].bbox;
				bboxAll.Add( bboxOne );
			} else
			if (selector->currentTabName()==BODY)
				for (int j=0; j<list.size(); j++){
					bboxOne = brfdata.body[list[j].row()].bbox;
					bboxAll.Add( bboxOne );
				}

		AskTransformDialog *d = askTransformDialog;
		d->setMultiObj(list.size()>1);
		d->setSensitivityOne( myRound( bboxOne.Diag() / 40 ) ) ; // );
		d->setSensitivityAll( myRound( bboxAll.Diag() / 40 ) );


		/*
		// reset to Indentity when used twice over same selection...
		static char lastUsedPath[1024]="";
		static int lastUsedI = -1;
		static int lastUsedTab = -1;
		if (
			(strcmp(lastUsedPath,curFile.toAscii().data())==0) &&
			(lastUsedI == list[0].row()) &&
			(lastUsedTab = selector->currentTabName())
		) {
			// applying the same transofrm to the same object:
			d->reset();
		}
		strcpy(lastUsedPath,curFile.toAscii().data());
		lastUsedI = list[0].row();
		lastUsedTab = selector->currentTabName();
		*/
		if (!executingRepeatedCommand) d->reset();
		executingRepeatedCommand = false;

		d->setBoundingBox(bboxAll.min.V(), bboxAll.max.V());
		bool ok = d->exec() == QDialog::Accepted;

		int start = (glWidget->applyExtraMatrixToAll)?0:list.size()-1;

		if (ok) {

			if (selector->currentTabName()==MESH)
				for (int j=start; j<list.size(); j++)
					brfdata.mesh[list[j].row()].Transform(d->matrix);

			if (selector->currentTabName()==BODY)
				for (int j=start; j<list.size(); j++)
					brfdata.body[list[j].row()].Transform(d->matrix);

			setModified();
		}

	}
	glWidget->clearExtraMatrix();
	updateGl();
}


void MainWindow::transferRigging(){
	int i = selector->firstSelected();
	QModelIndexList list= selector->selectedList();
	if (list.size()<2 || !brfdata.mesh[i].IsRigged()) {
		QMessageBox::information(this,
		                         tr("Transfer Rigging"),
		                         tr("Transfer rigging:\nselect a rigged mesh first,\nthen all target meshes.\n")
		                         );

	} else {
		vector<BrfMesh> mv;
		mv.push_back(brfdata.mesh[i]);
		for (int j=1; j<list.size(); j++){
			brfdata.mesh[list[j].row()].TransferRigging(mv,0,0);
		}
	}

	selector->updateData(brfdata);
	setModified();
}

void MainWindow::reskeletonize(){
	int k=0;

	BrfMesh m2 = brfdata.mesh[selector->firstSelected()];
	int method=0, output=0;
	QPair<int,int> res = askRefSkel( m2.maxBone,  method, output);

	int a=res.first; int b=res.second;
	if (a==-1) return;
	if (a==b) {
		QMessageBox::information(this,
		                         "OpenBRF",
		                         tr("Same skeleton:\nreskeletonization canceled.\n")
		                         );
		return;
	}
	bool withAll = (b>=(int)reference.skeleton.size());

	if (!withAll)
	if (reference.skeleton[a].bone.size()!=reference.skeleton[b].bone.size()) {
		QMessageBox::information(this,
		                         "OpenBRF",
		                         tr("Different number of bones:\nreskeletonization canceled.\n")
		                         );
		return;
	}

	std::vector<BrfMesh> toInsert;

	for (int ii=0; ii<selector->selectedList().size(); ii++) {
		int i = selector->selectedList()[ii].row()+k;
		if (i<0) continue;
		if (i>(int)brfdata.mesh.size()) continue;

		for (uint bb=0; bb<reference.skeleton.size(); bb++) {
			if (withAll) {
				b = bb;
				if (a==b) continue;
				if (reference.skeleton[a].bone.size()!=reference.skeleton[b].bone.size())
					continue;
			}
			BrfMesh m = brfdata.mesh[i];
			m.KeepOnlyFrame( currentDisplayFrame() );

			if (method==1)
				m.ReskeletonizeHuman( reference.skeleton[a], reference.skeleton[b]);
			else
				m.Reskeletonize( reference.skeleton[a], reference.skeleton[b]);

			if (output==1) {
				char newName[1024];
				sprintf(newName,"%s_%s",m.name,reference.skeleton[b].name);
				m.SetName(newName);
				toInsert.push_back(m);
				k++;
			}
			else if (output==2 || withAll) {
				brfdata.mesh[i] = m;
			}
			else { // output == 1
				brfdata.mesh[i].EnsureTwoFrames();
				//if (brfdata.mesh[i].frames.size()<=0) KeepOnlyFrame(0);
				brfdata.mesh[i].AddFrameDirect(m);
				//brfdata.mesh[i].AddFrameDirect(brfdata.mesh[i]);
			}
			if (!withAll) break;
		}

		setModified();
	}

	for (uint i=0; i<toInsert.size(); i++) insert(toInsert[i]);

	selector->updateData(brfdata);

}


void MainWindow::moveUpSel(){
	int i = selector->firstSelected();
	int j = i-1; if (j<0) return;
	switch (selector->currentTabName()) {
	case MESH: _swap(brfdata.mesh, i, j); break;
	case TEXTURE: _swap(brfdata.texture, i, j); break;
	case SHADER: _swap(brfdata.shader, i,j); break;
	case MATERIAL:  _swap(brfdata.material, i,j); break;
	case SKELETON: _swap(brfdata.skeleton, i,j); break;
	case ANIMATION: _swap(brfdata.animation, i,j); break;
	case BODY: _swap(brfdata.body, i,j); break;
	default: assert(0);
	}
	selector->updateData(brfdata);
	selector->moveSel(-1);
	inidataChanged();
	setModified(false);
}
void MainWindow::moveDownSel(){
	int i = selector->firstSelected();
	int j = i+1;
	bool res;
	switch (selector->currentTabName()) {
	case MESH: res=_swap(brfdata.mesh, i, j); break;
	case TEXTURE: res=_swap(brfdata.texture, i, j); break;
	case SHADER: res=_swap(brfdata.shader, i,j); break;
	case MATERIAL:  res=_swap(brfdata.material, i,j); break;
	case SKELETON: res=_swap(brfdata.skeleton, i,j); break;
	case ANIMATION: res=_swap(brfdata.animation, i,j); break;
	case BODY: res=_swap(brfdata.body, i,j); break;
	default: assert(0);
	}
	if (res) {
		selector->updateData(brfdata);
		selector->moveSel(+1);
		inidataChanged();
		setModified(false);
	}
}
static void _findCommonPrefix(QString& a, QString b){
	for (int i=a.size(); i>=0; i--) {
		QString a0=a;
		QString b0=b;
		a0.truncate(i);
		b0.truncate(i);
		a = a0;
		if (a0==b0) break;
	}
}

void MainWindow::renameSel(){
	QString commonPrefix;
	int n = 0, max = selector->selectedList().size();
	TokenEnum t=(TokenEnum)selector->currentTabName();

	if (!max) return;
	for (int j=0; j<max; j++) {
		int i = selector->selectedList()[j].row();
		char* name=NULL;
		switch (t) {
		case MESH: name=_getName(brfdata.mesh,i); break;
		case TEXTURE: name=_getName(brfdata.texture,i); break;
		case SHADER: name=_getName(brfdata.shader,i); break;
		case MATERIAL:  name=_getName(brfdata.material, i ); break;
		case SKELETON: name=_getName(brfdata.skeleton, i); break;
		case ANIMATION: name=_getName(brfdata.animation,i); break;
		case BODY: name=_getName(brfdata.body, i); break;
		default: assert(0);
		}
		if (name) {
			if (!n)  commonPrefix= QString("%1").arg(name);
			else  _findCommonPrefix( commonPrefix , name);
			n++;
		}
	}

	if (n>0) {
		bool ok;
		QString newPrefix;
		if (n==1) {
			newPrefix = QInputDialog::getText(
			      this,
			      tr("OpenBrf"),
			      tr("Renaming %1...\nnew name:").arg(IniData::tokenFullName(t)),
			      QLineEdit::Normal,
			      QString(commonPrefix), &ok
			      );
			if (newPrefix==commonPrefix) ok = false;
			if ((t==TEXTURE) && (!newPrefix.contains('.'))) newPrefix+=".dds";
		}
		else {
			int ps = commonPrefix.size();
			newPrefix = QInputDialog::getText(
			      this,
			      tr("OpenBrf"),
			      tr("%3 common prefix for %1 %2...\nnew prefix:").arg(n).arg(IniData::tokenPlurName(t)).arg((ps)?tr("Changing the"):tr("Adding a")),
			      QLineEdit::Normal,
			      commonPrefix, &ok
			      );
			if (newPrefix==commonPrefix) ok = false;
		}
		if (ok) {
			for (int j=0; j<max; j++) {
				int i = selector->selectedList()[j].row();
				char* name=NULL;

				switch (t) {
				case MESH: name=_getName(brfdata.mesh,i); break;
				case TEXTURE: name=_getName(brfdata.texture,i); break;
				case SHADER: name=_getName(brfdata.shader,i); break;
				case MATERIAL:  name=_getName(brfdata.material, i ); break;
				case SKELETON: name=_getName(brfdata.skeleton, i); break;
				case ANIMATION: name=_getName(brfdata.animation,i); break;
				case BODY: name=_getName(brfdata.body, i); break;
				default: assert(0);
				}
				QString newName = QString("%1").arg(name);
				int ps = commonPrefix.size();
				newName =newPrefix + newName.remove( 0,ps );
				_setNameOnCharStar(name,newName);
				if (t==MESH) brfdata.mesh[i].AnalyzeName();
			}
			setModified();
			inidataChanged();
			updateSel();

		}
	}
}
void MainWindow::deleteSel(){
	int i = selector->lastSelected();
	if (i==-1) return;
	unsigned int res=0;
	switch (selector->currentTabName()) {
	case MESH: res=_del(brfdata.mesh, selector->selectedList()); break;
	case TEXTURE: res=_del(brfdata.texture,  selector->selectedList()); break;
	case SHADER: res=_del(brfdata.shader,  selector->selectedList()); break;
	case MATERIAL:  res=_del(brfdata.material,  selector->selectedList()); break;
	case SKELETON: res=_del(brfdata.skeleton,  selector->selectedList()); break;
	case ANIMATION: res=_del(brfdata.animation,  selector->selectedList()); break;
	case BODY: res=_del(brfdata.body,  selector->selectedList()); break;
	default: return; // no selection, no tab
	}
	if (res>0) {
		if (i<0 || i>=(int)res) i=res-1;
		selector->selectOneSilent(selector->currentTabName(),i);
	}
	inidataChanged();
	updateSel();
	setModified(false);
}



void MainWindow::editCutFrame(){
	if (selector->currentTabName()!=MESH) return;
	int i = selector->firstSelected();
	if (i<0 || i>=(int)brfdata.mesh.size()) return;

	BrfMesh &m(brfdata.mesh[i]);
	if (m.frame.size()<=1) editCut(); // last frame... cut entire mesh!
	else {
		editCopyFrame();
		int j = guiPanel->getCurrentSubpieceIndex(MESH);
		m.frame.erase( m.frame.begin()+j,m.frame.begin()+j+1);
		m.AdjustNormDuplicates();
		selector->selectOne(MESH,i);
		setModified();
		//guiPanel->ui->frameNumber->ximum(m.frame.size()-1);
	}
}

QString MainWindow::senderText()const{
	QObject *o = sender();
	if (!o) return QString();
	return o->objectName();
}

void MainWindow::editCopyHitbox(){
	BrfSkeleton &s(getUniqueSelected<BrfSkeleton>());
	if (!&s) {
		QMessageBox::information(this,"OpenBrf",tr("%1: Select one skeleton with a hitbox first").arg(senderText()));
		return;
	}

	BrfBody *b= hitboxSet.FindBody(s.name);
	if (!b){
		QMessageBox::information(this,"OpenBrf",tr("%1: skeleton %2 has no kwown hitbox to copy").arg(senderText()).arg(s.name));
		return;
	}

	clipboard.Clear();
	clipboard.skeleton.push_back(s);
	clipboard.body.push_back(*b);
	_setName(clipboard.body[0],s.name);
	saveSystemClipboard();

}

void MainWindow::editPasteHitbox(){
	BrfSkeleton &s2(getUniqueSelected<BrfSkeleton>());
	if (!&s2) {
		QMessageBox::information(this,"OpenBrf",tr("%1: Select one skeleton with a hitbox first").arg(senderText()));
		return;
	}

	BrfBody res;

	if (clipboard.IsOneSkelOneHitbox()){
		// copy hitbox verbatim!
		res = clipboard.body[0];
		BrfSkeleton &s1 ( clipboard.skeleton[0] );

		if (s1.bone.size() != s2.bone.size() ){
			QMessageBox::warning(this,"OpenBRF",
			                     tr("Wrong number of bones! (%1 in %2 VS %3 in %4). Cannot perform action")
			                     .arg(s1.bone.size()).arg(s1.name).arg(s2.bone.size()).arg(s2.name)
			                     );
			return;
		}

	} else if (clipboard.totSize()==1 && clipboard.body.size()==1){
		const BrfBody &b (clipboard.body[0]);

		if (!s2.LayoutHitboxes(b,res,true)) {
			QMessageBox::warning(this,"OpenBRF",
			                     tr("Wrong number of bones! (%1 in %2 VS %3 in %4). Cannot perform action")
			                     .arg(b.part.size()).arg(b.name).arg(s2.bone.size()).arg(s2.name)
			                     );
			return;
		}


	} else {
		QMessageBox::information(this,"OpenBrf",
		                         tr("Cannot paste hitboxes: I don't have a hitboxes plus skeleton in clipboard")
		                         );
		return;
	}

	_setName(res,s2.name); // copy name of new skeleton
	int bi = hitboxSet.Find(res.name, BODY);
	if (bi>=0) hitboxSet.body[bi] = res; // substitute hitboxes in set
	else hitboxSet.body.push_back(res); // add hitbox to set
	setModifiedHitboxes(true);
	updateGui();
	updateGl();



}

void MainWindow::editCopyFrame(){
	if (selector->currentTabName()!=MESH) return;
	int i = selector->firstSelected();
	if (i<0 || i>=(int)brfdata.mesh.size()) return;
	BrfMesh &m(brfdata.mesh[i]);
	int j = guiPanel->getCurrentSubpieceIndex(MESH);
	if (j<0 || j>=(int)m.frame.size()) return;

	clipboard.Clear();
	clipboard.mesh.push_back(m);
	clipboard.mesh[0].KeepOnlyFrame(j);

	saveSystemClipboard();
}

void MainWindow::onSelectedPoint(float x, float y, float z){
	QString st = QString("(%1 , %2 , %3)").arg(x).arg(y).arg(z);

	QMimeData *mime = new QMimeData();
	mime->setText( st );

	statusBar()->showMessage(QString("Point %1 copyed to clipboard").arg(st));
	QApplication::clipboard()->clear();
	QApplication::clipboard()->setMimeData(mime);
}


void MainWindow::onClipboardChange(){
	const QMimeData *mime = QApplication::clipboard()->mimeData();
	if (!mime) return;
	bool isMyData = mime->hasFormat("application/openBrf");

	//QMessageBox::information(this,"Clipboard stuff",(isMyData)?"with my data!":"not my data");

	if (isMyData) {
		loadSystemClipboard();
	} else {
		clipboard.Clear();
	}
	if (isMyData)
		statusBar()->showMessage((isMyData)?
		                           QString(tr("%1 new BRF items found in clipboard...")).arg(clipboard.totSize())
		                         :tr("Unusable data in clipboard"));

	editPasteAct->setEnabled( clipboard.totSize() );

	editPasteHitboxAct->setEnabled( clipboard.IsOneSkelOneHitbox() ||
	                                (clipboard.totSize()==1 && clipboard.body.size()==1) );

	bool allRigged=true;
	for (unsigned int i=0; i<clipboard.mesh.size(); i++)
		if (!clipboard.mesh[i].IsRigged()) allRigged = false;

	editPasteRiggingAct->setEnabled( (allRigged && clipboard.mesh.size()>0) || (clipboard.skeleton.size()==1));


	if (clipboard.mesh.size()!=0){
		// maybe it was just rigged meshes?



		// maybe it was a single frame mesh?
		editPasteFrameAct->setEnabled((clipboard.mesh.size()==1) && (clipboard.mesh[0].frame.size()==1));

		editPasteTextcoordsAct->setEnabled(clipboard.mesh.size()==1);
		editPasteVertColorsAct->setEnabled(clipboard.mesh.size()==1);
		editPasteVertAniAct->setEnabled((clipboard.mesh.size()==1) &&  (clipboard.mesh[0].frame.size()>=1));
		editPasteModificationAct->setEnabled(true);
		editPasteMergeMeshAct->setEnabled(true);

	} else {
		//editPasteRiggingAct->setEnabled(false);
		editPasteFrameAct->setEnabled(false);
		editPasteModificationAct->setEnabled(false);
		editPasteFrameAct->setEnabled(false);
		editPasteTextcoordsAct->setEnabled(false);
		editPasteVertColorsAct->setEnabled(false);
		editPasteVertAniAct->setEnabled(false);
	}

	editPasteAniLowerPartsAct->setEnabled((clipboard.animation.size()==1));

	editPasteTimingsAct->setEnabled(
	      (
	        ((clipboard.mesh.size()==1)&&(clipboard.mesh[0].frame.size()>0))
	        ||(clipboard.animation.size()==1)
	        )
	      );

}

bool MainWindow::createScenePropText(){
	int n1, n2;
	const char* txt = brfdata.GetAllObjectNamesAsSceneProps(&n1,&n2);
	if (txt) {
		QString filename = curFile;
		int k = curFile.lastIndexOf('/');
		filename.remove(0,k+1);
		QString res = QString("# from '%1': begin (OpenBRF)\n%2# from '%1': end (OpenBRF)\n").arg(filename).arg(txt);
		QApplication::clipboard()->setText(res);
		QMessageBox::information(this,"OpenBrf",tr("Copyed prop code for %1 objects\n(%2 with matching collison mesh)\non the clipboard.\n\nPaste at will!").arg(n1).arg(n2) );
		return true;
	}
	statusBar()->showMessage(tr("No prop mesh found"));
	return false;
}

void MainWindow::saveSystemClipboard(){

	QMimeData *mime = new QMimeData();

	// save string as object name
	const char* text = clipboard.GetAllObjectNames(); //GetFirstObjectName();
	if (text) mime->setText( text );

	// save data
	QTemporaryFile file;
	file.open();
	FILE* pFile = fdopen(file.handle(), "wb");
	clipboard.version = usingWarband;
	clipboard.Save(pFile);
	fflush(pFile);

	QFile refile(file.fileName());

	refile.open(QIODevice::ReadOnly);

	/*
	//refile.reset();
	if (!refile.isOpen()) {
		QMessageBox::information(this,"Cannot open file",QString("%1").arg(refile.fileName()));
	} else {
		QMessageBox::information(this,"...",
			QString("file size: %1 (%2)").arg(refile.size()).arg(refile.readAll().size())
		);
		refile.reset();
	}*/

	mime->setData("application/openBrf",refile.readAll());

	refile.close();
	file.close();


	QApplication::clipboard()->clear();
	QApplication::clipboard()->setMimeData(mime);


}

void MainWindow::loadSystemClipboard(){
	//return;
	clipboard.Clear();
	const QMimeData *mime = QApplication::clipboard()->mimeData();
	const QByteArray &ba = mime->data("application/openBrf");
	QTemporaryFile f;
	wchar_t fn[1000];
	f.open();
	f.fileName().toWCharArray(fn);
	fn[f.fileName().size()]=0;
	f.write(ba);
	f.flush();
	/*
	if (!f.isOpen()) {
		QMessageBox::information(this,"Cannot open file",QString("%1").arg(f.fileName()));
	} else {
		QMessageBox::information(this,"load",
			QString("filename: %3\n%4,file size: %1 (%2)").
			arg(f.size()).arg(ba.size()).arg(f.fileName()).arg(QString("pippo"))
		);
	}*/

	//FILE* pFile = fdopen(f.handle(), "rb");
	if (!clipboard.Load(fn)) {
		QMessageBox::information(this,"Cannot load file",QString("%1").arg(f.fileName()));
	}
	f.close();
	//
}

void MainWindow::editPasteAniLowerParts(){
	QModelIndexList list= selector->selectedList();

	if (clipboard.animation.size()!=1) return;

	for (int j=0; j<list.size(); j++){
		brfdata.animation[list[j].row()].CopyLowerParts(clipboard.animation[0]);
	}

	setModified();
	updateGui();
	updateGl();


}

void MainWindow::editPasteTextcoords(){
	QModelIndexList list= selector->selectedList();
	for (int j=0; j<list.size(); j++){
		brfdata.mesh[list[j].row()].CopyTextcoords(clipboard.mesh[0]);
	}
	updateSel();
	setModified();
}


void MainWindow::editPasteVertColors(){
	QModelIndexList list= selector->selectedList();
	for (int j=0; j<list.size(); j++){
		brfdata.mesh[list[j].row()].CopyVertColors(clipboard.mesh[0],mustOverwriteColors());
	}
	updateSel();
	setModified();
}

void MainWindow::editPasteVertAni(){
	QModelIndexList list= selector->selectedList();
	for (int j=0; j<list.size(); j++){
		brfdata.mesh[list[j].row()].CopyVertAni(clipboard.mesh[0]);
	}
	updateSel();
	setModified();
}

void MainWindow::editPasteRigging(){

	QModelIndexList list= selector->selectedList();
	bool allRigged=true;
	for (unsigned int i=0; i<clipboard.mesh.size(); i++)
		if (!clipboard.mesh[i].IsRigged()) allRigged = false;

	bool canPasteFromSkel = clipboard.skeleton.size() == 1;
	bool canPasteFromMesh = (clipboard.mesh.size()>0) && allRigged;

	if (!list.size() || (selector->currentTabName()!=MESH) || (!canPasteFromSkel && !canPasteFromMesh)) {
		QMessageBox::information(this,
		                         tr("Copy Rigging into another mesh"),
		                         tr("Copy Rigging into another mesh:\n"
		                            "- select one or more sample rigged mesh\n"
		                            "- copy them (ctrl+C)\n"
		                            "- then select one or more target meshes (rigged or otherwise),\n"
		                            "- then paste rigging.\n"
		                            "\n"
		                            "(works best if sample mesh is similar to target meshes)\n"
		                            )
		                         );

	} else {
		if (canPasteFromMesh) {
			for (int j=0; j<list.size(); j++){
				brfdata.mesh[list[j].row()].TransferRigging(clipboard.mesh,0,0);
			}
			statusBar()->showMessage(tr("Transferred rigging into %1 mesh(es) from %2 exemplar mesh(es).").arg(list.size()).arg(clipboard.mesh.size()));
		} else {
			std::vector<BrfMesh> tmp(1);
			//BrfMesh m;
			//tmp.push_back(m);
			clipboard.skeleton[0].BuildDefaultMesh(tmp[0]);
			for (int j=0; j<list.size(); j++){
				brfdata.mesh[list[j].row()].TransferRigging(tmp,0,0);
			}
			statusBar()->showMessage(tr("Transferred rigging into %1 mesh(es) from skeleton '%2'.").arg(list.size()).arg(clipboard.skeleton[0].name));
		}
	}

	selector->updateData(brfdata);
	setModified();
}

void MainWindow::editPasteMergeMesh(){

	QModelIndexList list= selector->selectedList();

	bool stDone = false;
	std::vector<BrfMesh> &vec(clipboard.mesh);

	// match lods or not? do, if clipboard has no LOD and LOD
	bool hasLodN = false;
	bool hasLod0 = false;
	for (int j=0; j<(int)vec.size(); j++) {
		if (vec[j].IsNamedAsLOD()==0) hasLod0 = true;
		else hasLodN = true;
	}
	bool matchLod = hasLod0 && hasLodN;

	for (int j=0; j<list.size(); j++) {
		BrfMesh &m(getSelected<BrfMesh>(j));
		if (!&m) continue;
		for (int k=0; k<(int)vec.size(); k++){
			BrfMesh d = vec[k];
			//qDebug("%s %s (%d %d)",d.name,m.name, d.IsNamedAsLOD(),m.IsNamedAsLOD());
			if (matchLod && (d.IsNamedAsLOD()!=m.IsNamedAsLOD())) continue;
			//qDebug("merging");
			//d.TransformUv(1,1,0.5,0.5);
			d.UniformizeWith(m);
			if (!m.Merge(d)) QMessageBox::critical(this,"OpenBRF","Strange error in editPasteMergeMesh!");
			stDone = true;
		}
	}

	if (stDone) {
		updateGl();
		updateGui();
		setModified();
	}
}

void MainWindow::editPasteFrame(){
	if (clipboard.mesh.size()==1 ) { //&& clipboard.mesh[0].frame.size()==1) {
		int i = selector->firstSelected();
		if (selector->currentTabName()!=MESH || i < 0 || i>=(int)brfdata.mesh.size()) {
			editPaste(); // no mesh selected: paste new mesh...
			return;
		}
		int j=guiPanel->getCurrentSubpieceIndex(MESH);
		if (j<0) j=0;
		BrfMesh &m(brfdata.mesh[i]);
		bool res=false;
		if (assembleAniMode()==0) {
			res = m.AddFrameMatchVert(clipboard.mesh[0],j);
			if (!res) statusBar()->showMessage(tr("Vertex number mismatch... using texture-coord matching instead of vertex-ordering"),7000);
		}
		if (assembleAniMode()==2) {
			res = m.AddFrameMatchPosOrDie(clipboard.mesh[0],j);
			if (!res) statusBar()->showMessage(tr("Vertex number mismatch... using texture-coord matching instead"),7000);
		}
		if (!res) m.AddFrameMatchTc(clipboard.mesh[0],j);
		statusBar()->showMessage(tr("Added frame %1").arg(j+1),2000);
		setModified();
		selector->selectOne(MESH,i);
	}

}

void MainWindow::editCut(){
	clipboard.Clear();
	addSelectedToClipBoard();
	deleteSel();
	saveSystemClipboard();
}

void MainWindow::editCopy(){
	clipboard.Clear();
	addSelectedToClipBoard();
	saveSystemClipboard();
}

void MainWindow::editAddToCopy(){
	addSelectedToClipBoard();
	saveSystemClipboard();
}

void MainWindow::editCopyComplete(){
	clipboard.Clear();
	addSelectedToClipBoard();
	completeClipboard(false);
	saveSystemClipboard();
}

void MainWindow::editCutComplete(){
	clipboard.Clear();
	addSelectedToClipBoard();
	completeClipboard(true);
	deleteSel();
	saveSystemClipboard();
}


void MainWindow::sortEntries(){
	switch (selector->currentTabName()) {
	case MESH:     _sort(brfdata.mesh);  break;
	case TEXTURE:  _sort(brfdata.texture); break;
	case SHADER:   _sort(brfdata.shader); break;
	case MATERIAL: _sort(brfdata.material); break;
	case SKELETON: _sort(brfdata.skeleton); break;
	case ANIMATION:_sort(brfdata.animation); break;
	case BODY:     _sort(brfdata.body); break;
	default: return ; //assert(0);
	}
	setModified();
	inidataChanged();
	updateSel();
	updateGui();
}



void MainWindow::completeClipboard(bool andDelete){
	std::vector<bool> takeM(brfdata.material.size(),false);
	std::vector<bool> takeT(brfdata.texture.size(),false);
	std::vector<bool> takeS(brfdata.shader.size(),false);

	for(uint i=0; i<clipboard.mesh.size(); i++){
		int k = brfdata.Find(clipboard.mesh[i].material,MATERIAL);
		if (k>=0) takeM[k]=true;
	}

	for (int i=takeM.size()-1; i>=0; i--) if (takeM[i]) {
		qDebug("i = %d",i);
		clipboard.material.push_back( brfdata.material[i] );
		if (andDelete) brfdata.material.erase(brfdata.material.begin()+i);
	}

	for(uint i=0; i<clipboard.material.size(); i++){
		int k;
		k = brfdata.FindTextureWithExt(clipboard.material[i].diffuseA);
		if (k>-1) takeT[k] = true;
		k = brfdata.FindTextureWithExt(clipboard.material[i].diffuseB);
		if (k>-1) takeT[k] = true;
		k = brfdata.FindTextureWithExt(clipboard.material[i].bump);
		if (k>-1) takeT[k] = true;
		k = brfdata.FindTextureWithExt(clipboard.material[i].enviro);
		if (k>-1) takeT[k] = true;
		k = brfdata.FindTextureWithExt(clipboard.material[i].spec);
		if (k>-1) takeT[k] = true;
		k = brfdata.Find(clipboard.material[i].shader,SHADER);
		if (k>-1) takeS[k] = true;
	}

	for (int i=takeT.size()-1; i>=0; i--) if (takeT[i]) {
		clipboard.texture.push_back( brfdata.texture[i] );
		if (andDelete) brfdata.texture.erase(brfdata.texture.begin()+i);
	}

	for (int i=takeS.size()-1; i>=0; i--) if (takeS[i]) {
		clipboard.shader.push_back( brfdata.shader[i] );
		if (andDelete) brfdata.shader.erase(brfdata.shader.begin()+i);
	}

}

void MainWindow::addSelectedToClipBoard(){
	switch (selector->currentTabName()) {
	case MESH:     _copy(brfdata.mesh,     selector->selectedList(), clipboard.mesh);  break;
	case TEXTURE:  _copy(brfdata.texture,  selector->selectedList(), clipboard.texture); break;
	case SHADER:   _copy(brfdata.shader,   selector->selectedList(), clipboard.shader); break;
	case MATERIAL: _copy(brfdata.material, selector->selectedList(), clipboard.material); break;
	case SKELETON: _copy(brfdata.skeleton, selector->selectedList(), clipboard.skeleton); break;
	case ANIMATION:_copy(brfdata.animation,selector->selectedList(), clipboard.animation); break;
	case BODY:     _copy(brfdata.body,     selector->selectedList(), clipboard.body); break;
	default: return ; //assert(0);
	}
}

void MainWindow::editPasteTimings(){
	std::vector<int> timings;
	if (clipboard.mesh.size()==1) {
		clipboard.mesh[0].GetTimings(timings);
	} else if (clipboard.animation.size()==1) {
		clipboard.animation[0].GetTimings(timings);
	} else {
		statusBar()->showMessage(tr("Cannot paste timings! Select *one* animated mesh or skel animation"),8000);
		return;
	}

	int max = selector->selectedList().size();
	TokenEnum t=(TokenEnum)selector->currentTabName();


	if (t==MESH){
		for (int j=0; j<max; j++) {
			int i = selector->selectedList()[j].row();
			brfdata.mesh[i].SetTimings(timings);
		}
		statusBar()->showMessage(tr("Pasted timings over %1 (animated) mesh").arg(max),8000);
	} else if (t==ANIMATION){
		for (int j=0; j<max; j++) {
			int i = selector->selectedList()[j].row();
			brfdata.animation[i].SetTimings(timings);
		}
		statusBar()->showMessage(tr("Pasted timings over %1 skeletal animations").arg(max),8000);
	} else {
		max =0;
		statusBar()->showMessage(tr("Cannot paste times over that"),8000);
	}

	if (max>0) {  updateGui();  setModified();  }
}

void MainWindow::editPasteMod(){
	int max = selector->selectedList().size();
	TokenEnum t=(TokenEnum)selector->currentTabName();


	if (clipboard.mesh.size()!=1 || clipboard.mesh[0].frame.size()!=2
	    || t!=MESH || max <1 ) {
		QMessageBox::information(this,tr("OpenBrf"),tr("To use paste modification mesh: first"
		                                               "copy a 2 frames mesh. Then, select one or more destination meshes, and \"paste modification\""
		                                               "any vertex in any frame of the destination mesh that are in the same pos of frame 0,"
		                                               "will be moved on the position of frame 1."));
	} else {
		for (int j=0; j<max; j++) {
			int i = selector->selectedList()[j].row();
			brfdata.mesh[i].CopyModification(clipboard.mesh[0]);
		}
		updateGl();
		updateGui();
		setModified();
	}
}

void MainWindow::editPaste(){
	//deleteSel();

	if (clipboard.IsOneSkelOneHitbox()) {
		// special paste: produce a hitbox as a body part
		BrfBody res;
		if (clipboard.skeleton[0].LayoutHitboxes(clipboard.body[0],res,false)) {
			insert(res);
			setModified(false);
			return;
		}
	}

	for (int i=0; i<(int)clipboard.body.size(); i++) insert(clipboard.body[i]);
	for (int i=0; i<(int)clipboard.texture.size(); i++) insert(clipboard.texture[i]);
	for (int i=0; i<(int)clipboard.shader.size(); i++) insert(clipboard.shader[i]);
	for (int i=0; i<(int)clipboard.material.size(); i++) insert(clipboard.material[i]);
	for (int i=0; i<(int)clipboard.mesh.size(); i++) insert(clipboard.mesh[i]);
	for (int i=0; i<(int)clipboard.skeleton.size(); i++) insert(clipboard.skeleton[i]);
	for (int i=0; i<(int)clipboard.animation.size(); i++) insert(clipboard.animation[i]);

	setModified(false);
}

void MainWindow::duplicateSel(){
	int i = selector->firstSelected();
	switch (selector->currentTabName()) {
	case MESH: _dup(brfdata.mesh, i); brfdata.mesh[i].AnalyzeName(); break;
	case TEXTURE: _dup(brfdata.texture, i); break;
	case SHADER: _dup(brfdata.shader, i); break;
	case MATERIAL:  _dup(brfdata.material, i); break;
	case SKELETON: _dup(brfdata.skeleton, i); break;
	case ANIMATION: _dup(brfdata.animation, i); break;
	case BODY: _dup(brfdata.body, i); break;
	default: assert(0);
	}

	inidataChanged();
	updateSel();

	selector->moveSel(+1);
	setModified();
}



Pair MainWindow:: askRefSkel(int nbones,  int &method, int &output){
	if (reference.skeleton.size()<2) return Pair(-1,-1);

	static int lastA=0, lastB=0;
	static int from =0, to=1;
	AskSkelDialog d(this,reference.skeleton,from,to, lastA, lastB);
	int res=d.exec();
	if (res==QDialog::Accepted) {
		lastA = method = d.getMethodType();
		lastB = output = d.getOutputType();
		return Pair(from=d.getSkelFrom (),to=d.getSkelTo());
	}
	else
		return Pair(-1,-1);
}

void MainWindow::setSelection(const QModelIndexList &l, int k){
	comboViewmodeSelector->setVisible(l.size()>1);
}

void MainWindow::meshRemoveBack(){
	int k=0;
	for (int j=0; j<selector->selectedList().size(); j++) {
		int i = selector->selectedList()[j].row();
		if (i<0 || i>=(int)brfdata.mesh.size()) continue;
		BrfMesh &m(brfdata.mesh[i]);
		m.RemoveBackfacingFaces();
		k++;
	}
	if (k>0) {
		setModified();
		guiPanel->setSelection(selector->selectedList(),MESH);
		updateGl();
	}
}

void MainWindow::meshAddBack(){
	int k=0;
	for (int j=0; j<selector->selectedList().size(); j++) {
		int i = selector->selectedList()[j].row();
		if (i<0 || i>=(int)brfdata.mesh.size()) continue;
		BrfMesh &m(brfdata.mesh[i]);
		m.AddBackfacingFaces();
		k++;
	}
	if (k>0) {
		setModified();
		guiPanel->setSelection(selector->selectedList(),MESH);
		updateGl();
	}
}

bool MainWindow::maybeWarnIfVertexAniTooBig(const BrfMesh &m, const BrfAnimation &a){
	int posXframe = m.frame[0].pos.size();
	int totPos = posXframe*a.frame.size();
	float totMB = (totPos*24)/(1024.0f*1024.0f);
	if (totMB>5.0) {
		int answ = QMessageBox::warning(this, "OpenBrf",tr("This will produce a vertex ani\nwith %1x%2 xyz positions+normals (%4 MB).\n\nProceed?")
			.arg(posXframe).arg(a.frame.size()).arg(totMB,4),
		   QMessageBox::Yes|QMessageBox::Cancel,QMessageBox::Yes
		);
		return (answ==QMessageBox::Yes) ;
	}
	return true;
}

void MainWindow::addToRef(){
	int i = selector->firstSelected();
	assert(i>=0);
	switch (selector->currentTabName()){
	case ANIMATION:
		reference.animation.push_back(brfdata.animation[i]);
		break;
	case SKELETON:
		reference.skeleton.push_back(brfdata.skeleton[i]);
		break;
	default: assert(0);
	}

	//bool wasModified = isModified;
	saveReference();
	//isModified = wasModified;	updateTitle();

}

void MainWindow::meshToVertexAni(){
	for (int j=0; j<selector->selectedList().size(); j++) {
		BrfMesh &m0(getSelected<BrfMesh>(j));
		if (!&m0) continue;
		BrfMesh m = m0;
		m.KeepOnlyFrame( currentDisplayFrame() );
		BrfSkeleton *s = currentDisplaySkeleton();
		if (!s) continue;
		BrfAnimation *a = currentDisplayAnimation();
		if (!a) continue;

		if (!maybeWarnIfVertexAniTooBig(m,*a)) return;

		if (!m.RiggedToVertexAni(*s,*a)) {
			QMessageBox::warning(this,"OpenBRF",tr("Incompatible animation")); return;
		}
		char newName[2048];
		sprintf( newName, "%s_%s", m.name, a->name );
		m.SetName(newName);

		m.DiscardRigging();
		insert(m);

	}
}

void MainWindow::aniToVertexAni(){

	for (int j=0; j<selector->selectedList().size(); j++) {
		BrfAnimation &a(getSelected<BrfAnimation>(j));
		if (!&a) continue;

		BrfSkeleton &s(*currentDisplaySkeleton());
		if (!&s) continue;

		int skinNumber = currentDisplaySkin();
		if (skinNumber<0) skinNumber = askRefSkin();
		if (skinNumber<0) return;
		BrfMesh m = reference.GetCompleteSkin(skinNumber);

		if (!maybeWarnIfVertexAniTooBig(m,a)) return;

		if (!m.RiggedToVertexAni(s,a)) {
			QMessageBox::warning(this,"OpenBRF",tr("Incompatible skin")); return;
		}
		m.SetName(a.name);
		m.DiscardRigging();
		insert(m);
	}

}

void MainWindow::meshMountOnBone(){

	/* produce ALL carry positions
	BrfMesh &m(getSelected<BrfMesh>(0));
	if (!&m) return;
	BrfMesh m3 = m;
	for (uint i=0; i<carryPositionSet.size(); i++) {
		CarryPosition &cp(carryPositionSet.at(i));
		BrfMesh m2 = m3;
		sprintf(m2.name,"%s_%s",m3.name,cp.name);
		m2.Apply(cp,reference.skeleton[0],1.0,true);
		insert(m2);
		//break;
	}
	setModified();
	updateGl(); updateGui();
	return;*/

	/*
	bool isOri;
	int selectedCarryPos;

	Pair p = askRefBoneInt(false, isOri, selectedCarryPos);

	if (p.first==-1) {
		statusBar()->showMessage(tr("Canceled."), 2000);
		return;
	}*/

	int k=0;
	for (int j=0; j<selector->selectedList().size(); j++) {
		BrfMesh &m(getSelected<BrfMesh>(j));
		if (!&m) continue;
		if (!makeMeshRigged(m,false, j==0)) break;
		k++;
	}
	if (k>0) {
		setModified();
		updateGl();
		updateGui();
		updateSel();
	}

	//statusBar()->showMessage(tr("Mounted %1 mesh%2 on bone %3").arg(k).arg((k>1)?"es":"s").arg(p.second), 8000);

}

void MainWindow::meshUnmount(){
	int i = guiPanel->getCurrentSkeletonIndex();
	if (i<0 || i>=(int)reference.skeleton.size()) {
		QMessageBox::warning(this,"OpenBRF",tr("I need to know from which skeleton to Unmount. Select a skeleton in the panel."));
		return;
	}
	BrfSkeleton &s(reference.skeleton[i]);

	int k=0;
	for (int i=0; i<getNumSelected(); i++) {
		BrfMesh &m( getSelected<BrfMesh>(i));
		if (!&m) continue;
		m.Unmount(s);
		m.DiscardRigging();
		k++;
	}
	if (k) {
		setModified(true);
		updateGl();
	}

}

bool MainWindow::makeMeshRigged(BrfMesh &m, bool sayNotRigged,  bool askUserAgain){

	if (!reference.skeleton.size()) {
		QMessageBox::warning(this, "OpenBRF", tr("Not a single skeleton found in reference data! Cancelling operation."));
		return false;
	}
	static bool isAtOrigin = false;
	static int carryPosIndex = 0;
	static int boneIndex = 0;
	static int skelIndex = 0;

	if (askUserAgain) {
		AskBoneDialog d(this,reference.skeleton, carryPositionSet );
		d.sayNotRigged(sayNotRigged);

		int res=d.exec();
		if (res!=QDialog::Accepted) {
			statusBar()->showMessage(tr("Canceled."), 2000);
			return false;
		}

		isAtOrigin = d.pieceAtOrigin();
		carryPosIndex = d.getCarryPos();
		skelIndex = d.getSkel();
		boneIndex = d.getBone();
	}


	BrfSkeleton &s(reference.skeleton[skelIndex]);
	if (!&s) return false;

	if (carryPosIndex==-1) {
		m.SetUniformRig(boneIndex);
		if (isAtOrigin) {
			m.MountOnBone(s,boneIndex);

			char newname[255];
			sprintf(newname,"%s_on_%s",m.name,s.bone.at(boneIndex).name );
			m.SetName(newname);
		}
	} else {
		CarryPosition &cp(carryPositionSet[carryPosIndex]);
		if (cp.needExtraTrasl) {
			if (!guiPanel->ui->rulerSpin->isVisible()) {
				int answ = QMessageBox::warning(this, "OpenBrf",tr("To apply carry position '%1', I need to know the weapon lenght.\nUse the ruler tool to tell me the lenght of weapon '%2'.\n\nActivate ruler tool?")
				   .arg(cp.name).arg(m.name),
				   QMessageBox::Yes|QMessageBox::Cancel,QMessageBox::Yes
				);
				if (answ==QMessageBox::Yes) {
					guiPanel->setMeasuringTool(0);
				}
				return false;
			}
		}
		float weaponLenght = guiPanel->ui->rulerSpin->value()/100.0;
		m.Apply( cp, s, weaponLenght, isAtOrigin );
		char newname[255];
		sprintf(newname,"%s_carried_on_%s",m.name,cp.name );
		m.SetName(newname);

	}
	return true;

}

void MainWindow::addToRefMesh(int k){
	int i=selector->firstSelected();
	assert (selector->currentTabName()==MESH);
	assert(i<(int)brfdata.mesh.size());
	BrfMesh m = brfdata.mesh[i];
	m.KeepOnlyFrame(guiPanel->getCurrentSubpieceIndex(MESH));

	if (!m.IsRigged()) {
		if (!makeMeshRigged( m,true, true)) return;

	}
	char ch =char('A'+k);
	char newname[500];
	sprintf(newname, "skin%c.%s", ch , brfdata.mesh[i].name);
	m.SetName(newname);
	reference.mesh.push_back(m);
	//bool wasModified = isModified;
	saveReference();
	//if (wasModified) setModified(false); else setNotModified();

	statusBar()->showMessage(tr("Added mesh %1 to set %2.").arg(m.name).arg(ch), 5000);
}

template <class T> const char* add(vector<T>& v, const vector<T>& t, int i){
	v.push_back(t[i]);
	return t[i].name;
}
void MainWindow::breakAni(int which, bool useIni){
	if ((int)brfdata.animation.size()>which) {

		BrfAnimation ani = brfdata.animation[which];

		if (!useIni) {
			int res = ani.Break(brfdata.animation);
			if (res>0) {
				updateSel();
				inidataChanged();

				setModified();
				//selector->setCurrentIndex(100);

				statusBar()->showMessage(tr("Animation %2 split in %1 chunks!").arg(res).arg(ani.name), 2000);
			} else {
				statusBar()->showMessage(tr("Animation could be auto-split (frames are too conescutive)"), 10000);
			}
		} else {
			//QString path = this->modName; //settings->value("LastModulePath").toString();
			//if (path.isEmpty()) path = settings->value("LastOpenPath").toString();
			QString fileName = QFileDialog::getOpenFileName(
			      this,
			      tr("Select an \"actions.txt\" file (hint: it's in the module dir)") ,
			      tr("%1\\actions.txt").arg(modPath()),
			      tr("Txt file(*.txt)")
			      );
			if (fileName.isEmpty()) {
				statusBar()->showMessage(tr("Split canceled."), 2000);
				return;
			}
			//settings->setValue("LastModulePath",QFileInfo(fileName).absolutePath());
			wchar_t newTxt[2048];
			vector<BrfAnimation> resVec;
			int res = ani.Break(resVec, fileName.toStdWString().c_str(),newTxt );

			if (res==0) statusBar()->showMessage(tr("Nothing to split (or could not split)."));
			else {
				for (uint i=0; i<resVec.size(); i++) insert(resVec[i]);
				updateSel();
				inidataChanged();

				setModified();
				//selector->setCurrentIndex(2);

				statusBar()->showMessage(
				      tr("Animation %2 split in %1 chunks -- new animation.txt file save in \"%3\"!")
				      .arg(res).arg(ani.name).arg(QString::fromStdWString(std::wstring(newTxt))), 8000);
			}
		}
	}
}


void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
	// accept just text/uri-list mime format
	//if (event->mimeData()->hasFormat("text/uri-list"))
	//{
	event->acceptProposedAction();
	//}
}

void MainWindow::dropEvent(QDropEvent *event)
{
	QList<QUrl> urlList;
	QString fName;

	if (event->mimeData()->hasUrls())
	{
		urlList = event->mimeData()->urls(); // returns list of QUrls
		// if just text was dropped, urlList is empty (size == 0)


		if ( urlList.size() > 0) // if at least one QUrl is present in list
		{
			QStringList list;
			bool areTextures = true;
			for (int i=0; i<urlList.size(); i++) {
				QFileInfo info( urlList[i].toLocalFile() );
				if (info.suffix().compare("dds",Qt::CaseInsensitive)!=0) areTextures = false;

				list.push_back( info.completeBaseName() );
			}

			if (areTextures) {
				addNewGeneral<BrfTexture>( list );
			} else {
				QFileInfo info;
				QString fn = urlList[0].toLocalFile();
				info.setFile( fn ); // information about file
				if ( info.isFile() ) loadFile( fn ); // if is file, setText
			}
		}
	}
}


int MainWindow::afterMeshImport() const{
	if (optionAfterMeshLoadRecompute->isChecked()) return 2;
	if (optionAfterMeshLoadMerge->isChecked()) return 1;
	return 0;
}

bool MainWindow::usingModReference() const{
	return (optionUseModReference->isChecked());
}

int MainWindow::assembleAniMode() const{
	if (optionAssembleAniMatchVert->isChecked()) return 0;
	if (optionAssembleAniQuiverMode->isChecked()) return 2;
	return 1;
}

int MainWindow::currAoBrightnessLevel() const{
	for (int i=0; i<5; i++) if (optionAoBrightness[i]->isChecked()) return i;
	return 2; // default value?
}

int MainWindow::currAoFromAboveLevel() const{
	for (int i=0; i<2; i++) if (optionAoFromAbove[i]->isChecked()) return i;
	return 1; // default value
}

bool MainWindow::currAoPerFace() const{
	for (int i=0; i<2; i++) if (optionAoPerFace[i]->isChecked()) return i;
	return 0; // default value
}

void MainWindow::optionAutoFixTextureUpdated(){
	if ( (glWidget->fixTexturesOnSight  = optionAutoFixTextureOn->isChecked()) )
		updateGl();
}

void MainWindow::optionLanguageSet0(){setLanguage(0);}
void MainWindow::optionLanguageSet1(){setLanguage(1);}
void MainWindow::optionLanguageSet2(){setLanguage(2);}
void MainWindow::optionLanguageSet3(){setLanguage(3);}
void MainWindow::optionLanguageSet4(){setLanguage(4);}

void MainWindow::optionLanguageSetCustom(){
	if (maybeSave()) {
		nextTranlationFilename = askImportFilename("QLinguist translation file (*.qm)");
		if (!nextTranlationFilename.isEmpty()) {
			glWidget->forgetChachedTextures();
			qApp->exit(101);
		}
	}
}

void MainWindow::setLanguage(int k){


	if (k!=curLanguage) {
		if (!maybeSave()) return;

		curLanguage = k;

		glWidget->forgetChachedTextures();

		// quit and restart
		qApp->exit(101);

		//QMessageBox::information(this,"OpenBrf",tr("Language changed:\nRerun OpenBrf for changes to take place"));
	}
	for (int i=0; i<4; i++) optionLanguage[i]->setChecked(i==k);
	curLanguage = k;

}



void MainWindow::saveOptions() const {

	settings->setValue("afterMeshImport",afterMeshImport());
	settings->setValue("assembleAniMode",assembleAniMode());
	settings->setValue("lastSearchString",lastSearchString);
	//settings->setValue("autoFixDXT1",(int)(glWidget->fixTexturesOnSight));
	settings->setValue("autoZoom",(int)(glWidget->commonBBox));
	settings->setValue("inferMaterial",(int)(glWidget->inferMaterial));
	settings->setValue("groupMode",(int)(glWidget->getViewmodeMult() ));
	settings->setValue("curLanguage",(int)curLanguage);
	settings->setValue("aoBrightness",(int)currAoBrightnessLevel());
	settings->setValue("aoAboveLevel",(int)currAoFromAboveLevel());
	settings->setValue("aoPerFace",(int)currAoPerFace());

	settings->setValue("lod1",lodBuild[0]);
	settings->setValue("lod2",lodBuild[1]);
	settings->setValue("lod3",lodBuild[2]);
	settings->setValue("lod4",lodBuild[3]);

	settings->setValue("lod1Perc",lodPercent[0]);
	settings->setValue("lod2Perc",lodPercent[1]);
	settings->setValue("lod3Perc",lodPercent[2]);
	settings->setValue("lod4Perc",lodPercent[3]);

	settings->setValue("lodReplace",(lodReplace)?1:0);

	settings->setValue("background",background);

	settings->setValue("useOpenGl2",(int)optionUseOpenGL2->isChecked());

	settings->setValue("useCustomFeminizer", (int)optionFeminizerUseCustom->isChecked());

	settings->setValue("useModReference", (int)usingModReference() );


}

QString MainWindow::modPath() const{
	return mabPath+"/Modules/"+modName;
}

void MainWindow::setUseOpenGL2(bool b){
	if (b==glWidget->useOpenGL2) return;
	if (b && askIfUseOpenGL2(false)) {
		glWidget->setUseOpenGL2(true);
		optionUseOpenGL2->setChecked(true);
		statusBar()->showMessage("OpenGL2.0 activated",8000);
	} else {
		glWidget->setUseOpenGL2(false);
		optionUseOpenGL2->setChecked(false);
		guiPanel->ui->cbNormalmap->setChecked(false);
		guiPanel->ui->cbSpecularmap->setChecked(false);
	}
}


void MainWindow::setNormalmap(int k){
	if (!glWidget) return;
	if (k) {
		// setting normalmaps... need to enable opengl2.0
		if (glWidget->useOpenGL2 || askIfUseOpenGL2(true)) {
			glWidget->setUseOpenGL2(true);
			optionUseOpenGL2->setChecked(true);
			glWidget->setNormalmap(true);
		} else {
			guiPanel->ui->cbNormalmap->setChecked(false);
		}
	} else {
		glWidget->setNormalmap(false);
	}
}

void MainWindow::setSpecularmap(int k){
	if (!glWidget) return;
	if (k) {
		// setting normalmaps... need to enable opengl2.0
		if (glWidget->useOpenGL2 || askIfUseOpenGL2(true)) {
			glWidget->setUseOpenGL2(true);
			optionUseOpenGL2->setChecked(true);
			glWidget->setSpecularmap(true);
		} else {
			guiPanel->ui->cbSpecularmap->setChecked(false);
		}
	} else {
		glWidget->setSpecularmap(false);
	}
}

void MainWindow::setUseAlphaCommands(bool mode){
	useAlphaCommands = mode;
	optionAoInAlpha->setVisible(mode);
}


void MainWindow::loadOptions(){

	{
		int k=1;
		QVariant s =settings->value("afterMeshImport");
		if (s.isValid()) k = s.toInt();
		optionAfterMeshLoadRecompute->setChecked(k==2);
		optionAfterMeshLoadMerge->setChecked(k==1);
		optionAfterMeshLoadNothing->setChecked(k==0);
	}

	{
		int k=0;
		QVariant s =settings->value("groupMode");
		if (s.isValid()) k = s.toInt();

		k=2; /* <== override */
		glWidget->setViewmodeMult(k);
		comboViewmodeBG->button(0)->setChecked(k==0);
		comboViewmodeBG->button(1)->setChecked(k==1);
		comboViewmodeBG->button(2)->setChecked(k==2);
	}

	{
		int k=1;
		QVariant s =settings->value("assembleAniMode");
		if (s.isValid()) k = s.toInt();
		optionAssembleAniMatchVert->setChecked(k==0);
		optionAssembleAniMatchTc->setChecked(k==1);
		optionAssembleAniQuiverMode->setChecked(k==2);
	}

	{
		QVariant s =settings->value("curLanguage");
		if (s.isValid()) curLanguage = s.toInt(); else curLanguage = 0;
	}



	{
		int k=0;
		/*QVariant s =settings->value("autoFixDXT1");
	if (s.isValid()) k = s.toInt();
	optionAutoFixTextureOff->setChecked(k==0);
	optionAutoFixTextureOn->setChecked(k==1);*/
		glWidget->fixTexturesOnSight = k;
	}

	{
		int k=0;
		QVariant s =settings->value("autoZoom");
		if (s.isValid()) k = s.toInt();
		optionAutoZoomUseSelected->setChecked(k==0);
		optionAutoZoomUseGlobal->setChecked(k==1);
		glWidget->commonBBox = k;
	}

	/* {
	int k=1;
	QVariant s =settings->value("inferMaterial");
	if (s.isValid()) k = s.toInt();
	optionInferMaterialOff->setChecked(k==0);
	optionInferMaterialOn->setChecked(k==1);
	glWidget->inferMaterial = k;
	}*/

	{
		int k=2;
		QVariant s =settings->value("aoBrightness");
		if (s.isValid()) k = s.toInt();
		for (int h=0; h<5; h++)
			optionAoBrightness[h]->setChecked(h==k);
	}

	{
		int k=0;
		QVariant s =settings->value("useOpenGl2");
		if (s.isValid()) k = s.toInt();
		glWidget->setUseOpenGL2(k);
		//optionUseOpenGL2->blockSignals(true);
		optionUseOpenGL2->setChecked(k);
		//optionUseOpenGL2->blockSignals(false);
		this->guiPanel->ui->cbNormalmap->setChecked(k);
		this->guiPanel->ui->cbSpecularmap->setChecked(k);
		glWidget->setSpecularmap(k);
		glWidget->setNormalmap(k);

	}

	{
		int k=1;
		QVariant s =settings->value("aoAboveLevel");
		if (s.isValid()) k = s.toInt();
		for (int h=0; h<2; h++)
			optionAoFromAbove[h]->setChecked(h==k);
	}

	{
		int k=1;
		QVariant s =settings->value("aoPerFace");
		if (s.isValid()) k = s.toInt();
		for (int h=0; h<2; h++)
			optionAoPerFace[h]->setChecked(h==k);
	}

	{
		int k=0;
		QVariant s =settings->value("useCustomFeminizer");
		if (s.isValid()) k = s.toInt();
		optionFeminizerUseDefault->setChecked(k==0);
		optionFeminizerUseCustom->setChecked(k!=0);
	}

	{
		int k=0;
		QVariant s =settings->value("aoInAlpha");
		if (s.isValid()) k = s.toInt();
		optionAoInAlpha->setChecked(k);
	}

	for (int i=0,defaultVal=5000; i<4; i++,defaultVal/=2){
		bool k=true;
		QVariant s =settings->value(QString("lod%1").arg(i+1));
		if (s.isValid()) k = s.toBool();
		lodBuild[i]=k;
		float f=defaultVal/100.0;
		QVariant s2 =settings->value(QString("lod%1Perc").arg(i+1));
		if (s2.isValid()) f = s2.toDouble();
		lodPercent[i]=f;
	}
	{
		int k=1;
		QVariant s =settings->value("lodReplace");
		if (s.isValid()) k = s.toInt();
		lodReplace = k;
	}

	{
		QColor k(128,128,128);
		QVariant s =settings->value("background");
		if (s.isValid()) k = s.value<QColor>();
		background = k;
	}


	{
		int k=1;
		QVariant s =settings->value("useModReference");
		if (s.isValid()) k = s.toInt();
		optionUseModReference->setChecked(k==1);
		optionUseOwnReference->setChecked(k==0);
	}

	modName = settings->value("modName").toString();
	if (modName.isEmpty()) modName = "native";
	mabPath = settings->value("mabPath").toString();
	inidata.setPath(mabPath,mabPath+"/Modules/"+modName);

	lastSearchString = settings->value("lastSearchString").toString();
}

void MainWindow::updatePaths(){
	modStatus->setText(QString("module:[<b>%1</b>]").arg(modName) );

	glWidget->texturePath[0]=mabPath+"/Textures";
	glWidget->texturePath[1]=modPath()+"/Textures";

	settings->setValue("modName",modName);
	settings->setValue("mabPath",mabPath);
}




bool MainWindow::saveReference(){
	guiPanel->setReference(&reference);
	if ((int)reference.animation.size()>=glWidget->selRefAnimation) glWidget->selRefAnimation=-1;
	if (reference.GetFirstUnusedLetter()>=glWidget->selRefSkin) glWidget->selRefSkin=-1;

	if (usingModReference() && !loadedModReference ) {
		int ans = QMessageBox::question(this,"OpenBRF",tr(
		                                  "<p>You are saving into the generic OpenBRF reference file <br>\"%1\"</p>"
		                                  "<p>Would you rather save in the reference file <i>specific</i> for Module %3<br>\"%2\"<br>?</p>"
		                                  ).arg(referenceFilename(0)).arg(referenceFilename(1)).arg(modName), QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel);

		if (ans==QMessageBox::Cancel) return false;
		if (ans==QMessageBox::Yes) loadedModReference = true;
	}
	QString fn = referenceFilename(loadedModReference);
	//QMessageBox::information(this, "OpenBRF",QString("Saving ref: %1").arg(fn));

	if (!reference.Save(fn.toStdWString().c_str()))
	{
		QMessageBox::warning(this, "OpenBRF",tr("Cannot save reference file!"));
	}
	//setNotModified();
	return true;
}

int MainWindow::GetFirstUnusedRefLetter() const{
	return reference.GetFirstUnusedLetter();
}

void MainWindow::enterOrExitVertexDataMode(){
  setEditingVertexData( !editingVertexData );
}

bool MainWindow::editRef()
{
	if (!maybeSave()) return false;

	if (editingRef) {
		// stop editing refernce file
		curFile = curFileBackup;
		brfdata = brfdataBackup;
		setEditingRef(false);
		findCurFileInIni();
		updateSel();
		return true;
	} else {
		selector->setIniData(NULL,-1);
		curFileBackup = curFile;
		brfdataBackup = brfdata;
		curFile = referenceFilename(loadedModReference);
		brfdata = reference;
		updateSel();
		setEditingRef(true);
		statusBar()->showMessage(tr("Editing reference file..."), 2000);
		updateTitle();
		return true;
	}
}


void MainWindow::inidataChanged(){
	selector->iniDataWaitsSaving = true;
	selector->setIniData(NULL,curFileIndex);
}

bool MainWindow::loadFile(const QString &_fileName)
{
	QString fileName = _fileName;
	fileName.replace("\\","/");
	//QMessageBox::information(this, "OpenBRF",tr("Loading %1.").arg(_fileName));

	if (!maybeSave()) return false;
	setEditingRef(false);

	if (!brfdata.Load(fileName.toStdWString().c_str())) {


		QMessageBox::information(this, "OpenBRF",
		                         tr("Cannot load %1.").arg(fileName));

		return false;

	} else  {
		//tryLoadMaterials();
		//brfdata.Merge(hitboxSet);

		selector->iniDataWaitsSaving = false;
		setCurrentFile(fileName);
		updateSel();

		//glWidget->selectNone();
		//selector->setCurrentIndex(100); // for some reason, if I set the 0 message is not sent
		int first = brfdata.FirstToken();
		if (first>=0) selectOne(first, 0);



		//scanBrfDataForMaterials(brfdata);

		//statusBar()->showMessage(tr("File loaded!"), 2000);
		setNotModified();
		undoHistoryClear();
		return true;
	}
}

bool MainWindow::saveFile(const QString &fileName)
{

	//setCurrentFile(fileName);
	if (curFileIndex>=0 && curFileIndex<(int)inidata.file.size()){
		if (inidata.origin[curFileIndex]!=IniData::MODULE_RES)
			if (QMessageBox::warning(
			      this,"OpenBRF",tr("You are saving a CommonRes file!\n(i.e. not one specific of this module).\n\nAre you sure?"),
			      QMessageBox::Ok, QMessageBox::No
			      )== QMessageBox::No) return false;
	}
	if (brfdata.HasAnyTangentDirs() && brfdata.version==0) {
		QMessageBox::warning(
		      this,"OpenBRF - Warning",tr("You are trying to save meshes with tangent directions in M&B 1.011 file format.\nUnfortunately, tangent directions can only be saved in Warband file format.\nTangent directions will not be saved..."),
		      QMessageBox::Ok
		      );
	}
	if (!brfdata.Save(fileName.toStdWString().c_str())) {
		QMessageBox::information(this, "OpenBRF",
		                         tr("Cannot write file %1.").arg(fileName));
		return false;
	} else {
		statusBar()->showMessage(tr("File saved!"), 2000);
		if (curFileIndex>=0 && curFileIndex<(int)inidata.file.size()){
			inidata.file[curFileIndex]=brfdata; // update ini file
			inidata.updateBeacuseBrfDataSaved();
			selector->iniDataWaitsSaving = false;
			selector->setIniData(&inidata,curFileIndex);
			updateSel();
		}
		setNotModified();
		markCurrendUndoAsSaved();
		return true;
	}
}

void MainWindow::goUsedBy(){
	int k=-1;
	QVariant prop;
	QObject *p = sender();
	if (p) {
		prop = p->property("id");
		if (prop.isValid()) k = prop.toInt();
		else statusBar()->showMessage("Debug: Prop non valid!");
	} else statusBar()->showMessage("Debug: Sender not valid!");
	if (k>=0) {
		if (curFileIndex<(int)inidata.file.size()) {
			ObjCoord c(curFileIndex,selector->firstSelected(), selector->currentTabName());
			const std::vector<ObjCoord> &v(
			      inidata.usedBy(c)
			      //inidata.file[c.fi].GetUsedBy(c.oi, c.t )
			      );
			if (k<(int)v.size()) goTo(v[k]);
			else statusBar()->showMessage(QString("Debug: got %1<%2 (%3)!").arg(k).arg(v.size()).arg(inidata.nameShort( c )));
		}
	}

}

void MainWindow::selectBrfData(){
	if (!maybeSave()) return;
	loadIni(1);

	while (1){
		AskSelectBRFDialog d(this);
		connect(d.openModuleIniButton(),SIGNAL(clicked()),this,SLOT(openModuleIniFile()));
		for (unsigned int k = 0; k<inidata.file.size(); k++){
			int h = inidata.origin[k]==IniData::MODULE_RES?0:2;
			QString shortName = inidata.filename[k];
			shortName = QString("%1. %2")
			    .arg(inidata.iniLine[k])
			    .arg(QFileInfo(shortName).completeBaseName());
			if (inidata.updated>2) {
				int s = inidata.totSize(k);
				if (inidata.updated<4) {
					shortName.append(QString(" (%1)").arg(s));
				} else {
					int u = inidata.totUsed(k);
					shortName.append(QString(" (%2+%1)").arg(s-u).arg(u));
				}
			}
			d.addName(h,shortName,inidata.filename[k]);

		}
		QDir dir(inidata.modPath);
		dir.cd("Resource");

		QStringList filters;
		filters << "*.brf" ;
		dir.setNameFilters(filters);
		dir.setFilter(QDir::Files);
		QStringList list =  dir.entryList();
		for (int i=0; i<list.size(); i++){
			QString name = QFileInfo(list[i]).baseName();
			QString fullname = dir.absoluteFilePath(list[i]);
			if (inidata.findFile(fullname)==-1)
				d.addName(1, name ,fullname);
		}

		//for (int dir.count()
		d.doExec();
		if (d.loadMe=="???1") { loadIni(4); continue; }
		if (d.loadMe=="???2") { refreshIni(); continue; }

		if (!d.loadMe.isEmpty()){
			loadFile(d.loadMe);
		}
		break;
	}
}

void MainWindow::showUnrefTextures(){
	loadIni(4);
	while(1){
		AskUnrefTextureDialog d(this);

		QDir dir(inidata.modPath);
		dir.cd("Textures");

		QStringList filters;
		filters << "*.dds" << "*.TIF" ;
		dir.setNameFilters(filters);
		dir.setFilter(QDir::Files);
		QStringList list =  dir.entryList();
		for (int i=0; i<list.size(); i++){
			QString name = QFileInfo(list[i]).fileName();
			if (!(inidata.findTexture(name)))
				d.addFile(name);
		}

		d.texturePath = dir.canonicalPath();

		if (d.exec()!=QDialog::Accepted) break;
	}

}


void MainWindow::moduleOpenFolder(){
	QDesktopServices::openUrl(modPath());
}


void MainWindow::showModuleStats(){
	loadIni(4);
	QMessageBox::information(this,"OpenBRF",inidata.stats());
}

void MainWindow::computeUsedBy(){
	loadIni(4);
}

bool MainWindow::open()
{
	if (maybeSave()) {
		QString fileName = QFileDialog::getOpenFileName(
		      this,
		      tr("Open File") ,
		      settings->value("LastOpenPath").toString(),
		      tr("Resource (*.brf)")
		      );
		// QDir::currentPath());

		if (!fileName.isEmpty())
			if (!loadFile(fileName)) return false;

		return true;

	}
	return false;
}

bool MainWindow::save()
{
	bool res = false;
	if (editingRef) {
		reference = brfdata;
		statusBar()->showMessage(tr("Reference file saved!"), 4000);
		res = saveReference();
		setNotModified();
		markCurrendUndoAsSaved();
	} else {
		if (curFile.isEmpty()) res = saveAs();
		else res = saveFile(curFile);
	}
	return res;
}

bool MainWindow::saveAs()
{
	QString f0=tr("M&B Resource (*.brf)"),f1 = tr("WarBand Resource v.1 (*.brf)");
	QString selectedf = (brfdata.version==1)?f1:f0;
	QString fileName = QFileDialog::getSaveFileName(this,
	                                                tr("Save File") ,
	                                                (curFile.isEmpty())?
	                                                  settings->value("LastOpenPath").toString():
	                                                  curFile,
	                                                f0+";;"+f1,
	                                                &selectedf
	                                                );


	if (fileName.isEmpty()) return false;

	setEditingRef(false);
	setCurrentFile(fileName);
	brfdata.version=(selectedf==f1)?1:0;
	saveFile(fileName);

	return true;
}

void MainWindow::updateTitle(){
	QString maybestar;

	// signal modified status
	if (isModified&&isModifiedHitboxes) maybestar=QString("(*)(**)");
	else if (isModified) maybestar=QString("(*)");
	else if (isModifiedHitboxes) maybestar=QString("(**)");

	QString notInIni = (curFileIndex==-1)?tr(" [not in module.ini]"):tr("");
	QString tit("OpenBrf");
	if (!editingRef) {
		if (curFile.isEmpty())
			setWindowTitle(tr("%1%2").arg(tit).arg(maybestar));
		else
			setWindowTitle(tr("%1 - %2%3%4").arg(tit).arg(curFile).arg(maybestar).arg(notInIni));
	} else
		setWindowTitle(tr("%1 - editing internal reference data %3 %2").arg(tit).arg(maybestar)
		               .arg((loadedModReference)?tr("(for [%1] mod)").arg(modName):QString()));
}



bool MainWindow::performRedo(){
	if (undoLvlCurr >= undoLvlLast)  return false;

	// apply undo
	QString commandName;

	undoLvlCurr++;
	UndoLevel* currUndo = undoHistory(undoLvlCurr) ;
	if (!currUndo) return false;

	commandName = currUndo->actionDescription();
	isModified = currUndo->needsBeSaved;
	brfdata = currUndo->data;

	statusBar()->showMessage(tr("Redone %1").arg(commandName));
	qDebug() << QString("Redone %1").arg(commandName);

	updateSel();
	updateGui();
	updateGl();
	updateTitle();
	updateUndoRedoAct();

	lastAction = NULL;

	return true;
}

bool MainWindow::performUndo(){
	if (undoLvlCurr <=0 )  return false;


	QString commandName;
	{
		UndoLevel* currUndo = undoHistory(undoLvlCurr);
		if (!currUndo) return false;
		commandName = currUndo->actionDescription();
	}

	undoLvlCurr--;
	UndoLevel* currUndo = undoHistory(undoLvlCurr);
	if (!currUndo) return false;

	isModified = currUndo->needsBeSaved;
	brfdata = currUndo->data;

	statusBar()->showMessage(tr("Undone %1").arg(commandName));
	qDebug() << QString("Undone %1").arg(commandName);

	updateSel();
	updateGui();
	updateGl();
	updateTitle();
	updateUndoRedoAct();

	lastAction = NULL;

	return true;
}

QString UndoLevel::actionDescription() const{
	if (actionRepetitions==1) return actionName;
	else return QString("%1 (x%2)").arg(actionName).arg(actionRepetitions);
}

void MainWindow::updateUndoRedoAct(){

	UndoLevel *currUndo = undoHistory(undoLvlCurr);
	if ((undoLvlCurr > 0) && currUndo ) {
		undoAct->setEnabled(undoHistory(undoLvlCurr-1)!=NULL);
		undoAct->setText(tr("Undo %1").arg(currUndo->actionDescription()));
	} else {
		undoAct->setEnabled(false);
		undoAct->setText(tr("Undo"));
	}

	currUndo = undoHistory(undoLvlCurr+1);
	if ((undoLvlCurr < undoLvlLast) && currUndo ) {
		redoAct->setEnabled(true);
		redoAct->setText(tr("Redo %1").arg(currUndo->actionDescription()));
	} else {
		redoAct->setEnabled(false);
	  redoAct->setText(tr("Redo"));
	}

}


void MainWindow::undoHistoryClear(){

	undoLvlCurr = -1;
	undoLvlLast = -1;

	lastAction = (QAction*)0xFF;
	numModifics = 1;

	undoHistoryAddAction(NULL);

	updateUndoRedoAct();
}

int MainWindow::undoHistoryRingIndex(int lvl) const{
	if (lvl<=undoLvlLast-(int)undoHistoryRing.size()) return -1;
	return (lvl % undoHistoryRing.size());
}
UndoLevel* MainWindow::undoHistory(int lvl){
	int j = undoHistoryRingIndex(lvl);
	if (j==-1) return NULL; else return &(undoHistoryRing[j]);
}

void MainWindow::markCurrendUndoAsSaved(){
	uint j = (uint)undoHistoryRingIndex(undoLvlCurr);
	for (uint i=0; i<undoHistoryRing.size(); i++)
		undoHistoryRing[i].needsBeSaved = (i!=j);
}


void MainWindow::undoHistoryAddAction(){
	undoHistoryAddAction(NULL);
}

void MainWindow::undoHistoryAddEditAction(){
	undoHistoryAddAction(fakeEditAction);
}

void MainWindow::undoHistoryAddAction(QAction *q){

	if (q==repeatLastCommandAct) return;
	if (q==undoAct) return;
	if (q==redoAct) return;
	if (q && q->menu()!=NULL) return;


	//if (q) qDebug("Action was triggered: %s",q->text().toAscii().data());
	//else qDebug("Action was triggered: NULL");


	if (numModifics!=0) {
		QString commandName;
		if (q)
			commandName = q->data().toString() + q->text();
		else
			commandName = "edit";

		if (q!=lastAction) {
			undoLvlCurr++;
			//undoHistory.resize(undoLvlCurr+1);
			//undoHistory[undoLvlCurr].actionRepetitions = 0;
			assert( undoHistory(undoLvlCurr) );
			undoHistory(undoLvlCurr)->actionRepetitions = 0;
		}

		//if (undoLvlLast<undoLvlCurr)
		undoLvlLast=undoLvlCurr;

		//qDebug() << "-- stored! (lvl=" << undoLvlCurr
		//         <<" last=" << undoLvlLast
		//         <<" index="<< undoHistoryRingIndex(undoLvlCurr)<<"/"<<undoHistoryRing.size()<< ")";


		UndoLevel * storeHere = undoHistory(undoLvlCurr);
		storeHere->data = brfdata;
		storeHere->needsBeSaved = isModified;
		storeHere->actionName = commandName;
		int tmp = storeHere->actionName.lastIndexOf("...");
		if (tmp!=-1)storeHere->actionName.truncate(tmp);
		storeHere->actionRepetitions += numModifics;
		numModifics = 0;



		lastAction = q;
	}
	if (!q) lastAction = q;
	updateUndoRedoAct();


}

// called whenever the data is modified (even multiple times in the same action)
void MainWindow::setModified(bool repeatable){
	isModified = true;
	if (repeatable) setNextActionAsRepeatable = true; // action becomes repetable
	//qDebug("set Modified!");
	updateTitle();

	numModifics++; // one more modific done
}


// called just AFTER any action is processed (modifying actions, or not)
void MainWindow::onActionTriggered(QAction *q){

	if (q==repeatLastCommandAct) return;
	if (q==undoAct) return;
	if (q==redoAct) return;

	// update "repeatLastCommand" action
	if (setNextActionAsRepeatable) {
		setNextActionAsRepeatable = false;

		repeatableAction = q;
		tokenOfRepeatableAction = selector->currentTabName();
		QString commandName = q->data().toString() + q->text();

		repeatLastCommandAct->setText(QString("&Repeat %1").arg(commandName));
		repeatLastCommandAct->setEnabled(true);
	}


}

void MainWindow::setNotModified(){
	isModified = false;
	updateTitle();
}




void MainWindow::setModifiedHitboxes(bool mod){
	if (mod!=isModifiedHitboxes) {
		if (mod) saveHitboxAct->setText( saveHitboxAct->text().replace("...","(**)...") );
		else saveHitboxAct->setText( saveHitboxAct->text().replace("(**)...","...") );
	}
	isModifiedHitboxes = mod;
	updateTitle();
}


bool MainWindow::guessPaths(QString fn){
	bool res=false;
	QString _modPath;

	QDir b(fn);
	glWidget->texturePath[2]=b.absolutePath(); //+"/Textures";

	if  (QString::compare(b.dirName(),"Resource",Qt::CaseInsensitive)==0)
		if (b.cdUp())  // go out of "resource"
			if (b.exists("module.ini"))
				if (b.cdUp()) // go out of <modName>
					if (QString::compare(b.dirName(),"modules",Qt::CaseInsensitive )==0) {
						QDir a(fn);
						a.cdUp();
						_modPath = a.absolutePath();
						modName = a.dirName();
						a.cdUp();
						a.cdUp(); // out of "modules"
						mabPath = a.absolutePath();
						usingWarband = (a.exists("mb_warband.exe")||(!a.exists("mount&blade.exe"))) ;
						res=true;
					}

	{
		QDir a(fn);
		a.cdUp();
		if (a.cd("ModuleSystem")) {
			if (a.exists("forOpenBRF.txt"))
				menuBar()->addAction(tldMenuAction);
		}
	}



	QDir a(fn);
	if  (QString::compare(a.dirName(),"commonres",Qt::CaseInsensitive)==0)
		if (a.cdUp()) {
			mabPath = a.absolutePath();
			if (a.cd("Modules") && (a.cd(modName)) ) {
				_modPath = a.absolutePath();
			} else {
				modName = "native";
				_modPath = modPath();
			}
			res=true;
		}

	updatePaths();
	if (inidata.setPath(mabPath,_modPath)) refreshReference();


	loadIni(2);

	if (res) {
		/* store in recent mods */
		QStringList files = settings->value("recentModList").toStringList();
		files.removeAll(_modPath);
		files.prepend(_modPath);
		while (files.size() > MaxRecentFiles)
			files.removeLast();
		settings->setValue("recentModList", files);
		foreach (QWidget *widget, QApplication::topLevelWidgets()) {
			MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
			if (mainWin) mainWin->updateRecentModActions();
		}

		updateGui();
		updateGl();

	}
	return res;
}

void MainWindow::updateSelectedMenu(){
	selectedMenu->clear();
	selector->updateContextMenu();
	selectedMenu->addActions(selector->contextMenu->actions());
}

bool MainWindow::loadIni(int lvl){

	QTime qtime;
	qtime.start();

	if (inidata.updated==0) {
		refreshSkeletonBodiesXml(); // reload skeletons from xml
	}

	bool res = inidata.loadAll(lvl); // if lvl == 2 only tex mat etc

	statusBar()->showMessage( tr("%5 %1 brf files from module.ini of \"%3\"-- %2 msec total [%4 text/mat/shad]").
	                          arg(inidata.file.size()).arg(qtime.elapsed()).arg(modName).arg(inidata.nRefObjects())
	                          .arg((res)?tr("scanned"):tr("ERRORS found while scanning")),6000);

	guiPanel->setIniData(inidata);
	findCurFileInIni();

	if (lvl==4) {
		if (!res) QMessageBox::warning(this,"OpenBRF",inidata.errorStringOnScan);
		updateSel();
	}


	return true;
}

void MainWindow::setCurrentFile(const QString &fileName)
{
	curFile = fileName;

	/* store in recent files */
	QStringList files = settings->value("recentFileList").toStringList();
	files.removeAll(fileName);
	files.prepend(fileName);
	while (files.size() > MaxRecentFiles)
		files.removeLast();
	settings->setValue("recentFileList", files);
	settings->setValue("lastOpenPath",QFileInfo(fileName).absolutePath());
	foreach (QWidget *widget, QApplication::topLevelWidgets()) {
		MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
		if (mainWin) mainWin->updateRecentFileActions();
	}

	QString path = QFileInfo(fileName).canonicalPath();
	if (fileName.endsWith("reference.brf",Qt::CaseInsensitive)) {
		// see if it is a specific mod file...
		QDir dir(path);
		if (dir.cd("Resource")) path = dir.path();
		qDebug("It was a REFERENCE FILE! testing... '%s'",path.toAscii().data());
	}

	// try to guess mab path and module...
	guessPaths( path );



	findCurFileInIni();
}

void MainWindow::findCurFileInIni()
{
	curFileIndex = inidata.findFile(curFile);
	if (curFileIndex>=0) {
		selector->setIniData(&inidata,curFileIndex);
	} else {
		selector->setIniData(NULL,-1);
	}

	if (curFileIndex == -1) selector->setIniData(NULL,-1);
	else selector->setIniData(&inidata,curFileIndex);

	updateTitle();

}
MainWindow::~MainWindow()
{
	saveOptions();
	//delete ui;
}

bool MainWindow::goTo(ObjCoord o){
	if (o.fi!=curFileIndex )
		if (!loadFile( inidata.filename[o.fi]) ) return false;

	curFileIndex = o.fi;

	selectOne(o.t,o.oi);
	return true;
}

bool MainWindow::navigateLeft(){

	int currTab = selector->currentTabName();
	int stackPos=0;
	if (currTab==MATERIAL) stackPos = 1;
	if (currTab==SHADER) stackPos = 2;
	if (currTab==TEXTURE) stackPos = 2;

	if (currTab==MESH) {
		int nextTab = BODY;
		/* cheat: search only in current file */
		BrfMesh &m = getSelected<BrfMesh>();
		if (!&m) return false;
		char nextName[1024];
		sprintf(nextName,m.GetLikelyCollisonBodyName() );
		int loc = brfdata.Find( nextName, nextTab );
		if ( loc!=-1 ) {
			selectOne(nextTab,loc);
			return true;
		} else return false;
	}

	if (!stackPos) return false;

	QPair<ObjCoord,QString> p = navigationStack[stackPos-1];

	if (p.second.isEmpty()) {
		return false;
	}
	qDebug("-1");

	if (p.first.fi==-1) {
		if (!loadFile( p.second )) return false;
		curFileIndex = p.first.fi;
		selectOne(p.first.t,p.first.oi);
	} else {
		if (!goTo(p.first)) return false;
	}


	guiPanel->setNavigationStackDepth( stackPos-1 );

	return true;
}



bool MainWindow::navigateRight(){

	int currTab = selector->currentTabName();

	if ((currTab!=BODY) && (currTab!=MESH) &&  (currTab!=MATERIAL) && (currTab!=SHADER)) {
		return false;
	}

	inidata.loadAll(2); // ini must be loaded, at least mat and textures

	QPair<ObjCoord,QString> old(
		  ObjCoord(curFileIndex,selector->firstSelected(), TokenEnum(selector->currentTabName())),
	      curFile);

	int nextTab = MATERIAL;
	QString nextName;
	int stackPos = -1;


	if (currTab==BODY) {
		nextTab = MESH;
		BrfBody &b = getSelected<BrfBody>();
		if (!&b) return false;
		nextName = QString(b.name);
		if (nextName.startsWith("bo_",Qt::CaseInsensitive)) nextName = nextName.remove(0,3);
		/* cheat: search only in current file */
		int loc = brfdata.Find( nextName.toLatin1().data(), nextTab );
		if ( loc!=-1 ) {
			selectOne(nextTab,loc);
			return true;
		} else return false;
	}
	if (currTab==MESH) {

		if (!guiPanel->ui->boxMaterial->hasFrame()) return false;
		stackPos = 0;
		nextTab = MATERIAL;
		nextName = guiPanel->ui->boxMaterial->text();
	}
	if (currTab==MATERIAL) {
		QLineEdit *le = guiPanel->materialLeFocus();
		if (!le) return false;
		if (!le->hasFrame()) return false;
		nextName = le->text();
		if (nextName==QString("none")) return false;


		if (guiPanel->curMaterialFocus==GuiPanel::SHADERNAME){
			nextTab = SHADER;
		} else {
			nextTab = TEXTURE;
		}
		stackPos = 1;

	}

	if (currTab==SHADER) {
		nextName = guiPanel->ui->leShaderFallback->text();
		ObjCoord p = inidata.indexOf(nextName,SHADER);
		if (!p.isValid()) return false;
		if (!goTo(p)) return false;
		selector->currentWidget()->setFocus();
		return true;
	}

	ObjCoord p = inidata.indexOf(nextName,nextTab);
	if (p.fi==-1) {
		statusBar()->showMessage( tr("Navigate: cannot find \"%1\" in current module").arg(nextName),6000);
		return false;
	}

	if (!goTo(p)) return false;
	navigationStack[stackPos]=old;
	guiPanel->setNavigationStackDepth( stackPos+1 );

	selector->currentWidget()->setFocus();

	return true;

}

void MainWindow::cancelNavStack(){
	//navigationStackPos = 0;
	guiPanel->setNavigationStackDepth( 0 );
	for (int i=0; i<2; i++) navigationStack[i]=
	    QPair<ObjCoord, QString>(ObjCoord::Invalid(),"");
}

void  MainWindow::selectOne(int kind, int i){
	selector->selectOne(kind,i);
}
bool MainWindow::navigateUp(){
	return false;
}
bool MainWindow::navigateDown(){
	return false;
}
bool MainWindow::searchBrf(){
	return false;
}
bool MainWindow::refreshIni(){
	if (!maybeSaveHitboxes()) return false;
	int tmp = inidata.updated;
	inidata.updated=0; // force reload all
	brfdata.ForgetTextureLocations();
	glWidget->forgetChachedTextures();
	glWidget->readCustomShaders();
	loadCarryPositions();
	//refreshSkeletonBodiesXml();

	bool res = loadIni(tmp);
	updateGl();
	updateGui();
	return res;

}
bool MainWindow::openNextInMod(){
	if (!inidata.file.size()) return false;
	if (!maybeSave()) return false;

	if (curFileIndex == -1) curFileIndex=0;
	else {
		curFileIndex++;
		if (curFileIndex>=(int)inidata.file.size()-1) return false;
	}
	return loadFile(inidata.filename[curFileIndex]);
}

bool MainWindow::openPrevInMod(){
	if (!inidata.file.size()) return false;
	if (!maybeSave()) return false;

	if (curFileIndex == -1) curFileIndex=0;
	else {
		if (curFileIndex ==0) return false;
		curFileIndex--;
	}

	return loadFile(inidata.filename[curFileIndex]);
}

bool MainWindow::checkIni(){

	if (!maybeSave()) return false;

	AskModErrorDialog *d=new AskModErrorDialog(this, inidata, false, "");
	d->setup();
	d->exec();
	if (d->i>=0) {
		loadFile(inidata.filename[d->i]);
		selectOne(d->kind, d->j);
	}
	delete d;
	return true;
}

bool MainWindow::refreshSkeletonBodiesXml(){
	// try read mod speicfic file

	QString fn = modPath()+"/Data/skeleton_bodies.xml";
	int res = hitboxSet.LoadHitBoxesFromXml(fn.toStdWString().c_str());

	// if file not found: try reading default path
	if (res==-1) {
		fn = mabPath+"/Data/skeleton_bodies.xml";
		res = hitboxSet.LoadHitBoxesFromXml(fn.toStdWString().c_str());
	}

	if (res==1) {
		lastSkeletonBodiesXmlPath =  fn;
		setModifiedHitboxes(false);
		return true; // read correctly
	}

	if (res==-1) return false; // file not found: fail silently

	// problem with file: give a warning
	QMessageBox::warning(this, tr("OpenBrf"), QString("Error loading skeleton hitbox: ")
	                     +BrfData::LastHitBoxesLoadSaveError() );

	return false;
}

bool MainWindow::searchIni(){

	int oldIni = inidata.updated;
	if (!maybeSave()) return false;

	static bool optA = true;
	static int optB = -1;
	static QString optC;
	AskModErrorDialog *d=new AskModErrorDialog(this, inidata, true, lastSearchString );
	d->setOptions(optA,optB,optC);
	d->setup();
	d->exec();
	if (d->i>=0) {
		loadFile(inidata.filename[d->i]);
		selectOne(d->kind, d->j);
	} else {
		if (inidata.updated!=oldIni) updateSel(); // refresh colors
	}
	d->getOptions(&optA,&optB,&optC);
	delete d;
	return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
	if (maybeSave()) {
		event->accept();
	} else {
		event->ignore();
	}
}

void MainWindow::newFile(){
	if (maybeSave()) {
		loadIni(2);
		brfdata.Clear();
		curFile.clear();
		curFileIndex = -1;
		setNotModified();
		undoHistoryClear();

		updateGui();
		updateGl();
		updateSel();
		updateTitle();
		inidataChanged();
		setEditingRef(false);
	}
}


void MainWindow::registerExtension(){
	QString exeFile = QCoreApplication::applicationFilePath();
	exeFile.replace('/',QString("\\\\"));

	//QSettings settings(QSettings::NativeFormat, QSettings::SystemScope,"HKEY_CLASSES_ROOT");
	{
		//QSettings settings("HKEY_CLASSES_ROOT", QSettings::NativeFormat);
		//QSettings settings("HKEY_LOCAL_MACHINE", QSettings::NativeFormat);
		//settings.beginGroup("SOFTWARE");
		//settings.beginGroup("Classes");


		//settings.beginGroup(".brf");
		QSettings settings(QSettings::NativeFormat,QSettings::SystemScope, "classes", ".brf");
		settings.setValue("","brf.resourceT");
		//settings.endGroup();
	}
	//QSettings settings("HKEY_CLASSES_ROOT", QSettings::NativeFormat);
	QSettings settings(QSettings::NativeFormat,QSettings::SystemScope,"classes", "brf.resource");

	//settings.beginGroup("brf.resource");
	//settings.setValue("","Mount&Blade Binary Resource File");
	settings.setValue("","");
	settings.setValue("FriendlyTypeName","Mount&Blade Binary Resource File");
	settings.setValue("PerceivedType","Application");

	settings.beginGroup("DafualtIcon");
	settings.setValue("",QString("%1%2 test").arg(exeFile).arg(",0") );
	settings.endGroup();

	settings.beginGroup("shell");
	//settings.setValue("","");
	settings.beginGroup("open");
	//settings.setValue("","");
	settings.beginGroup("command");
	settings.setValue("",
	                  QString("\"%1\" \"%2\"").
	                  arg(exeFile).arg("%1") );
	settings.endGroup();
	settings.endGroup();
	settings.endGroup();
	//settings.endGroup();
	//QSettings::Format brfFormat = QSettings::registerFormat("brf",readFile,writeFile,Qt::CaseInsensitive);

	//QSettings fsettings(brfFormat, QSettings::UserSettings, "mtarini", "openBrf");

	//fsettings.setValue
	//int f = QWindowsMime::registerMimeType("brf");
	//if (!f) statusBar()->showMessage("Failed");
	//statusBar()->showMessage(tr("Registered %1?").arg(f));
	//QSetting sett(brfFormat,
}

bool MainWindow::openRecentFile()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (action)
		return loadFile(action->data().toString());
	return false;
}

bool MainWindow::openRecentMod()
{
	if (!maybeSaveHitboxes()) return false;
	QAction *action = qobject_cast<QAction *>(sender());
	if (action)
		return guessPaths(action->data().toString()+"/Resource");
	return false;
}


void MainWindow::updateRecentFileActions()
{
	QStringList files = settings->value("recentFileList").toStringList();

	int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

	for (int i = 0; i < numRecentFiles; ++i) {
		QString text = tr("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
		recentFileActs[i]->setText(text);
		recentFileActs[i]->setData(files[i]);
		recentFileActs[i]->setVisible(true);
	}
	for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
		recentFileActs[j]->setVisible(false);

	separatorAct->setVisible(numRecentFiles > 0);
}


void MainWindow::updateRecentModActions()
{
	QStringList files = settings->value("recentModList").toStringList();

	int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

	for (int i = 0; i < numRecentFiles; ++i) {
		recentModActs[i]->setText(files[i]);
		recentModActs[i]->setData(files[i]);
		recentModActs[i]->setVisible(true);
	}
	for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
		recentModActs[j]->setVisible(false);

}


QString MainWindow::strippedName(const QString &fullFileName)
{
	return QFileInfo(fullFileName).fileName();
}

void MainWindow::optionLodSettings(){
	AskLodOptionsDialog *d = new AskLodOptionsDialog(this);
	d->setData(lodPercent, lodBuild, lodReplace);
	if (d->exec()==QDialog::Accepted){
		d->getData(lodPercent, lodBuild, &lodReplace);
		statusBar()->showMessage(tr("New Lod parameters set"));
	}
	statusBar()->showMessage(tr("Cancelled"));
}

void MainWindow::optionSetBgColor(){
	QColor color = QColorDialog::getColor(QColor(128,128,128,255), this);
	if (!color.isValid()) return;
	else {
		background = color;
		glWidget->setDefaultBgColor(background,!editingRef);
	}

}


#include "askFlagsDialog.h"

template<class BrfType> void MainWindow::getAllFlags(const vector<BrfType> &v, unsigned int &curfOR, unsigned int &curfAND){
	QModelIndexList list=selector->selectedList();
	curfOR=0;
	curfAND = 0xFFFFFFFF;
	for (int i=0; i<(int)list.size(); i++) {
		int sel = list[i].row();
		if (sel<0 || sel>=(int)v.size()) continue;
		curfOR |= v[sel].flags;
		curfAND &= v[sel].flags;
	}
}

void MainWindow::getAllRequires(const vector<BrfShader> &v, unsigned int &curfOR, unsigned int &curfAND){
	QModelIndexList list=selector->selectedList();
	curfOR=0;
	curfAND = 0xFFFFFFFF;
	for (int i=0; i<(int)list.size(); i++) {
		int sel = list[i].row();
		if (sel<0 || sel>=(int)v.size()) continue;
		curfOR |= v[sel].requires;
		curfAND &= v[sel].requires;
	}
}

template<class BrfType> bool MainWindow::setAllFlags(vector<BrfType> &v, unsigned int toZero, unsigned int toOne){
	bool mod = false;
	QModelIndexList list=selector->selectedList();
	for (int i=0; i<(int)list.size(); i++) {
		int sel = list[i].row();
		if (sel<0 || sel>=(int)v.size()) continue;
		unsigned int oldflags = v[sel].flags;
		v[sel].flags |= toOne;
		v[sel].flags &= toZero;
		if (oldflags!=v[sel].flags) mod = true;
	}
	updateGui();
	if (mod) {
		setModified();
		undoHistoryAddAction(fakeEditFlagAction);
	}
	return mod;
}

bool MainWindow::setAllRequires(vector<BrfShader> &v, unsigned int toZero, unsigned int toOne){
	bool mod = false;
	QModelIndexList list=selector->selectedList();
	for (int i=0; i<(int)list.size(); i++) {
		int sel = list[i].row();
		if (sel<0 || sel>=(int)v.size()) continue;
		unsigned int oldreqs = v[sel].requires;
		v[sel].requires |= toOne;
		v[sel].requires &= toZero;
		if (oldreqs!=v[sel].requires) mod = true;
	}
	updateGui();
	if (mod) setModified();
	return mod;
}



#define TR AskFlagsDialog::tr
void MainWindow::setFlagsBody(){

	QString FlagNameArray[64] = {
	  TR("Two-sided"),"",
	  TR("No Collision"),"",
	  TR("No Shadow"),TR("Game won't use this collision object"),
	  "","",
	  "","",
	  "","",
	  "","",
	  "","",

	  "","",
	  "","",
	  "","",
	  "","",
	  "","",
	  "","",
	  "","",
	  "","",

	  TR("Difficult"),"",
	  TR("Unwalkable"),"",
	  "","",
	  "","",
	  "","",
	  "","",
	  "","",
	  "","",

	  "","",
	  "","",
	  "","",
	  "","",
	  "","",
	  "","",
	  "","",
	  "","",

	};

	int sel = selector->firstSelected();
	if (sel<0 || sel>=(int)brfdata.body.size()) return;

	int selp = guiPanel->getCurrentSubpieceIndex(BODY);
	if (selp<0 || selp>(int)brfdata.body[sel].part.size()) return;
	BrfBodyPart &p(brfdata.body[sel].part[selp]);

	if (p.type==BrfBodyPart::MANIFOLD) return;

	AskFlagsDialog *d = new AskFlagsDialog(this,tr("Collision objects flags"), p.flags,p.flags, FlagNameArray);

	if (d->exec()==QDialog::Accepted) {

		uint flags = d->toOne();

		if (p.flags != flags) {
			p.flags = flags;
			setModified();
			guiPanel->updateBodyPartData();
		}
	}

}
void MainWindow::setFlagsMesh(){

	QString FlagNameArray[64] = {
	  TR("Unknown (for props?)"),TR("Exact meaning of this flag is unknown."),
	  TR("Unknown (for particles?)"),TR("Exact meaning of this flag is unknown."),
	  TR("Unknown (plants?)"),TR("Exact meaning of this flag is unknown."),
	  TR("Unknown (for particles?)"),TR("Exact meaning of this flag is unknown."),
	  "","",
	  TR("Unknown (hairs and body parts?)"),TR("Exact meaning of this flag is unknown."),
	  "","",
	  "","",

	  TR("Unknown (for particles?)"),TR("Exact meaning of this flag is unknown."),
	  TR("Unknown (screen space?)"),TR("Exact meaning of this flag is unknown."),
	  "","",
	  "","",
	  "","",
	  "","",
	  "","",
	  "","",

	  TR("R: (tangent space)"),TR("This flag is automatically set if mesh has tangent directions defined."),
	  TR("R: (Warband format)"),TR("This flag is automatically set for meshes in WB formats"),
	  "","",
	  "","",
	  "","",
	  "","",
	  "","",
	  "","",

	  TR("Pre-exponentiate colors"),TR("Vertex colors will be pre-exponentiated (for gamma corrections) if this flag is set."),
	  "","",
	  TR("Unknown (for particles?)"),TR("Exact meaning of this flag is unknown."),
	  "","",
	  "","",
	  "","",
	  "","",
	  "","",

	};

	unsigned int curfOR, curfAND;
	getAllFlags( brfdata.mesh, curfOR, curfAND );

	AskFlagsDialog *d = new AskFlagsDialog(this,tr("Mesh flags"), curfOR,curfAND, FlagNameArray);

	if (d->exec()==QDialog::Accepted) {
		setAllFlags(brfdata.mesh,d->toZero(),d->toOne() );
	}

}

void MainWindow::setFlagsTexture(){

	QString FlagNameArray[64] = {
	  TR("Unknown"),"",
	  TR("Force hi-res"),TR("By default, depending on the game settings, higher-res mipmap levels might by not loaded"),
	  TR("Unknown"),"",
	  TR("Languange dependent"),TR("If set, depending on the game language settings, this texture is substituted by the one found in the language folder (WB only)"),
	  TR("HDR only"),TR("If High-Dynamic-Ramge is off, this texture won't be loaded"),
	  TR("No HDR"),TR("If High-Dynamic-Ramge is on, this texture won't be loaded"),
	  "","",
	  TR("Unknown"),"",

	  "","",
	  "","",
	  "","",
	  "","",
	  "B:","", // maybe mipmaps X
	  "B:","",
	  "B:","",
	  "B:","",

	  "B:","", // maybe mipmaps Y
	  "B:","",
	  "B:","",
	  "B:","",
	  TR("Clamp U"),TR("By default, texture U is set to wrap (horizontally tiled texture)"),
	  TR("Clamp V"),TR("By default, texture V is set to wrap (vertically tiled texture)"),
	  "","",
	  "","",

	  "B:","", // animation frames
	  "B:","",
	  "B:","",
	  "B:","",
	  "","",
	  "","",
	  "","",
	  "","",

	};

	unsigned int curfOR, curfAND;
	getAllFlags( brfdata.texture, curfOR, curfAND );

	AskFlagsDialog *d = new AskFlagsDialog(this,tr("Texture flags"), curfOR,curfAND, FlagNameArray);

	int aniVals[16] = {0,4,8,12,16,20,24,28,32,36,40,44,48,52,56,60};
	d->setBitCombo(TR("Animation frames"),TR("N. of frames of texture anim (append \"_0\", \"_1\" ... to dds file names)."),24,28,aniVals);
	const char* aniPows[16] = {"default","2","4","8","16","32","64","128","256","512","1024","2K", "4K","8K","16K","32K",};
	d->setBitCombo(TR("Size U (?)"),TR("Unclear meaining, usually only set for facial textures"),12,16,aniPows);
	d->setBitCombo(TR("Size V (?)"),TR("Unclear meaining, usually only set for facial textures"),16,20,aniPows);

	if (d->exec()==QDialog::Accepted) {
		setAllFlags(brfdata.texture, d->toZero(), d->toOne() );
	}

}


void MainWindow::setFlagsMaterial(){

	QString FlagNameArray[64] = {
	  TR("No fog"),TR("This object must not be affected by fog"),
	  TR("No Lighting"),TR("This object won't be dynamically relit"),
	  "","",
	  TR("No Z-write"),TR("Rendering object leaves the depth buffer unaffected"),
	  TR("No depth Test"),TR("Object ignores the depth test: i.e. it will be always drawn over others."), //
	  TR("Specular enable"),TR("Specular reflections are enabled."),
	  TR("Unknown (for alpha test?)"),TR("Exact meaning of this flag is unknown."),
	  TR("Uniform lighting"),"",


	  TR("Blend"),TR("Enable alpha-blending (for semi-transparencty)"),
	  TR("Blend add"),TR("Alpha-blend function: add"),
	  TR("Blend multiply"),TR("Alpha-blend function: mulitply"),
	  TR("Blend factor"),TR("Alpha-blend function: factor"),
	  "B:","", // alpha test value
	  "B:","", // alpha test value
	  TR("Unknown (for alpha test?)"),TR("Exact meaning of this flag is unknown."),
	  "","",

	  TR("Render 1st"),"",
	  TR("Origin at camera"),"",
	  TR("LoD"),TR("If set, this material is optimized for LODs>1"),
	  "","",
	  "","",
	  "","",
	  "","",
	  "","",

	  "B:","", // render order
	  "B:","",
	  "B:","",
	  "B:","",

	  TR("Invert bumpmap"),TR("If set, bumpmap should be considered inverted on Y axis"),
	  "","",
	  "","",
	  "","",

	};

	unsigned int curfOR, curfAND;
	getAllFlags( brfdata.material, curfOR, curfAND );

	AskFlagsDialog *d = new AskFlagsDialog(this,tr("Material flags"), curfOR,curfAND, FlagNameArray);

	int vals[16] = {-8,-7,-6,-5,-4,-3,-2,-1,0,1,2,3,4,5,6,7};
	d->setBitCombo(TR("Render order"),TR("Determines what is rendered first (neg number), or later (pos numbers)"),24,28, vals,8);

	const char* valsAlpha[4] = {"No","< 8/256","< 136/256","< 251/256"};
	d->setBitCombo(TR("Alpha test:"),TR("Alpha testing (for cutouts). Pixels more transparent than a given number will be not drawn."),12,14, valsAlpha);

	if (d->exec()==QDialog::Accepted) {
		setAllFlags(brfdata.material, d->toZero(), d->toOne() );
	}

}

void MainWindow::setFlagsShaderRequires(){

	QString FlagNameArray[64] = {
	  "","",
	  "","",
	  "","",
	  "","",

	  "","",
	  "","",
	  "","",
	  "","",

	  "","",
	  "","",
	  "","",
	  "","",

	  TR("pixel shader"), TR("requires config setting use_pixel_shaders and video card PS 1.1 capability"),// 0x1000
	  TR("mid quality"),  TR("requires config setting shader_quality > 0"),// 0x2000
	  TR("hi quality"), TR("requires config setting shader_quality > 1 and some additional video card PS 2.0a/b capabilities"),  // 0x4000

	  "","",
	  "","",
	  "","",
	  "","",

	  "","",
	  "","",
	  "","",
	  "","",

	  "","",
	  "","",
	  "","",
	  "","",

	  "","",
	  "","",
	  "","",
	  "","",

	};

	unsigned int curfOR, curfAND;
	getAllRequires( brfdata.shader, curfOR, curfAND );

	AskFlagsDialog *d = new AskFlagsDialog(this,tr("Shader Requirements"), curfOR,curfAND, FlagNameArray );

	if (d->exec()==QDialog::Accepted) {
		setAllRequires( brfdata.shader, d->toZero(), d->toOne() );
	}

}

void MainWindow::setFlagsShader(){

	QString FlagNameArray[64] = {
	  "","",
	  "","",
	  "","",
	  "","",

	  "","",
	  TR("specular enable"),TR("enables specular light"),    // 0x20
	  "","",
	  TR("static_lighting"),TR("meshes using this shader will simulate lighting by vertex painting (static, on scene creation)"),

	  "","",
	  "","",
	  "","",
	  "","",

	  TR("preshaded")        ,TR("uses preshaded technique"), //0x1000
	  TR("uses instancing")  ,TR("shader receives instance data as input (TEXCOORD1..4)"), //0x2000
	  "","",
	  TR("biased")           ,TR("used for shadowmap bias"), //0x8000

	  "","",
	  "","",
	  "","",
	  "","",

	  "","",
	  "","",
	  "","",
	  "","",

	  "","",
	  "","",
	  "","",
	  "","",

	  TR("uses pixel shader"),TR("this shader uses pixel shader"), //0x10000000
	  TR("uses HLSL")        ,TR("if not set the FFP will be used"), //0x20000000
	  TR("uses normal map")  ,TR("shader receives binormal and tangent as input (TANGENT, BINORMAL)"), //0x40000000
	  TR("uses skinning")    ,TR("shader receives skinning data as input (BLENDWEIGHTS, BLENDINDICES)"), //0x80000000

	};

	unsigned int curfOR, curfAND;
	getAllFlags( brfdata.shader, curfOR, curfAND );

	AskFlagsDialog *d = new AskFlagsDialog(this, tr("Shader flags"), curfOR,curfAND, FlagNameArray);

	if (d->exec()==QDialog::Accepted) {
		setAllFlags(brfdata.shader, d->toZero(), d->toOne() );
	}

}


