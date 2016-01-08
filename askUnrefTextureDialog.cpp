/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#include <qdir.h>
#include <qprocess.h>
#include <qmessagebox.h>

#include "askUnrefTextureDialog.h"
#include "ui_askUnrefTextureDialog.h"


AskUnrefTextureDialog::AskUnrefTextureDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AskUnrefTextureDialog)
{
    ui->setupUi(this);
    //ui->listWidget->setEnabled(false);
    connect(ui->pushButton,SIGNAL(clicked()),this,SLOT(refresh()));
    connect(ui->listWidget,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(openTexture(QListWidgetItem*)));
    connect(ui->moveAll, SIGNAL(clicked()), this, SLOT(moveAllToUnused()) );
    ui->moveAll->setEnabled(false);
}

void AskUnrefTextureDialog::refresh(){
  accept();
}

AskUnrefTextureDialog::~AskUnrefTextureDialog()
{
    delete ui;
}

void AskUnrefTextureDialog::moveAllToUnused(){
    // find unused "unused" path
    QDir d;
    QString final;
    int n = 0;
    d = QDir(texturePath);
    do {
        final = QString("_unused");
        if (n>0) final += QString("_%1").arg(n);
        n++;
        if (!d.exists(final)) break;
    } while (1);

    int reply = QMessageBox::question(this, tr("OpenBRF"),
                                      tr("Move %1 textures to folder\n%2 ?").arg(ui->listWidget->count()).arg(d.canonicalPath()+ '/' +final),
      QMessageBox::Yes | QMessageBox::No
    );
    if (reply == QMessageBox::Yes) {
      int res = false;
      int done = 0;
      QDir e = d;
      if (d.mkdir(final)) {
          if (e.cd(final)) {
              for (int i=0; i<ui->listWidget->count(); i++) {
                  QString fn = ui->listWidget->item(i)->text();
                  if (d.rename(fn, final+'/'+fn)) done++;
              }
              res = true;
          }

      }
      if (res) {
        res = QMessageBox::question(
          this,tr("OpenBRF"),tr("Moved %1 textures in new folder\n%2").arg(done).arg(e.canonicalPath()),
          tr("open folder"), tr("ok"),QString(),1);
        if (res==0) {
          QStringList args;
          args << QString("/select,") << QDir::toNativeSeparators(e.canonicalPath());
          QProcess::startDetached("explorer", args);
        }
        accept();
      }
      else
        QMessageBox::warning(this,tr("OpenBRF"),tr("Error creating folder %1 in\n%2").arg(final).arg(e.canonicalPath()));

    }

}

void AskUnrefTextureDialog::openTexture(QListWidgetItem *i){
    QStringList args;
    args << QString("/select,") << QDir::toNativeSeparators(texturePath+'/'+i->text());
    QProcess::startDetached("explorer", args);
}

void AskUnrefTextureDialog::addFile(QString f){
  ui->listWidget->addItem(f);
  ui->moveAll->setEnabled(true);

}

void AskUnrefTextureDialog::changeEvent(QEvent *e)
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
