#include "DialogCreateProductPage.h"
#include "ui_DialogCreateProductPage.h"

DialogCreateProductPage::DialogCreateProductPage(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::DialogCreateProductPage)
{
    ui->setupUi(this);
}

DialogCreateProductPage::~DialogCreateProductPage()
{
    delete ui;
}
