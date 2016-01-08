/* OpenBRF -- by marco tarini. Provided under GNU General Public License */

#ifndef ASKTRANSFORMDIALOG_H
#define ASKTRANSFORMDIALOG_H

#include <QtGui/QDialog>


namespace Ui {
    class AskTransformDialog;
}

class QAbstractButton;

class AskTransformDialog : public QDialog {
    Q_OBJECT
public:
    AskTransformDialog(QWidget *parent = 0);
    ~AskTransformDialog();

    float* matrix;
    bool* applyToAll;

    void setApplyToAllLoc(bool *loc);
    void setSensitivityOne(double sens); // sets sensibility of transform when moving a single object
    void setSensitivityAll(double sens); // ... or when moving multiple objects
    void setMultiObj(bool);

    void applyAlignments();
    void setBoundingBox(float* minv, float * maxv);

protected:
    void changeEvent(QEvent *e);
    float bb_min[3], bb_max[3];

public slots:
    int exec();
    void reset();

private:
    Ui::AskTransformDialog *ui;
    double sensitivityOne, sensitivityAll;
private slots:
    void update();

    void updateSensitivity();
    void onAlignmentLX(bool);
    void onAlignmentLY(bool);
    void onAlignmentLZ(bool);
    void onAlignmentMX(bool);
    void onAlignmentMY(bool);
    void onAlignmentMZ(bool);
    void onAlignmentRX(bool);
    void onAlignmentRY(bool);
    void onAlignmentRZ(bool);
    void disableAlignX();
    void disableAlignY();
    void disableAlignZ();
    void onButton(QAbstractButton*);
signals:
    void changed();

};

#endif // ASKTRANSFORMDIALOG_H
