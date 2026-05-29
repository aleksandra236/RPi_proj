#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

namespace Ui { class LoginDialog; }

struct Vozac {
    QString id;
    QString ime;
    QString prezime;
    QString tablice;
    QString jmbg;
    QString brojLK;
};

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

    Vozac getVozac() const { return prijavljeniVozac; }

private slots:
    void on_btn_prijava_clicked();

private:
    Ui::LoginDialog *ui;
    Vozac prijavljeniVozac;
    bool provjeriID(const QString &id, Vozac &v);
};

#endif
