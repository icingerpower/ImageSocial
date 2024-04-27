#ifndef DIALOGCREATEPRODUCTPAGE_H
#define DIALOGCREATEPRODUCTPAGE_H

#include <QDialog>

namespace Ui {
class DialogCreateProductPage;
}

class DialogCreateProductPage : public QDialog
{
    Q_OBJECT

public:
    explicit DialogCreateProductPage(QWidget *parent = nullptr);
    ~DialogCreateProductPage();

private:
    Ui::DialogCreateProductPage *ui;
};

#endif // DIALOGCREATEPRODUCTPAGE_H
