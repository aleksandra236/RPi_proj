#include "dialog.h"
#include "ui_dialog.h"
#include <QMessageBox>
#include <QDate>

Dialog::Dialog(const Vozac &vozac, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
    , blinkStanje(false)
    , trenutniVozac(vozac)
{
    ui->setupUi(this);
    setWindowTitle("Alkotest Vozaca");

    fdADC  = fdADC_global;
    fdRTCC = fdRTCC_global;

    ui->le_ime->setText(vozac.ime);
    ui->le_prezime->setText(vozac.prezime);
    ui->le_tablice->setText(vozac.tablice);
    ui->le_jmbg->setText(vozac.jmbg);
    ui->le_brojlk->setText(vozac.brojLK);

    ui->lbl_slikaSmena->setPixmap(
        QPixmap(":/dnevna.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->lbl_slikaStatus->setPixmap(
        QPixmap(":/nema.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->lbl_lcd->setPixmap(
        QPixmap(":/lcdnema.png").scaled(120, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->lbl_led->setPixmap(
        QPixmap(":/svetli.png").scaled(80, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    ui->lbl_status->setText("Vozac prijavljen. Uradite alkotest.");
    ui->pb_alkohol->setValue(0);

    initGPIO();
    initLCD();
    lcdIspisaj("Dobrodosli", vozac.ime.toLatin1().constData());
    setLED(true);

    blinkTimer = new QTimer(this);
    connect(blinkTimer, &QTimer::timeout, this, &Dialog::blinkLED);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &Dialog::onTimer);
    timer->start(1000);
}

Dialog::~Dialog()
{
    blinkTimer->stop();
    timer->stop();
    if (lcdHandle >= 0) {
        setLED(false);
        lcdClear(lcdHandle);
    }
    delete ui;
}

void Dialog::initGPIO()
{
    pinMode(21, OUTPUT); digitalWrite(21, LOW);
    pinMode(10, OUTPUT); digitalWrite(10, LOW);
    pinMode(11, OUTPUT); digitalWrite(11, LOW);
}

void Dialog::initLCD()
{
    lcdHandle = lcdInit(2, 16, 4,
                        LCD_RS, LCD_EN,
                        LCD_D4, LCD_D5, LCD_D6, LCD_D7,
                        0, 0, 0, 0);
    if (lcdHandle < 0)
        qDebug("LCD inicijalizacija nije uspela!");
}

void Dialog::lcdIspisaj(const QString &red1, const QString &red2)
{
    if (lcdHandle < 0) return;
    lcdClear(lcdHandle);
    lcdPosition(lcdHandle, 0, 0);
    lcdPuts(lcdHandle, red1.toLatin1().constData());
    lcdPosition(lcdHandle, 0, 1);
    lcdPuts(lcdHandle, red2.toLatin1().constData());
}

int Dialog::citajADC(int kanal)
{
    if (fdADC < 0) return 0;
    wiringPiI2CWrite(fdADC, 0x40 | (kanal & 0x03));
    wiringPiI2CRead(fdADC);
    return wiringPiI2CRead(fdADC);
}

void Dialog::updateDatum()
{
    QDate danas = QDate::currentDate();
    ui->le_datum->setText(
        QString("%1.%2.%3.")
        .arg(danas.day(),   2, 10, QChar('0'))
        .arg(danas.month(), 2, 10, QChar('0'))
        .arg(danas.year()));
}

void Dialog::updateSmena(int ldr)
{
    if (ldr > LDR_DAN) {
        ui->lbl_smena->setText("DNEVNA VOZNJA");
        ui->lbl_smena->setStyleSheet(
            "background-color: #ffffc0; color: #806000; "
            "border: 1px solid gray; padding: 2px 6px; font-weight: bold;");
        ui->lbl_slikaSmena->setPixmap(
            QPixmap(":/dnevna.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        ui->lbl_smena->setText("NOCNA VOZNJA");
        ui->lbl_smena->setStyleSheet(
            "background-color: #1c1c3a; color: #9999ff; "
            "border: 1px solid #7777cc; padding: 2px 6px; font-weight: bold;");
        ui->lbl_slikaSmena->setPixmap(
            QPixmap(":/nocna.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void Dialog::setLED(bool ukljucene)
{
    int v = ukljucene ? HIGH : LOW;
    digitalWrite(21, v);
    digitalWrite(10, v);
    digitalWrite(11, v);

    ui->lbl_led->setPixmap(
        QPixmap(ukljucene ? ":/svetli.png" : ":/blink.png").scaled(
            80, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void Dialog::blinkLED()
{
    blinkStanje = !blinkStanje;
    int v = blinkStanje ? HIGH : LOW;
    digitalWrite(21, v);
    digitalWrite(10, v);
    digitalWrite(11, v);

    ui->lbl_led->setPixmap(
        QPixmap(blinkStanje ? ":/blink.png" : ":/svetli.png").scaled(
            80, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void Dialog::onTimer()
{
    updateDatum();
    int ldr = citajADC(LDR_KANAL);
    updateSmena(ldr);
}

void Dialog::on_btn_alkotest_clicked()
{
    QMessageBox::information(this, "Alkotest", "Duvajte u uredjaj...");

    int mq3      = citajADC(MQ3_KANAL);
    int procenat = (mq3 * 100) / 255;
    ui->pb_alkohol->setValue(procenat);

    if (mq3 > ALKOHOL_GRANICA) {
        ui->lbl_status->setText("ALKOHOL DETEKTOVAN");
        ui->lbl_status->setStyleSheet("color: red; font-weight: bold;");
        ui->lbl_slikaStatus->setPixmap(
            QPixmap(":/detektovan.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->lbl_lcd->setPixmap(
            QPixmap(":/lcdima.png").scaled(120, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->pb_alkohol->setStyleSheet("QProgressBar::chunk { background-color: red; }");
        lcdIspisaj("ALKOHOL DETEKT.", "VOZILO ZAKLJ.");
        blinkTimer->start(300);
    } else {
        ui->lbl_status->setText("NEMA ALKOHOLA");
        ui->lbl_status->setStyleSheet("color: green; font-weight: bold;");
        ui->lbl_slikaStatus->setPixmap(
            QPixmap(":/nema.png").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->lbl_lcd->setPixmap(
            QPixmap(":/lcdnema.png").scaled(120, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        ui->pb_alkohol->setStyleSheet("QProgressBar::chunk { background-color: green; }");
        lcdIspisaj("NEMA ALKOHOLA", "VOZILO OTKLJ.");
        blinkTimer->stop();
        setLED(true);
    }
}
