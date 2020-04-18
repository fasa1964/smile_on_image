#ifndef INFODIALOG_H
#define INFODIALOG_H

#include <QDialog>

namespace Ui {
class InfoDialog;
}


/// \brief InfoDialog about this application
/// and about Qt
/// \todo Check license
class InfoDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InfoDialog(QWidget *parent = nullptr);
    ~InfoDialog();

private slots:
    void aboutqtButtonClicked();

private:
    Ui::InfoDialog *ui;
};

#endif // INFODIALOG_H
