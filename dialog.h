#ifndef DIALOG_H
#define DIALOG_H

#include "ui_dialog.h"
#include <QDialog>

struct Vectors3D{
    QVector<double> x, y, z;
};

struct Point3D{
    double x, y, z;
};

struct Func3D{
    QString i, j, k;
};

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private:
    bool readInput(double & t, double & dt, double & qm, Point3D & M0, Point3D & V0, Func3D & B, Func3D & E);
    void printPlot(QCustomPlot * plot, QVector<double> val, QVector<double> t);
    void printPlots(QVector<double> & Mx, QVector<double> & My, QVector<double> & Mz,
                    QVector<double> & Vx, QVector<double> & Vy, QVector<double> & Vz,
                    QVector<double> & ax, QVector<double> & ay, QVector<double> & az,
                    QVector<double> & t);

private slots:
    void on_pushButton_clicked();

    void on_radioButton_q_m_clicked();

    void on_radioButton_qm_clicked();

private:
    Ui::Dialog *ui;
};

#endif // DIALOG_H
