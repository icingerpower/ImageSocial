#ifndef IMAGEDRAWERABSTRACT_H
#define IMAGEDRAWERABSTRACT_H

#include <QImage>

class ImageDrawerAbstract
{
public:
    static QList<const ImageDrawerAbstract *> allImageDrawers();
    static QStringList allImageDrawerNames();
    static const ImageDrawerAbstract * getDrawer(
            const QString &name);
    ImageDrawerAbstract();
    virtual ~ImageDrawerAbstract();
    virtual QString name() const = 0;
    virtual QString nameFile() const = 0;
    virtual void draw(QImage &image) const = 0;
};

#endif // IMAGEDRAWERABSTRACT_H
