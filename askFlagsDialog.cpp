/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#include "askFlagsDialog.h"
#include "ui_askFlagsDialog.h"

#include<QtGui>
#include<assert.h>

AskFlagsDialog::AskFlagsDialog(QWidget *parent, QString title, unsigned int ones, unsigned int zeros, QString *names) :
    QDialog(parent),
    m_ui(new Ui::AskFlagsDialog)
{


  myStatusBar = 0;
  //assert (l.size()==32);
  //assert (tips.size()==32);
  m_ui->setupUi(this);

  myStatusBar = new QStatusBar;
  this->layout()->addWidget(myStatusBar);
  myStatusBar->showMessage("");

  setWindowTitle(title);

  m_ui->widget->setLayout(new QVBoxLayout(m_ui->widget));
  m_ui->widget_2->setLayout(new QVBoxLayout(m_ui->widget_2));
  m_ui->widget_3->setLayout(new QVBoxLayout(m_ui->widget_3));
  m_ui->widget_4->setLayout(new QVBoxLayout(m_ui->widget_4));

  for (unsigned int i=0; i<32; i++) {
    cb[i]=new QCheckBox(names[i*2], this);
    cb[i]->setStatusTip(names[i*2+1]);
    QWidget *w;
    if (i<8) w=m_ui->widget;
    else if (i<16) w=m_ui->widget_2;
    else if (i<24) w=m_ui->widget_3;
    else w=m_ui->widget_4;

    w->layout()->addWidget(cb[i]);

    unsigned int one = 1;
    bool isOne = ones & (one<<i);
    bool isZero = zeros & (one<<i);
    if (isOne==isZero) {
      cb[i]->setTristate(false);
      cb[i]->setChecked(isOne);
    }
    else {
      cb[i]->setTristate(true);
      cb[i]->setCheckState(Qt::PartiallyChecked);
      //qDebug("%x %x",ones & (one<<i),zeros & (one<<i))
    }
    
    status[i] = _NORMAL;

    if (names[i*2].isEmpty()) {
      cb[i]->setText(tr("unused (?)"));
      cb[i]->setStatusTip(tr("This seems to be unused."));
      QFont f = cb[i]->font();
      f.setItalic(true);
      cb[i]->setFont(f);
      status[i] = _UNUSED;

      //if (!isOne && !isZero) cb[i]->setVisible(false);
    }
    if (names[i*2].startsWith("R:")) {
      QString t = names[i*2];
      t.replace("R:",tr("reserved"));

      cb[i]->setText(t);
      cb[i]->setEnabled(false);
      QFont f = cb[i]->font();
      f.setItalic(true);
      cb[i]->setFont(f);
      status[i] = _RESERVED;
    }
    if (names[i*2].startsWith("B:")) {
      status[i] = _BIT;
    }

  }
  m_ui->widget  ->layout()->addItem(new QSpacerItem(0,0, QSizePolicy::Minimum,QSizePolicy::Expanding));
  m_ui->widget_2->layout()->addItem(new QSpacerItem(0,0, QSizePolicy::Minimum,QSizePolicy::Expanding));
  m_ui->widget_3->layout()->addItem(new QSpacerItem(0,0, QSizePolicy::Minimum,QSizePolicy::Expanding));
  m_ui->widget_4->layout()->addItem(new QSpacerItem(0,0, QSizePolicy::Minimum,QSizePolicy::Expanding));
  
  m_ui->buttonBox->setLocale(QLocale::system());

  m_ui->showAll->setChecked(showingAll);
  connect(m_ui->showAll,SIGNAL(clicked(bool)),this,SLOT(showAll(bool)));


}

void AskFlagsDialog::setBitCombo(QString name, QString tip, int aa, int bb, const char** options, int zeroIndex){
  _setBitCombo(name,tip,aa,bb,NULL,options,zeroIndex);
}

void AskFlagsDialog::setBitCombo(QString name, QString tip, int aa, int bb, int* options, int zeroIndex){
  _setBitCombo(name,tip,aa,bb,options,NULL,zeroIndex);
}

void AskFlagsDialog::_setBitCombo(QString name, QString tip, int aa, int bb, int* optionsInt, const char** optionsChar, int zeroIndex){
  QWidget *g = new QWidget(this);// or QGroupBox(t,this);
  g->setLayout(new QHBoxLayout(g));

  QWidget *w;
  if (aa<8) w=m_ui->widget;
  else if (aa<16) w=m_ui->widget_2;
  else if (aa<24) w=m_ui->widget_3;
  else w=m_ui->widget_4;
  /*if (aa%8==0) {*/
    w->layout()->addWidget(g);
  //}

  QComboBox *b = new QComboBox(this);
  g->layout()->setMargin(0);
  QLabel * label = new QLabel(name+":",this);
  g->layout()->addWidget(label);
  label->setStatusTip(tip);
  g->layout()->addWidget(b);
  for (int j=0; j<(1<<(bb-aa)); j++) {
    if (optionsInt)
      b->addItem(QString("%1").arg(optionsInt[j]));
    else if (optionsChar)
      b->addItem(QString("%1").arg(optionsChar[j]));
    else
      b->addItem(QString("%1").arg(j));
    b->setStatusTip(tip);
  }
  for (int i=aa; i<bb; i++) {
    assert( status[i]==_BIT );
    cb[i]->setText(QString("%1 (bit %2)").arg(name).arg(i-aa+1));
    cb[i]->setStatusTip(tip);

    QFont f = cb[i]->font();
    f.setItalic(true);
    cb[i]->setFont(f);
  }

  bcGB.push_back(g);
  bcCB.push_back(b);
  bcA.push_back(aa);
  bcB.push_back(bb);
  bcZeroIndex.push_back(zeroIndex);

  checkbox2combobox(bcGB.size()-1);
}

void AskFlagsDialog::comboboxes2checkboxes(){
  for (unsigned int i=0; i<bcCB.size(); i++) combobox2checkbox(i);
}

void AskFlagsDialog::checkboxes2comboboxes(){
  for (unsigned int i=0; i<bcCB.size(); i++) checkbox2combobox(i);
}

void AskFlagsDialog::checkbox2combobox(int i){
  int res=0;
  bool undefined = false;
  const int N = 1<<(bcB[i] - bcA[i]);

  for (int b=bcA[i]; b<bcB[i]; b++) {
    switch (cb[b]->checkState()) {
    case Qt::Checked: res+=1<<(b-bcA[i]); break;
    case Qt::PartiallyChecked: undefined = true; break;
    case Qt::Unchecked: break;
    }
  }
  QComboBox* c(bcCB[i]);
  res = (res+bcZeroIndex[i])%N;
  if (!undefined) c->setCurrentIndex(res);
  else  c->setCurrentIndex(-1);
}

void AskFlagsDialog::combobox2checkbox(int i){
  int val = bcCB[i]->currentIndex();
  const int N = 1<<(bcB[i] - bcA[i]);
  val = (val+N-bcZeroIndex[i])%N;

  if (val!=-1) {
    for (int b=bcA[i]; b<bcB[i]; b++) {
      cb[b]->setCheckState( (val&(1<<(b-bcA[i])) )? Qt::Checked : Qt::Unchecked );
    }
  }
}



bool AskFlagsDialog::showingAll = false;

unsigned int AskFlagsDialog::toOne() const{
  unsigned int one = 1, res=0;
  for (int i=0; i<32; i++)
    if (cb[i]->checkState()==Qt::Checked) res|=(one<<i);
  return res;
}
unsigned int AskFlagsDialog::toZero() const{
  unsigned int one = 1, res=0xFFFFFFFF;
  for (int i=0; i<32; i++)
    if (cb[i]->checkState()==Qt::Unchecked) res&=~(one<<i);
  return res;
}

int AskFlagsDialog::exec(){
  showAll(showingAll);

  int res = QDialog::exec();
  if (res==QDialog::Accepted) {
    if (!showingAll) comboboxes2checkboxes();
  }
  return res;
}

AskFlagsDialog::~AskFlagsDialog()
{
  for (int i=0; i<32; i++) if (cb[i]) delete cb[i];
  delete m_ui;
}

void AskFlagsDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

bool AskFlagsDialog::event(QEvent * e)
{
  if ( e->type() == QEvent::StatusTip )
  {
    myStatusBar->showMessage(static_cast<QStatusTipEvent*>(e)->tip());
    return true;
  } // else
    //if (myStatusBar) myStatusBar->showMessage("Ciccia");

  return  QDialog::event(e);
}

void AskFlagsDialog::showAll(bool m)
{
  if (m) comboboxes2checkboxes(); else checkboxes2comboboxes();
  if (m) for (unsigned int i=0; i<bcGB.size(); i++) bcGB[i]->setVisible(false);


  showingAll = m;
  if (m) {
    for (int i=0; i<32; i++) cb[i]->setVisible(true);
    //m_ui->pushButton->setEnabled(false);
  }
  else {
    for (int i=0; i<32; i++) {
      cb[i]->setVisible(
        (status[i]==_NORMAL) ||
        ( (status[i]==_UNUSED) && (cb[i]->checkState()!=Qt::Unchecked) )
      );
    }
  }

  if (!m) for (unsigned int i=0; i<bcGB.size(); i++) bcGB[i]->setVisible(true);

}
