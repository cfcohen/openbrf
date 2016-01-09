/* OpenBRF -- by marco tarini. Provided under GNU General Public License */


#include "askBoneDialog.h"
#include "ui_askBoneDialog.h"


AskBoneDialog::AskBoneDialog(QWidget *parent,const std::vector<BrfSkeleton> &s,
                             const std::vector<CarryPosition> &_cp) :
    QDialog(parent),
    //sv(s),
    ui(new Ui::AskBoneDialog)

{
  sv=s;
	carrypos=_cp;
  ui->setupUi(this);

  for (int i=0; i<(int)sv.size(); i++)
    ui->cbSlel->addItem( sv[i].name );

  onSelectSkel(0);
  ui->radioButton->setChecked(true);
  ui->radioButton_2->setChecked(false);
  connect(ui->cbSlel, SIGNAL(currentIndexChanged(int)), this, SLOT(onSelectSkel(int)) );
	connect(ui->cbCarryPos, SIGNAL(currentIndexChanged(int)), this, SLOT(onSelectCarryPos(int)) );
	connect(ui->cbBone, SIGNAL(currentIndexChanged(int)), this, SLOT(onSelectBone(int)) );

}

void AskBoneDialog::onSelectBone(int /*i*/){
	if (ui->cbCarryPos->currentIndex()!=0)
		ui->cbCarryPos->setCurrentIndex(0);
}

void AskBoneDialog::onSelectCarryPos(int i){
	/*
	i--;
	if (i<0) return;
	int boneIndex = sv[ ui->cbSlel->currentIndex() ].FindBoneByName(
	  carrypos[i].boneName
	);
	qDebug("Found bone %d for name %s",boneIndex, carrypos[i].boneName);
	ui->cbBone->blockSignals(true);
	ui->cbBone->setCurrentIndex( boneIndex );
	ui->cbBone->blockSignals(false);
	*/
	i--;
	if (i<0) {
		ui->cbBone->setEnabled(true);
		ui->radioButton->setEnabled(true);
		ui->radioButton_2->setEnabled(true);
	} else {
		int boneIndex = sv[ ui->cbSlel->currentIndex() ].FindBoneByName(
			carrypos[i].boneName
		);
		//qDebug("Found bone %d for name %s",boneIndex, carrypos[i].boneName);
		ui->cbBone->blockSignals(true);
		ui->cbBone->setCurrentIndex( boneIndex );

		ui->radioButton->setChecked(true);
		ui->radioButton_2->setChecked(false);
		ui->cbBone->blockSignals(false);
		ui->cbBone->setEnabled(false);
		ui->radioButton->setEnabled(false);
		ui->radioButton_2->setEnabled(false);
	}

}

void AskBoneDialog::sayNotRigged(bool say){
  ui->label_3->setVisible(say);
}

bool AskBoneDialog::pieceAtOrigin()const{
  return ui->radioButton->isChecked();
}

void AskBoneDialog::onSelectSkel(int i){
  ui->cbBone->clear();
  for (unsigned int j=0; j<sv[i].bone.size(); j++)
    ui->cbBone->addItem( sv[i].bone[j].name );

	QString currText = ui->cbCarryPos->currentText();
	int currIndex = 0;
	ui->cbCarryPos->clear();

	// update carrypos array (show only ones with bones)
	ui->cbCarryPos->blockSignals(true);
	BrfSkeleton &s(sv[i]);
	ui->cbCarryPos->addItem( tr("<none>"));
	for (unsigned int j=0; j<carrypos.size(); j++){
		if (s.FindBoneByName(carrypos[j].boneName)!=-1) {
			QString carryName(carrypos[j].name);
			//if (carryName.startsWith("itcf_carry_"))
			//	carryName = carryName.remove(0,11);
			ui->cbCarryPos->addItem( carryName );
			if (carryName==currText) currIndex = j+1;
		}
	}
	ui->cbCarryPos->setCurrentIndex(currIndex);
	ui->cbCarryPos->blockSignals(false);
}

int AskBoneDialog::getSkel() const {
  return ui->cbSlel->currentIndex();
}

int AskBoneDialog::getCarryPos() const{
	return ui->cbCarryPos->currentIndex()-1; // -1 for <none>
}

int AskBoneDialog::getBone() const {
  return ui->cbBone->currentIndex();
}

AskBoneDialog::~AskBoneDialog()
{
    delete ui;
}

void AskBoneDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
