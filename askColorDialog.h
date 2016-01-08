#ifndef ASKCOLORDIALOG_H
#define ASKCOLORDIALOG_H

#include <QColorDialog>

class QLineEdit;

class AskColorDialog : public QColorDialog
{
	Q_OBJECT
public:
	explicit AskColorDialog(QColor initial, QWidget *parent = 0);
	
	static QColor myGetColor(const QColor &initial, QWidget *parent, const QString &title);


signals:
	
private slots:
	void setHex();
	void updateHex();

	static QString QColor2QString(const QColor &c);
	static QColor  QString2QColor(const QString &s);

private:
	QLineEdit* hexcode;

};

#endif // ASKCOLORDIALOG_H
