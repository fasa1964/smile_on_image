#include "comparethread.h"

#include <QImageReader>
#include <QMapIterator>

/// \brief Constructor of CompareThread
CompareThread::CompareThread()
{
    stopped = false;
    endpos = 0;
    startpos = 0;

    // Test Approximation
    equalpix = 0;
    imagepixels = 0;
    notequalpix = 0;
    setFast(false);

}

/// \brief Start the process for
/// comparing images
void CompareThread::compare()
{
    if(imageMap().isEmpty()){
        emit error( objectName() + ":" + tr("No image to compare!"));
        stopped = true;
    }

    // Status for update start and end position
    // just do it once
    bool done = false;


    QMapIterator <int, QString> it(imageMap());
    while (it.hasNext())
    {
        it.next();

        if(!done){
            while (it.key() != startpos) {
                it.next();
            }
            done = true;
        }

        // Stop when abort button was clicked
        if(stopped)
            break;

        QImage srcImage = getImage(it.key());
        QString src_name = imageMap().value(it.key()).split("/").last();

        for (int i = 0; i < imageMap().size(); i++)
        {

            // Stop when abort button was clicked
            if(stopped)
                break;

            QString dest_name = imageMap().value(i).split("/").last();

            if(it.key() != i /*&& src_name != dest_name*/ )
            {
                QImage destImage = getImage(i);

                emit compareFile(src_name, dest_name);

                // Test Approximation
                equalpix = 0;
                notequalpix = 0;

                if(srcImage.width() <= destImage.width())
                    imagepixels = srcImage.width() * srcImage.height();
                else
                    imagepixels = destImage.width() * destImage.height();

                if(compareImage(srcImage, destImage))
                {
                    // when image is identical
                    emit identicalOccured( imageMap().value(it.key()) ,imageMap().value(i));
                }
            }
        }

        emit processGrowth(it.key());

        if(it.key() >= endpos)
            break;
    }

    stop();
}

/// \brief Calls when thread has to stop
void CompareThread::stop()
{
    stopped = true;
}

/// \brief Retuns the image map
QMap<int, QString> CompareThread::imageMap() const
{
    return m_imageMap;
}

/// \brief Set the image map
void CompareThread::setImageMap(const QMap<int, QString> &imageMap)
{
    m_imageMap = imageMap;
}

void CompareThread::run()
{
    while (!stopped) {
        compare();
    }
}

/// \brief Load image from index of imageMap
/// \returns QImage
QImage CompareThread::getImage(int index)
{
    QString filename = imageMap().value(index);
    QImageReader reader( filename );

    QImage image;
    if(reader.canRead()){
        image = reader.read();

    }else{
        emit error( "ImageReader: " + objectName() + "\n"  + reader.errorString());
        stopped = true;
    }

    return image;
}


/// \brief Compare the images s & d
/// \returns true if equal or approach
bool CompareThread::compareImage(const QImage &s, const QImage &d)
{
    bool status = true;

     // First check the image depth
     if(s.depth() != d.depth())
         return false;

     if(getFast())
     {
         int y = 0;
         int x = 0;
         while (y != s.height()) {

             while (x != s.width()) {

                if(s.pixel(x,y) != d.pixel(x,y))
                    return false;
                x++;
             }

             y++;
             x =0;
         }

         return status;
     }

     // Test Approximation

     int ysize = 0;
     int xsize = 0;
     int y = 0;
     int x = 0;

     if(s.width() > d.width())
         xsize = d.width();
     else
         xsize = s.width();

     if(s.height() > d.height())
         ysize = d.height();
     else
         ysize = s.height();

     // compare the pixels
     while (y != ysize)
     {
         if(stopped)
             break;

         while (x != xsize )
         {

             if(s.pixel(x,y) != d.pixel(x,y))
             {

                 notequalpix++;
             }
             else
             {
                 equalpix++;
             }


             if(percentOf(equalpix+notequalpix) >= getApproximationValue() ) // When 5% of pixel was proved
             {
                 if(notequalpix > equalpix)
                     return false;

             }
             x++;
         }

         y++;
         x = 0;
     }

     return  status;
}


int CompareThread::getApproximationValue() const
{
    return m_approximationValue;
}

void CompareThread::setApproximationValue(int approximationValue)
{
    m_approximationValue = approximationValue;

    if(approximationValue == 0)
        setFast(true);
    else
        setFast(false);
}

bool CompareThread::getFast() const
{
    return m_fast;
}

void CompareThread::setFast(bool fast)
{
    m_fast = fast;
}

qreal CompareThread::percentOf(int ipix)
{
    qreal per = ipix * 100 / imagepixels ;
    return per;
}

void CompareThread::setRange(int start, int end)
{
    startpos = start;
    endpos = end;
}

void CompareThread::incomingCF(const QString &srcname, const QString &destname)
{
    emit compareFile(srcname, destname);
}

