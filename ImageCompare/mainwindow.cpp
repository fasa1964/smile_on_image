#include "mainwindow.h"
#include "ui_mainwindow.h"


#include <QFileDialog>
#include <QDir>
#include <QDirIterator>
#include <QPalette>
#include <QSettings>

#include <QImage>
#include <QImageReader>
#include <QMapIterator>
#include <QCursor>
#include <QMessageBox>
#include <QPixmap>
#include <QPair>

#include <comparethread.h>
#include <infodialog.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle( QApplication::applicationName() );

    // Set the Image extension into the type box
    QStringList fList = toStringList( QImageReader::supportedImageFormats() );
    ui->formatBox->addItems(fList);

    int core = QThread::idealThreadCount();
    if(core > 4)
        core = 4;
    ui->coreBox->setRange(1, core);

    ui->progress0Bar->hide();
    ui->progress1Bar->hide();
    ui->progress2Bar->hide();
    ui->progress3Bar->hide();
    processStepCounter = 0;



    connect(ui->closeButton, &QPushButton::clicked, this, &MainWindow::closeButtonClicked);
    connect(ui->pathButton, &QToolButton::clicked, this, &MainWindow::pathButtonClicked);
    connect(ui->startButton, &QToolButton::clicked, this, &MainWindow::startButtonClicked);
    connect(ui->deleteButton, &QToolButton::clicked, this, &MainWindow::deleteButtonClicked);
    connect(ui->infoButton, &QToolButton::clicked, this, &MainWindow::infoButtonClicked);

    connect(ui->approximationSlider, &QSlider::valueChanged, this, &MainWindow::approximationSliderChanged);

    connect(ui->pathEdit, &QLineEdit::textChanged, this, &MainWindow::pathEditTextChanged);
    connect(ui->allFormatsCheckBox, &QCheckBox::stateChanged, this, &MainWindow::allFormatsStateChanged);
    connect(ui->subdirectoriesCheckBox, &QCheckBox::stateChanged, this, &MainWindow::subdirectoriesStateChanged);
    connect(ui->formatBox, &QComboBox::currentTextChanged, this, &MainWindow::formatBoxTextChanged);

    connect(ui->listWidget, &QListWidget::itemClicked, this, &MainWindow::itemClicked);

    readSettings();

}

MainWindow::~MainWindow()
{
    delete ui;
}

/// \brief Writes all settings and close
/// the application
/// \todo Check if threads are running
/// and stop them before closing the application
void MainWindow::closeButtonClicked()
{
    writeSettings();
    close();
}

/// \brief Start's the comparing process
void MainWindow::startButtonClicked()
{

    setupFileMap();

    setCursor(Qt::WaitCursor);
    identicalImageList.clear();
    identicalImageMap.clear();

    ui->listWidget->clear();
    ui->identicalImageBox->setValue(identicalImageList.size());

    ui->pixmapLabel->clear();
    ui->pixmapNameLabel->clear();
    ui->identicalPixmapNameLabel->clear();


    ui->abortButton->setEnabled(true);
    ui->progressBar->setRange(0, fileMap.size()-1);
    ui->progressBar->setValue(0);
    ui->coreBox->setEnabled(false);
    ui->startButton->setEnabled(false);
    start.start();
    processStepCounter = 0;

    int coreCount = ui->coreBox->value();
    int maxsize = fileMap.size();
    int steps = maxsize / coreCount;
    for(int i = 0; i < coreCount; i++)
    {
        int min;
        if(i == 0)
            min = 0;
        else{
            min = (steps * i) + 1;
        }

        int max = steps * (i+1);

        if( i == coreCount-1 )
        {
            while (max != maxsize) {
                max++;
            }
        }


        QPair <int, int> p;
        p.first = min;
        p.second = max;
        threadRange.insert(i, p );
    }

    /// \brief Calculate the
    setupThreading();
}

/// \brief Open a DirectoryDialog
/// and set the choosed directory
/// in the pathEdit
void MainWindow::pathButtonClicked()
{
    QString path = ui->pathEdit->text();
    if(path.isEmpty())
        path = "/home";

    QString s = QFileDialog::getExistingDirectory(this, tr("Directory"), path);
    ui->pathEdit->setText(s);
}

/// \brief Delete the selected image
/// from disc.
void MainWindow::deleteButtonClicked()
{
    QListWidgetItem *item = ui->listWidget->currentItem();
    int row = ui->listWidget->currentRow();
    QString imagename = item->text();
    QString filename = identicalImageMap.value(imagename).first;


    QFile file(filename);
    if(file.exists())
    {
        if(!file.remove())
            QMessageBox::warning(this, tr("Removing file"), tr("Removing file failed!"));
        else {
            ui->listWidget->takeItem(row);
            ui->listWidget->update();
            ui->listWidget->sortItems(Qt::AscendingOrder);
            ui->pixmapLabel->clear();
            ui->pixmapLabel->setText(tr("Image has been removed!"));
            ui->deleteButton->setEnabled(false);
            int index = identicalImageList.indexOf(filename);
            identicalImageList.removeAt(index);
            identicalImageMap.remove(imagename);

            ui->identicalImageBox->setValue(identicalImageMap.values().size());
            setupFileMap();
        }
    }
    else {
        QMessageBox::warning(this, tr("Removing file"), tr("Sorry, this file does not exist!"));
    }

    ui->deleteButton->setEnabled(false);
}

/// \brief Opens an Info Dialog
void MainWindow::infoButtonClicked()
{
    InfoDialog *infoDlg = new InfoDialog(this);
    infoDlg->exec();

}

/// \brief Has all supported image format's
void MainWindow::formatBoxTextChanged(const QString &)
{
    setupFileMap();
}

/// \brief Reads and updates the filemap
/// with all supported image format
void MainWindow::allFormatsStateChanged(int)
{
    setupFileMap();
}

/// \brief Reads and updates all supported
/// images with subfolders when checked
void MainWindow::subdirectoriesStateChanged(int)
{
    setupFileMap();
}

/// \brief Change the color depend's on
/// if path exist or not
void MainWindow::pathEditTextChanged(const QString &text)
{
    if(pathExist(text)){
        setTextColor(Qt::blue, ui->pathEdit);
        setupFileMap();
    }else
        setTextColor(Qt::red, ui->pathEdit);
}


/// \brief Loads the image and shows in image label
void MainWindow::itemClicked(QListWidgetItem *item)
{
    // It's the source image
    QString s =  item->text();
    QString filename;

    // Get path of selected image
    filename = identicalImageMap.value(s).first;

    // Load Pixmap and set into Label
    QPixmap pix(filename);
    QPixmap pixmap = pix.scaledToWidth(229, Qt::FastTransformation);
    ui->pixmapLabel->setPixmap(pixmap);
    ui->deleteButton->setEnabled(true);

    // Set Name of source Image
    ui->pixmapNameLabel->setText(s);

    // The selected Image is identical with ...
    QString dest = identicalImageMap.value(s).second.split("/").last();
    ui->identicalPixmapNameLabel->setText(dest);

}

void MainWindow::approximationSliderChanged(int value)
{
    if(value == 0)
         ui->approximationLabel->setText("Fast");
    else{
        QString vs = QString::number(value, 10);
        ui->approximationLabel->setText(vs+"%");
    }
}


/// \brief Signals from CompareThread
/// with the given parameter what is
/// currently comparing
void MainWindow::currentCompare(const QString &s, const QString &d)
{
    ui->statusBar->showMessage(tr("Comparing: ")+ s + " : " +d);
}

/// \brief Signals from CompareThread
/// with the given error string
void MainWindow::threadError(const QString &error)
{
    QMessageBox::warning(this, tr("Thread Error"), tr("Error: ") + error);
    ui->statusBar->showMessage(tr("Process stoped!"));
}

/// \brief Signals from CompareThread
/// when identical image has found.
/// Store the file into a list and update the
/// counts in the image box
/// \param s = sourceImage d = destinationImage
void MainWindow::identcalImageOcuured(const QString &s, const QString &d)
{
    QPair<QString, QString> pair;
    pair.first = s;
    pair.second = d;

    QString filename = s.split("/").last();
    identicalImageMap.insert(filename, pair);

    identicalImageList << s;
    ui->identicalImageBox->setValue(identicalImageList.size());

    ui->listWidget->addItem(filename);
}

/// \brief Signals from CompareThread
/// when thread is finished.
/// Check if still one thread is running
void MainWindow::processFinished()
{

    bool finished = true;
    foreach (CompareThread *thread, threadMap.values())
    {
        if(thread->isRunning()){
            finished = false;
        }
    }

    if(finished){
        setCursor(Qt::ArrowCursor);
        ui->progress0Bar->hide();
        ui->progress1Bar->hide();
        ui->progress2Bar->hide();
        ui->progress3Bar->hide();
        ui->progressBar->setValue(0);
        ui->abortButton->setEnabled(false);
        ui->startButton->setEnabled(true);
        ui->coreBox->setEnabled(true);
        ui->statusBar->showMessage(tr("Process has been finished!"),5000);
        processStepCounter = 0;
        QTime n(0,0,0);
        QTime t = n.addMSecs(start.elapsed());
        ui->timeEdit->setTime(t);
        ui->listWidget->sortItems(Qt::AscendingOrder);

    }
}

/// \brief Shows the process step from thread1
void MainWindow::process0Step(int value)
{
    processStepCounter++;
    ui->progress0Bar->setValue(value);
    mainProcess();
}

/// \brief Shows the process step from thread2
void MainWindow::process1Step(int value)
{
    processStepCounter++;
    ui->progress1Bar->setValue(value);
    mainProcess();
}

/// \brief Shows the process step from thread3
void MainWindow::process2Step(int value)
{
    processStepCounter++;
    ui->progress2Bar->setValue(value);
     mainProcess();
}

/// \brief Shows the process step from thread4
void MainWindow::process3Step(int value)
{
    processStepCounter++;
    ui->progress3Bar->setValue(value);
    mainProcess();
}


/// \brief Shows the total process step
void MainWindow::mainProcess()
{
    ui->progressBar->setValue(processStepCounter);
}


void MainWindow::setupFileMap()
{
    fileMap.clear();

    QString path = ui->pathEdit->text();
    if(path.data()[path.size()-1] != '/')
        path = path + "/";

    QDirIterator::IteratorFlag flag;
    if(ui->subdirectoriesCheckBox->isChecked())
        flag = QDirIterator::Subdirectories;
    else
        flag = QDirIterator::NoIteratorFlags;


    QStringList filters;
    if(ui->allFormatsCheckBox->isChecked())
        filters = toStringList( QImageReader::supportedImageFormats() );
    else
        filters << ui->formatBox->currentText();

    // Convert filters to upper strings
    foreach(QString f, filters){
        QString upper = f.toUpper();
        filters << upper;
    }


    QDirIterator it(path, QDir::Files, flag);
    while (it.hasNext()) {
        QString s = it.next();
        if( filters.contains( s.split(".").last() ) )
            fileMap.insert( fileMap.size() ,s);
    }

    ui->imageCountBox->setValue(fileMap.size());

    if(fileMap.size() > 0)
        ui->startButton->setEnabled(true);
    else
        ui->startButton->setEnabled(false);
}

/// \brief Check if directoty exist
/// \returns true if exist
/// \param path is the pathEdit text
bool MainWindow::pathExist(const QString &path)
{
    QDir dir(path);
    return dir.exists();
}

/// \brief Set the text color in the widget
/// \param color is the text color
/// \param widegt is the widget
void MainWindow::setTextColor(QColor color, QWidget *widget)
{
    QPalette pal = widget->palette();
    pal.setColor(QPalette::Text, color);
    widget->setPalette(pal);
}

QStringList MainWindow::toStringList(QList<QByteArray> list)
{
    QStringList clist;

    foreach (const QByteArray &item, list){
               clist.append(QString::fromLocal8Bit(item));
    }

    return clist;
}

void MainWindow::setupThreading()
{
    int core = ui->coreBox->value();

    CompareThread *comp = new CompareThread();
    CompareThread *comp1 = new CompareThread();
    CompareThread *comp2 = new CompareThread();
    CompareThread *comp3 = new CompareThread();

    threadMap.insert(0, comp);
    threadMap.insert(1, comp1);
    threadMap.insert(2, comp2);
    threadMap.insert(3, comp3);

    for(int i = 0; i < core; i++)
    {

        threadMap.value(i)->setObjectName("comp"+QString::number(i,10));

        int min = threadRange.value(i).first;
        int max = threadRange.value(i).second;

        threadMap.value(i)->setImageMap(fileMap);
        threadMap.value(i)->setRange(min, max);

        threadMap.value(i)->setApproximationValue(ui->approximationSlider->value());

        setupConnecting(threadMap.value(i));

        // setup the progress bar
        if(i == 0){
            ui->progress0Bar->show();
            ui->progress0Bar->setValue(0);
            ui->progress0Bar->setRange(min, max-1);
        }
        if(i == 1){
            ui->progress1Bar->show();
            ui->progress1Bar->setValue(0);
            ui->progress1Bar->setRange(min, max-1);
        }

        if(i == 2){
            ui->progress2Bar->show();
            ui->progress2Bar->setValue(0);
            ui->progress2Bar->setRange(min, max-1);
        }

        if(i == 3){
            ui->progress3Bar->show();
            ui->progress3Bar->setValue(0);
            ui->progress3Bar->setRange(min, max-1);
        }

        threadMap.value(i)->start();

    }

}

void MainWindow::setupConnecting(CompareThread *thread)
{

    connect(thread, &CompareThread::error, this, &MainWindow::threadError, Qt::QueuedConnection);
    connect(thread, &CompareThread::finished, this, &MainWindow::processFinished, Qt::QueuedConnection);
    connect(thread, &CompareThread::identicalOccured, this, &MainWindow::identcalImageOcuured, Qt::QueuedConnection);
    connect(ui->abortButton, &QPushButton::clicked, thread, &CompareThread::stop, Qt::QueuedConnection);

    if(thread == threadMap.value(0))
    {
        connect(thread, &CompareThread::compareFile, this, &MainWindow::currentCompare, Qt::QueuedConnection);

    }

    if(thread != threadMap.value(0))
    {
        connect(thread, &CompareThread::compareFile, threadMap.value(0), &CompareThread::incomingCF, Qt::QueuedConnection);
    }

    if(thread == threadMap.value(0))
    {
        connect(thread, &CompareThread::processGrowth, this, &MainWindow::process0Step,Qt::QueuedConnection);
    }

    if(thread == threadMap.value(1))
    {
        connect(thread, &CompareThread::processGrowth, this, &MainWindow::process1Step,Qt::QueuedConnection);
    }

    if(thread == threadMap.value(2))
    {
        connect(thread, &CompareThread::processGrowth, this, &MainWindow::process2Step,Qt::QueuedConnection);
    }

    if(thread == threadMap.value(3))
    {
        connect(thread, &CompareThread::processGrowth, this, &MainWindow::process3Step,Qt::QueuedConnection);
    }
}

void MainWindow::readSettings()
{
    QSettings setting("ImageCompare", "FasaExample");

    QString sp = setting.value("path", "/home").toString();
    ui->pathEdit->setText(sp);

    bool subd = setting.value("subdirectorie", false).toBool();
    ui->subdirectoriesCheckBox->setChecked(subd);

    bool allFormats = setting.value("allformats", true).toBool();
    ui->allFormatsCheckBox->setChecked(allFormats);

    QString sf = setting.value("imageformat", "jpg").toString();
    ui->formatBox->setCurrentText(sf);

    int av = setting.value("approximation",0).toInt();
    ui->approximationSlider->setValue(av);
    approximationSliderChanged(av);

}

void MainWindow::writeSettings()
{
     QSettings setting("ImageCompare", "FasaExample");
     setting.setValue("path", ui->pathEdit->text());
     setting.setValue("subdirectorie", ui->subdirectoriesCheckBox->isChecked());
     setting.setValue("allformats", ui->allFormatsCheckBox->isChecked());
     setting.setValue("imageformat", ui->formatBox->currentText());
     setting.setValue("approximation", ui->approximationSlider->value());
}





