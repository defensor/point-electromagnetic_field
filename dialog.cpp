#include "dialog.h"
#include <fstream>
#include "PhysicsF.h"

#define MAX(a,b) (((a)>(b))?(a):(b))

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

Dialog::~Dialog()
{
    delete ui;
}

bool Dialog::readInput(double & t, double & dt, double & qm, Point3D & M0, Point3D & V0, Func3D & B, Func3D & E)
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    bool ok;

    // t & dt
    t = ui->doubleSpinBox_t->value();
    if (t <= 0){
        msgBox.setInformativeText("t must be a positive number!"); msgBox.exec(); return false;
    }
    dt = ui->doubleSpinBox_dt->value();
    if (dt <= 0){
        msgBox.setInformativeText("dt must be a positive number!"); msgBox.exec(); return false;
    }

    if (dt >= t){
        msgBox.setInformativeText("t must be greater than dt!"); msgBox.exec(); return false;
    }

    // q & m
    if (ui->radioButton_q_m->isChecked()){
        double q = QString(ui->lineEdit_q->text()).toDouble(&ok);
        if (!ok){ msgBox.setInformativeText("q must be a number!"); msgBox.exec(); return false;}
        double m = QString(ui->lineEdit_m->text()).toDouble(&ok);
        if (!ok){ msgBox.setInformativeText("m must be a number!"); msgBox.exec(); return false;}
        if (m <= 0){ msgBox.setInformativeText("m must be a positive number!"); msgBox.exec(); return false;}
        qm = q/m;
    }
    else{   // q/m
        qm = QString(ui->lineEdit_qm->text()).toDouble(&ok);
        if (!ok){ msgBox.setInformativeText("q/m not a number!"); msgBox.exec(); return false;}
     }

    // M0
    M0.x = QString(ui->lineEdit_Mx->text()).toDouble(&ok);
    if (!ok){ msgBox.setInformativeText("M0x must be a number!"); msgBox.exec(); return false;}
    M0.y = QString(ui->lineEdit_My->text()).toDouble(&ok);
    if (!ok){ msgBox.setInformativeText("M0y must be a number!"); msgBox.exec(); return false;}
    M0.z = QString(ui->lineEdit_Mz->text()).toDouble(&ok);
    if (!ok){ msgBox.setInformativeText("M0z must be a number!"); msgBox.exec(); return false;}

    // V0
    V0.x = QString(ui->lineEdit_Vx->text()).toDouble(&ok);
    if (!ok){ msgBox.setInformativeText("V0x must be a number!"); msgBox.exec(); return false;}
    V0.y = QString(ui->lineEdit_Vy->text()).toDouble(&ok);
    if (!ok){ msgBox.setInformativeText("V0y must be a number!"); msgBox.exec(); return false;}
    V0.z = QString(ui->lineEdit_Vz->text()).toDouble(&ok);
    if (!ok){ msgBox.setInformativeText("V0z must be a number!"); msgBox.exec(); return false;}

    // B
    B.i = ui->lineEdit_Bi->text().remove(QRegExp("\\s"));
    B.j = ui->lineEdit_Bj->text().remove(QRegExp("\\s"));
    B.k = ui->lineEdit_Bk->text().remove(QRegExp("\\s"));

    // E
    E.i = ui->lineEdit_Ei->text().remove(QRegExp("\\s"));
    E.j = ui->lineEdit_Ej->text().remove(QRegExp("\\s"));
    E.k = ui->lineEdit_Ek->text().remove(QRegExp("\\s"));

    return true;
}

void Dialog::printPlot(QCustomPlot * plot, QVector<double> val, QVector<double> t)
{
    plot->clearGraphs(); // очищаем график
    plot->addGraph(); // Добавляем один график в widget
    plot->graph(0)->setData(t, val); // Задаем данные

    // Установим область, которая будет показываться на графике
    // Для оси Ox

    plot->xAxis->setRange(0, *(t.end()-1));

    // Для оси Oy
    double minY = val[0], maxY = val[0];
    for (int i = 1; i < val.size(); i++)
    {
        if (val[i]<minY) minY = val[i];
        if (val[i]>maxY) maxY = val[i];
    }

    double range = MAX(abs(maxY), abs(minY));
    if(abs(maxY - minY) < 0.05*range){
        double shift = 0.1*range;
        minY -= shift;
        maxY += shift;
    }

    plot->yAxis->setRange(minY, maxY);//Для оси Oy

    //И перерисуем график на нашем widget
    plot->replot();
}

void Dialog::printPlots(QVector<double> & Mx, QVector<double> & My, QVector<double> & Mz,
                        QVector<double> & Vx, QVector<double> & Vy, QVector<double> & Vz,
                        QVector<double> & ax, QVector<double> & ay, QVector<double> & az,
                        QVector<double> & t)
{
    // M_x
    printPlot(ui->plot_x, Mx, t);

    // M_y
    printPlot(ui->plot_y, My, t);

    // M_z
    printPlot(ui->plot_z, Mz, t);

    //////////////////////////////////////////////////////////

    // V_x
    printPlot(ui->plot_Vx, Vx, t);

    // V_y
    printPlot(ui->plot_Vy, Vy, t);

    // V_z
    printPlot(ui->plot_Vz, Vz, t);

    //////////////////////////////////////////////////////////

    // a_x
    printPlot(ui->plot_ax, ax, t);

    // a_y
    printPlot(ui->plot_ay, ay, t);

    // a_z
    printPlot(ui->plot_az, az, t);
}

void Dialog::on_pushButton_clicked()
{
    int res;

    double t, dt, qm;
    Point3D M0, V0;
    Func3D B, E;

    res = readInput(t, dt, qm, M0, V0, B, E);
    if(!res)
        return;

    int N = t/dt + 2; //Вычисляем количество точек, которые будем отрисовывать

    QVector<double> Mx(N), My(N), Mz(N);
    QVector<double> Vx(N), Vy(N), Vz(N);
    QVector<double> ax(N), ay(N), az(N);
    QVector<double> T(N);

    // Начальная инициализация
    Mx[0] = M0.x;
    My[0] = M0.y;
    Mz[0] = M0.z;

    Vx[0] = V0.x;
    Vy[0] = V0.y;
    Vz[0] = V0.z;

    ax[0] = ay[0] = az[0] = 0;

    T[0] = 0;

    // Вызов все этой бадяги
    res = getVals(B.i, B.j, B.k,
            E.i, E.j, E.k,
            dt, N, qm,
            // Возвращаемы значения
            T,
            Mx, My, Mz,
            Vx, Vy, Vz,
            ax, ay, az);
    if (res){
        while (Mx.size() > res)
            Mx.removeLast();
        while (My.size() > res)
            My.removeLast();
        while (Mz.size() > res)
            Mz.removeLast();

        while (Vx.size() > res)
            Vx.removeLast();
        while (Vy.size() > res)
            Vy.removeLast();
        while (Vz.size() > res)
            Vz.removeLast();

        while (ax.size() > res)
            ax.removeLast();
        while (ay.size() > res)
            ay.removeLast();
        while (az.size() > res)
            az.removeLast();

        while (T.size() > res)
            T.removeLast();

        N = res;
    }

//    switch(res){
//    case 1:
//        msgBox.setInformativeText("E is NAN!");
//        msgBox.exec();
//        break;
//    case 2:
//        msgBox.setInformativeText("B is NAN!");
//        msgBox.exec();
//        break;
//    case 3:
//        msgBox.setInformativeText("a is NAN!");
//        msgBox.exec();
//        break;
//    case 4:
//        msgBox.setInformativeText("V is NAN!");
//        msgBox.exec();
//        break;
//    case 5:
//        msgBox.setInformativeText("M is NAN!");
//        msgBox.exec();
//        break;
//    }


    std::ofstream of("table.txt");
    of << "t\tx\ty\tz\tVx\tVy\tVz\tax\tay\taz" << std::endl;
    for (int i = 0; i < N; i++)
        of << T[i] << "\t" << Mx[i] << "\t" << My[i] << "\t" << Mz[i] << "\t" << Vx[i] << "\t" << Vy[i] << "\t" << Vz[i] << "\t" << ax[i] << "\t" << ay[i] << "\t" << az[i] << std::endl;

    printPlots(Mx, My, Mz, Vx, Vy, Vz, ax, ay, az, T);

    of.close();
}

void Dialog::on_radioButton_q_m_clicked()
{
   ui->lineEdit_q->setEnabled(true);
   ui->lineEdit_m->setEnabled(true);

   ui->lineEdit_qm->setDisabled(true);
}

void Dialog::on_radioButton_qm_clicked()
{
    ui->lineEdit_q->setDisabled(true);
    ui->lineEdit_m->setDisabled(true);

    ui->lineEdit_qm->setEnabled(true);
}
