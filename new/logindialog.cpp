#include "logindialog.h"
#include "ui_logindialog.h"
#include <QMessageBox>

static const QList<Vozac> baza = {
    {"001", "Marko",  "Nikolic",  "BG 123-AB", "0101990710123", "009876543"},
    {"002", "Petar",  "Petrovic", "NS 456-CD", "1502985720456", "001234567"},
    {"003", "Ana",    "Anic",     "NI 789-EF", "2203991730789", "005556789"},
};

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    setWindowTitle("Alkotest — Prijava");
    setFixedSize(300, 150);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

bool LoginDialog::provjeriID(const QString &id, Vozac &v)
{
    for (const Vozac &vozac : baza) {
        if (vozac.id == id) {
            v = vozac;
            return true;
        }
    }
    return false;
}

void LoginDialog::on_btn_prijava_clicked()
{
    QString id = ui->le_id->text().trimmed();
    if (id.isEmpty()) {
        QMessageBox::warning(this, "Greska", "Unesite ID!");
        return;
    }
    Vozac v;
    if (provjeriID(id, v)) {
        prijavljeniVozac = v;
        accept();
    } else {
        QMessageBox::warning(this, "Greska", "Nepostojeci ID!");
        ui->le_id->clear();
        ui->le_id->setFocus();
    }
}
