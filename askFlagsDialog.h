/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef ASKFLAGSDIALOG_H
#define ASKFLAGSDIALOG_H

#include <QtGui/QDialog>

class QCheckBox;
class QStatusBar;
class QGroupBox;
class QComboBox;

namespace Ui {
    class AskFlagsDialog;
}

class AskFlagsDialog : public QDialog {
    Q_OBJECT
public:
    AskFlagsDialog(QWidget *parent, QString title, unsigned int ones, unsigned int zeros, QString* names);
    ~AskFlagsDialog();
    unsigned int toOne() const;  // result: flags that must be set to 1
    unsigned int toZero() const;

    void setBitCombo(QString name, QString tip, int a, int b, int*  options, int zeroIndex = 0);
    void setBitCombo(QString name, QString tip, int a, int b, const char** options, int zeroIndex = 0);
protected:
    void changeEvent(QEvent *e);
    void comboboxes2checkboxes();
    void checkboxes2comboboxes();
    void combobox2checkbox(int i);
    void checkbox2combobox(int i);
    bool event(QEvent * e);
    QStatusBar *myStatusBar;
private:
    static bool showingAll;

    Ui::AskFlagsDialog *m_ui;
    QCheckBox* cb[32];
    enum{_NORMAL,_UNUSED,_RESERVED, _BIT} status[32];

    // bit combos: quick and dirty: four parallel vector
    std::vector<QWidget*> bcGB; // group box
    std::vector<QComboBox*> bcCB;
    std::vector<int> bcA; // (A-B): interval
    std::vector<int> bcB;
    std::vector<int> bcZeroIndex;
    void _setBitCombo(QString name, QString tip, int a, int b, int* optionsInt, const char** optionsChars, int zeroIndex);

public slots:
    int exec();
private slots:
    void showAll(bool);
};

#endif // ASKFLAGSDIALOG_H
