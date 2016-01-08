/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#include "askModErrorDialog.h"
#include "tmp/ui_askModErrorDialog.h"

#include "brfData.h"
#include "iniData.h"

#include <QPushButton>

void AskModErrorDialog::refresh(){
  iniDataReady=false;
  firstPaintDone=false;
  inidata->updated=0;
  setup();
}

void AskModErrorDialog::moreErrors(){
  maxErr+=10;
  performErrorSearch();
}

void AskModErrorDialog::getIniDataReady(){
  inidata->loadAll(4);

  iniDataReady=true;

  if (!isSearch) {
    performErrorSearch();
  } else {
    performSearch();
  }
}

void AskModErrorDialog::setup(){
  if (isSearch){
    m_ui->label->setText(tr("Look for:"));

    m_ui->checkBox->blockSignals(true);
    m_ui->checkBox->setChecked(searchCommonRes);
    m_ui->checkBox->blockSignals(false);

    m_ui->comboBox->blockSignals(true);
    m_ui->comboBox->addItem(tr("Any kind"));
    for (int i=0; i<N_TOKEN; i++) m_ui->comboBox->addItem(QString("%1").arg(IniData::tokenFullName(i)));
    m_ui->comboBox->setCurrentIndex(searchToken+1);
    m_ui->comboBox->blockSignals(false);

    m_ui->lineEdit->blockSignals(true);
    m_ui->lineEdit->setText(searchString);
    m_ui->lineEdit->blockSignals(false);
  }
  else{
    m_ui->label->setText(tr("Searching for errors..."));
  }
  m_ui->textBrowser->setText(tr("<i>scanning data...</i>"));
  i = j = kind = -1;
}

void AskModErrorDialog::setOptions(bool b, int i, QString st){
  searchToken=i;
  searchCommonRes=b;
  searchString=st;
}

void AskModErrorDialog::getOptions(bool *b, int *i,QString *st){
  //searchString = m_ui->lineEdit->text();
  //searchCommonRes = m_ui->checkBox->isChecked();
  //searchToken = m_ui->comboBox->currentIndex()-1;
  *i=searchToken;
  *b=searchCommonRes;
  *st=QString("%1").arg(searchString);
}


void AskModErrorDialog::linkClicked(const QUrl&l){
  QString s = l.toString();
  sscanf(s.toAscii().data(), "#%d.%d.%d",&i, &j, &kind);
  accept();
}

/*QString AskModErrorDialog::searchString(){
  return m_ui->lineEdit->text();
}*/

void AskModErrorDialog::performErrorSearch(){

    bool more = inidata->findErrors(maxErr);
    te->setText(inidata->errorList.join("<p>"));
    int ne=inidata->errorList.size();
    if (!ne) {
      m_ui->label->setText(tr("Found 0 errors in module!"));
    }
    else m_ui->label->setText(tr("Found %n%1 error:", "", ne).arg((more)?"+":""));

    m_ui->buttonBox->buttons()[1]->setEnabled(more);
}

void AskModErrorDialog::performSearch(){
  if (!iniDataReady) return;
  searchString = m_ui->lineEdit->text();
  searchCommonRes = m_ui->checkBox->isChecked();
  searchToken = m_ui->comboBox->currentIndex()-1;
  if ((searchString.length()>=3)
      ||(searchToken==ANIMATION)||(searchToken==SKELETON)||(searchToken==SHADER)) {
    m_ui->textBrowser->setText(inidata->searchAllNames(searchString,searchCommonRes,searchToken));
  } else {
    m_ui->textBrowser->setText(tr("<i>[ready]</i>"));
  }
}


AskModErrorDialog::AskModErrorDialog(QWidget *parent, IniData &i,bool search, QString searchSt) :
    QDialog(parent),
    m_ui(new Ui::askModErrorDialog)
{
  isSearch = search;

  m_ui->setupUi(this);
  inidata = &i;
  te = m_ui->textBrowser;
  te->setReadOnly(true);


  if (!search) {
    QPushButton *b = new QPushButton(tr("More errors"));
    b->setEnabled(false);
    m_ui->buttonBox->addButton(b,QDialogButtonBox::ActionRole);
    connect(b,SIGNAL(clicked()),this,SLOT(moreErrors()));
  }

  QPushButton *b = new QPushButton(tr("Refresh"));
  m_ui->buttonBox->addButton(b,QDialogButtonBox::ActionRole);
  connect(b,SIGNAL(clicked()),this,SLOT(refresh()));
  connect(te,SIGNAL( anchorClicked(QUrl)),
          this, SLOT(linkClicked(QUrl)) );


  m_ui->lineEdit->setVisible(isSearch);
  m_ui->searchOpt->setVisible(isSearch);
  m_ui->lineEdit->setText(searchSt);

  searchCommonRes = true;
  searchToken = -1;
  maxErr = 10;

  setWindowTitle(tr("OpenBrf -- Module %1").arg(i.name()));
  if (search) {

    resize(510, 250);
    connect(m_ui->lineEdit,SIGNAL(textEdited(QString)),this,SLOT(performSearch()));
    connect(m_ui->comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(performSearch()));
    connect(m_ui->checkBox,SIGNAL(clicked()),this,SLOT(performSearch()));
  }
  else {
    resize(710, 250);
  }
  iniDataReady=firstPaintDone=false;
  //update(); // an extra redraw command

}

void AskModErrorDialog::paintEvent ( QPaintEvent * event ){
  QWidget::paintEvent(event);
  if (firstPaintDone) {
    if (!iniDataReady) getIniDataReady();
  }
  if (!firstPaintDone) update();
  firstPaintDone=true;
}

AskModErrorDialog::~AskModErrorDialog()
{
  delete m_ui;
}

void AskModErrorDialog::changeEvent(QEvent *e)
{
  QDialog::changeEvent(e);
  switch (e->type()) {
  case QEvent::LanguageChange:
      m_ui->retranslateUi(this);
      break;
  case QEvent::ActivationChange:
  default:
      break;
  }
}
