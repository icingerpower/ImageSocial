#ifndef DIALOGREPLACE_H
#define DIALOGREPLACE_H

#include <QDialog>

namespace Ui {
class DialogReplace;
}

class DialogReplace : public QDialog
{
    Q_OBJECT

public:
    explicit DialogReplace(QWidget *parent = nullptr);
    explicit DialogReplace(const QString &before, QWidget *parent = nullptr);
    ~DialogReplace();
    bool wasAccepted() const;
    void accept() override;
    void reject() override;
    QString getBeforeText() const;
    QString getAfterText() const;

private:
    Ui::DialogReplace *ui;
    bool m_wasAccepted;
};

#endif // DIALOGREPLACE_H
