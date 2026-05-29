#include "logindialog.h"
#include "dialog.h"
#include <QApplication>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h>

int fdADC_global;
int fdRTCC_global;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    wiringPiSetup();

    fdADC_global  = wiringPiI2CSetup(0x48);
    fdRTCC_global = wiringPiI2CSetup(0x51);

    if (fdRTCC_global < 0) {
        fprintf(stderr, "Greska: RTC I2C init neuspesna!\n");
        return 1;
    }

    wiringPiI2CWriteReg8(fdRTCC_global, 0x00, 0x00);

    LoginDialog login;
    if (login.exec() == QDialog::Accepted) {
        Dialog w(login.getVozac());
        w.showMaximized();
        return a.exec();
    }

    return 0;
}
