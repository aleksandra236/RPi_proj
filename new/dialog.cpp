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

    // setScaledContents - Qt sam skalira sliku da popuni labelu
    ui->lbl_slikaSmena->setScaledContents(true);
    ui->lbl_slikaStatus->setScaledContents(true);
    ui->lbl_lcd->setScaledContents(true);
    ui->lbl_led->setScaledContents(true);

    ui->lbl_slikaSmena->setPixmap(QPixmap(":/slike/dnevna.png"));
    ui->lbl_slikaStatus->setPixmap(QPixmap(":/slike/nema.png"));
    ui->lbl_lcd->setPixmap(QPixmap(":/slike/lcdnema.png"));
    ui->lbl_led->setPixmap(QPixmap(":/slike/svetle.png"));
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
    pinMode(25, OUTPUT); digitalWrite(25, LOW);
    pinMode(26, OUTPUT); digitalWrite(26, LOW);
    pinMode(27, OUTPUT); digitalWrite(27, LOW);
    pinMode(28, OUTPUT); digitalWrite(28, LOW);
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
    // lbl_smena je naslov - ne diramo ga, uvek pise "Smena"
    // lbl_slikaSmena prikazuje sliku dnevna.png ili nocna.png
    if (ldr < LDR_DAN) {
        ui->lbl_slikaSmena->setPixmap(QPixmap(":/slike/dnevna.png"));
    } else {
        ui->lbl_slikaSmena->setPixmap(QPixmap(":/slike/nocna.png"));
    }
}

void Dialog::setLED(bool ukljucene)
{
    int v = ukljucene ? HIGH : LOW;
    digitalWrite(25, v);
    digitalWrite(26, v);
    digitalWrite(27, v);
    digitalWrite(28, v);
    ui->lbl_led->setPixmap(QPixmap(ukljucene ? ":/slike/svetle.png" : ":/slike/blink.png"));
}

void Dialog::blinkLED()
{
    blinkStanje = !blinkStanje;
    int v = blinkStanje ? HIGH : LOW;
    digitalWrite(25, v);
    digitalWrite(26, v);
    digitalWrite(27, v);
    digitalWrite(28, v);
    ui->lbl_led->setPixmap(QPixmap(blinkStanje ? ":/slike/blink.png" : ":/slike/svetle.png"));
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
    // Ispod granice = 0%, iznad = skalira od bazne vrednosti (94) do 255
    int procenat = 0;
    if (mq3 > ALKOHOL_GRANICA) {
        procenat = ((mq3 - ALKOHOL_GRANICA) * 100) / (255 - ALKOHOL_GRANICA);
        if (procenat == 0) procenat = 1; // minimum 1% kad je detektovan
    }
    ui->pb_alkohol->setValue(procenat);

    if (mq3 > ALKOHOL_GRANICA) {
        ui->pb_alkohol->setValue(procenat);
        ui->pb_alkohol->setStyleSheet("QProgressBar::chunk { background-color: red; }");
        ui->lbl_slikaStatus->setScaledContents(true);
        ui->lbl_slikaStatus->setPixmap(QPixmap(":/slike/detektovan.png"));
        ui->lbl_lcd->setScaledContents(true);
        ui->lbl_lcd->setPixmap(QPixmap(":/slike/lcdima.png"));
        ui->lbl_slikaStatus->repaint();
        ui->lbl_lcd->repaint();
        lcdIspisaj("ALKOHOL DETEKT.", "ZAKLJUCANO");
        blinkTimer->start(300);
    } else {
        ui->pb_alkohol->setValue(0);
        ui->pb_alkohol->setStyleSheet("QProgressBar::chunk { background-color: green; }");
        ui->lbl_slikaStatus->setScaledContents(true);
        ui->lbl_slikaStatus->setPixmap(QPixmap(":/slike/nema.png"));
        ui->lbl_lcd->setScaledContents(true);
        ui->lbl_lcd->setPixmap(QPixmap(":/slike/lcdnema.png"));
        ui->lbl_slikaStatus->repaint();
        ui->lbl_lcd->repaint();
        lcdIspisaj("NEMA ALKOHOLA", "OTKLJUCANO");
        blinkTimer->stop();
        setLED(true);
    }
}
