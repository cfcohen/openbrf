/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#include <QFileDialog>
#include <math.h>

#include "askNewUiPictureDialog.h"
#include "ui_askNewUiPictureDialog.h"

#include "ddsData.h"


AskNewUiPictureDialog::AskNewUiPictureDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AskNewUiPictureDialog)
{
    name[0]=0;
    ui->setupUi(this);
    ui->overlayDarken->setChecked(overlayMode==0);
    ui->overlayNormal->setChecked(overlayMode==1);
    ui->overlaySubstitute->setChecked(overlayMode==2);
    ui->measurePixel->setChecked(measurePix);
    ui->measurePerc->setChecked(!measurePix);
    ui->replaceOpt->setChecked(replace);

    showSizeAndPos();
    connect(ui->measurePerc,SIGNAL(clicked()),this,SLOT(updateMeasure()));
    connect(ui->measurePixel,SIGNAL(clicked()),this,SLOT(updateMeasure()));
    connect(ui->setFullScreen,SIGNAL(clicked()),this,SLOT(resetFullScreen()));
    connect(ui->browse,SIGNAL(clicked()),this,SLOT(browse()));
    connect(ui->setActualPixels,SIGNAL(clicked()),this,SLOT(resetActualPixels()));

    connect(ui->pushButton_NW, SIGNAL(clicked(bool)),this,SLOT(resetNW()));
    connect(ui->pushButton_N, SIGNAL(clicked(bool)),this,SLOT(resetN()));
    connect(ui->pushButton_NE, SIGNAL(clicked(bool)),this,SLOT(resetNE()));
    connect(ui->pushButton_W, SIGNAL(clicked(bool)),this,SLOT(resetW()));
    connect(ui->pushButton_C, SIGNAL(clicked(bool)),this,SLOT(resetC()));
    connect(ui->pushButton_E, SIGNAL(clicked(bool)),this,SLOT(resetE()));
    connect(ui->pushButton_SW, SIGNAL(clicked(bool)),this,SLOT(resetSW()));
    connect(ui->pushButton_S, SIGNAL(clicked(bool)),this,SLOT(resetS()));
    connect(ui->pushButton_SE, SIGNAL(clicked(bool)),this,SLOT(resetSE()));

    ui->setActualPixels->setEnabled(false);

    ext = "dds";
}

AskNewUiPictureDialog::~AskNewUiPictureDialog()
{
    delete ui;
}



void AskNewUiPictureDialog::resetNW(){ center(0,  1); }
void AskNewUiPictureDialog::resetN (){ center(0.5,1); }
void AskNewUiPictureDialog::resetNE(){ center(1,  1); }
void AskNewUiPictureDialog::resetW (){ center(0,  0.5); }
void AskNewUiPictureDialog::resetC (){ center(0.5,0.5); }
void AskNewUiPictureDialog::resetE (){ center(1,  0.5); }
void AskNewUiPictureDialog::resetSW(){ center(0,  0); }
void AskNewUiPictureDialog::resetS (){ center(0.5,0); }
void AskNewUiPictureDialog::resetSE(){ center(1,  0); }

void AskNewUiPictureDialog::center(float relposX, float relposY){
  px = (100.0-sx)*relposX;
  py = (100.0-sy)*relposY;
  showSizeAndPos();
}

void AskNewUiPictureDialog::onUiChanged(){
  measurePix = ui->measurePixel->isChecked();
  if (ui->overlayDarken->isChecked()) overlayMode = 0;
  if (ui->overlayNormal->isChecked()) overlayMode = 1;
  if (ui->overlaySubstitute->isChecked()) overlayMode = 2;

  if (measurePix){
    sx = toPerX( ui->sizeX->text().toInt() );
    sy = toPerY( ui->sizeY->text().toInt() );
    px = toPerX( ui->posX->text().toInt() );
    py = toPerY( ui->posY->text().toInt() );
  } else {
    sx = ui->sizeX->text().toFloat() ;
    sy = ui->sizeY->text().toFloat() ;
    px = ui->posX->text().toFloat() ;
    py = ui->posY->text().toFloat() ;
  }
  replace = ui->replaceOpt->isChecked();

  sprintf(name, "%s", ui->NameBox->text().toLatin1().data());
}

void AskNewUiPictureDialog::accept(){
  onUiChanged();
  QDialog::accept();
}

int AskNewUiPictureDialog::overlayMode=0;
bool AskNewUiPictureDialog::measurePix=true;
float AskNewUiPictureDialog::sx=100.0;
float AskNewUiPictureDialog::sy=100.0;
float AskNewUiPictureDialog::px=0.0;
float AskNewUiPictureDialog::py=0.0;
int AskNewUiPictureDialog::mode = 0;
char AskNewUiPictureDialog::name[255];
bool AskNewUiPictureDialog::replace = true;

int AskNewUiPictureDialog::toPixX(float x){return int(round(x/100*1024));}
int AskNewUiPictureDialog::toPixY(float x){return int(round(x/100*768));}
float AskNewUiPictureDialog::toPerX(int x){return x*100/1024.0f;}
float AskNewUiPictureDialog::toPerY(int x){return x*100/768.0f;}

void AskNewUiPictureDialog::updateMeasure(){
  bool oldMeasurePix = measurePix;
  measurePix = ui->measurePixel->isChecked();
  if (oldMeasurePix == measurePix) return;
  if (measurePix){
    ui->sizeX->setText(QString("%1").arg( toPixX(ui->sizeX->text().toFloat()) ) );
    ui->sizeY->setText(QString("%1").arg( toPixY(ui->sizeY->text().toFloat()) ) );
    ui-> posX->setText(QString("%1").arg( toPixX(ui-> posX->text().toFloat()) ) );
    ui-> posY->setText(QString("%1").arg( toPixY(ui-> posY->text().toFloat()) ) );
  } else {
    ui->sizeX->setText(QString("%1").arg( toPerX(ui->sizeX->text().toInt()) ) );
    ui->sizeY->setText(QString("%1").arg( toPerY(ui->sizeY->text().toInt()) ) );
    ui-> posX->setText(QString("%1").arg( toPerX(ui-> posX->text().toInt()) ) );
    ui-> posY->setText(QString("%1").arg( toPerY(ui-> posY->text().toInt()) ) );
  }


}

void AskNewUiPictureDialog::showSizeAndPos(){
  if (measurePix){
    ui->sizeX->setText(QString("%1").arg( toPixX(sx) ));
    ui->sizeY->setText(QString("%1").arg( toPixY(sy) ));
    ui->posX->setText(QString("%1").arg( toPixX(px) ));
    ui->posY->setText(QString("%1").arg( toPixY(py) ));
  } else {
    ui->sizeX->setText(QString("%1").arg( sx ) );
    ui->sizeY->setText(QString("%1").arg( sy ) );
    ui->posX->setText(QString("%1").arg( px ) );
    ui->posY->setText(QString("%1").arg( py ) );
  }
}

void AskNewUiPictureDialog::resetFullScreen(){
  sx=100; sy=100; px=0; py=0;
  showSizeAndPos();
}

void AskNewUiPictureDialog::setBrowsable(QString s){
  path = s;
}

bool loadOnlyDDSHeader(const QString &fileName, DdsData &data);


void AskNewUiPictureDialog::browse()
{
  QString fileName =QFileDialog::getOpenFileName(
    this,
    tr("Select a texture for a menu background file") ,
    path,
    QString("Direct Draw Texture (*.dds)")
  );

  if (!fileName.isEmpty()) {
    QString base = QFileInfo(fileName).baseName();
    ext = QFileInfo(fileName).completeSuffix();
    sprintf(name,"%s",base.toLatin1().data());
    ui->NameBox->setText(QString(name));
    DdsData data;
    if (loadOnlyDDSHeader(fileName,data)) {
      pixSx = data.sx;
      pixSy = data.sy;
      ui->setActualPixels->setEnabled(true);
      resetActualPixels();
    }
  }
}

void AskNewUiPictureDialog::resetActualPixels(){
  sx = toPerX(pixSx);
  sy = toPerY(pixSy);
  showSizeAndPos();
}

void AskNewUiPictureDialog::changeEvent(QEvent *e)
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
