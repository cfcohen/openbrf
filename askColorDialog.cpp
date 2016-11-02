#include "askColorDialog.h"

#include <QDialog>
#include <QShortcut>
#include <QApplication>
#include <QClipboard>
#include <QLayout>
#include <QLineEdit>

AskColorDialog::AskColorDialog(QColor initial, QWidget *parent) :
	QColorDialog(initial, parent)
{
	hexcode = new QLineEdit( this);
	hexcode->setMaxLength(9);
	QLayout *layout = this->layout();

	// find a nice spot in the layout...
	for (int k=0; k<2; k++) if (layout) {
		for (int i=2; i>=0; i--) {
			QLayoutItem *test = layout->itemAt(i);
			if (test) if (test->layout()) {
				layout = test->layout();
				//qDebug("ok %d (%d)",i,k);
				break;
			}
		}
	}

	if (layout) layout->addWidget(hexcode);

	connect(hexcode,SIGNAL(textEdited(QString)),this,SLOT(setHex()));
	connect(this,SIGNAL(currentColorChanged(QColor)),this,SLOT(updateHex()));


	setOption(ShowAlphaChannel, true);
	updateHex();
	setHex();
}


QColor AskColorDialog::myGetColor(const QColor &initial, QWidget *parent, const QString &title){
	AskColorDialog *dialog = new AskColorDialog(initial,parent);
	dialog->setWindowTitle(title);

	QColor res;


	if (dialog->exec()==QDialog::Rejected) res = QColor::Invalid;
	else res = dialog->currentColor();

	delete dialog;

	return res;
}

QString AskColorDialog::QColor2QString(const QColor &c){
	QChar zero('0');
	QString res = QString("#%1%2%3")
	    .arg(c.red(),2,16,zero)
	    .arg(c.green(),2,16,zero)
	    .arg(c.blue(),2,16,zero);
	if (c.alpha()!=255) {
		res.append( QString("%1")
		  .arg(c.alpha(),2,16,zero)
		);
	}
	return res.toUpper();
}

QColor  AskColorDialog::QString2QColor(const QString &ss){
	QString s(ss);
	if (s.startsWith('#')) s.remove(0,1);
	if ((s.length()!=6) && (s.length()!=8)) return QColor::Invalid;

	QColor res;
	bool ok0, ok1, ok2, ok3 = true;
	res.setRed(  s.mid(0,2).toInt(&ok0,16) );
	res.setGreen(  s.mid(2,2).toInt(&ok1,16) );
	res.setBlue(  s.mid(4,2).toInt(&ok2,16) );
	if (s.length()==8)
		res.setAlpha(  s.mid(6,2).toInt(&ok3,16) );

	if (ok0 && ok1 && ok2 && ok3 ) return res;
	else return QColor::Invalid;
}



void AskColorDialog::setHex(){

	QColor c = QString2QColor( hexcode->text() );

	if (c.isValid()) setCurrentColor( c );

	// set bold if color is valid
	QFont f = hexcode->font();
	f.setBold(c.isValid());
	hexcode->setFont( f );
}

void AskColorDialog::updateHex(){
	QString oldText = hexcode->text();
	if (QString2QColor(oldText)!=currentColor())
	  hexcode->setText( QColor2QString(currentColor()) );
}


