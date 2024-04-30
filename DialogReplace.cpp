#include "DialogReplace.h"
#include "ui_DialogReplace.h"

DialogReplace::DialogReplace(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogReplace)
{
    ui->setupUi(this);
    m_wasAccepted = false;
}

DialogReplace::DialogReplace(
    const QString &before, QWidget *parent)
    : DialogReplace(parent)
{
    ui->lineEditBefore->setText(before);
}

DialogReplace::~DialogReplace()
{
    delete ui;
}

bool DialogReplace::wasAccepted() const
{
    return m_wasAccepted;
}

void DialogReplace::accept()
{
    m_wasAccepted = true;
    QDialog::accept();
}

void DialogReplace::reject()
{
    m_wasAccepted = false;
    QDialog::reject();
}

QString DialogReplace::getBeforeText() const
{
    return ui->lineEditBefore->text();
}

QString DialogReplace::getAfterText() const
{
    return ui->lineEditAfter->text();
}
