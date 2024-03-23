#ifndef IMAGEDRAWERBORDERABSTRACT_H
#define IMAGEDRAWERBORDERABSTRACT_H

#include "ImageDrawerAbstract.h"

class ImageDrawerBorderAbstract : public ImageDrawerAbstract
{
public:
    ImageDrawerBorderAbstract();
    ~ImageDrawerBorderAbstract() override;
    void draw(QImage &image) const override;
    QString nameFile() const override;
    QString name() const override;
    virtual QString colorName() const = 0;
    virtual QColor color() const = 0;
};

#endif // IMAGEDRAWERBORDERABSTRACT_H
