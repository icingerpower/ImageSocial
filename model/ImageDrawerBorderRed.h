#ifndef IMAGEDRAWERBORDERRED_H
#define IMAGEDRAWERBORDERRED_H

#include "ImageDrawerBorderAbstract.h"

class ImageDrawerBorderRed
        : public ImageDrawerBorderAbstract
{
public:
    ImageDrawerBorderRed();
    QString colorName() const override;
    QColor color() const override;
};

#endif // IMAGEDRAWERBORDERRED_H
