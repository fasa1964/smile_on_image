#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("FImageCompare");
    app.setApplicationVersion("1.0");


    MainWindow window;

    QPixmap pix(":/Images/FImageCompare_48.png");
    window.setWindowIcon(QIcon(pix));

    window.show();

    return app.exec();
}
