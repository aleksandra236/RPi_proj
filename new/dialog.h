#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QTimer>
#include "logindialog.h"
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <lcd.h>

namespace Ui { class Dialog; }

extern int fdADC_global;
extern int fdRTCC_global;

#define LCD_RS   3
#define LCD_EN   14
#define LCD_D4   4
#define LCD_D5   12
#define LCD_D6   13
#define LCD_D7   6

#define MQ3_KANAL        0
#define LDR_KANAL        3
#define ALKOHOL_GRANICA  120
#define LDR_DAN          100


class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(const Vozac &vozac, QWidget *parent = nullptr);
    ~Dialog();

private slots:
    void onTimer();
    void blinkLED();
    void on_btn_alkotest_clicked();

private:
    Ui::Dialog *ui;
    QTimer *timer;
    QTimer *blinkTimer;

    int fdADC;
    int fdRTCC;
    int lcdHandle;
    bool blinkStanje;
    Vozac trenutniVozac;

    void initGPIO();
    void initLCD();

    int  citajADC(int kanal);
    void updateSmena(int ldr);
    void updateDatum();
    void setLED(bool ukljucene);
    void lcdIspisaj(const QString &red1, const QString &red2);
};

#endif
