#include "infodialog.h"
#include "ui_infodialog.h"

#include <QMessageBox>
#include <QPixmap>
InfoDialog::InfoDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoDialog)
{
    ui->setupUi(this);
    setWindowTitle("Info-Dialog");

    ui->pixlabel->setPixmap(QPixmap(":/Images/FImageCompare_72.png"));

    connect(ui->closeButton, &QPushButton::clicked, this, &InfoDialog::close);
    connect(ui->aboutqtButton, &QPushButton::clicked, this, &InfoDialog::aboutqtButtonClicked);
}

InfoDialog::~InfoDialog()
{
    delete ui;
}

void InfoDialog::aboutqtButtonClicked()
{
    QMessageBox::aboutQt(this);

}
