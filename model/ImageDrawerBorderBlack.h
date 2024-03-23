#ifndef IMAGEDRAWERBORDERBLACK_H
#define IMAGEDRAWERBORDERBLACK_H

#include "ImageDrawerBorderAbstract.h"

class ImageDrawerBorderBlack
        : public ImageDrawerBorderAbstract
{
public:
    ImageDrawerBorderBlack();
    QString colorName() const override;
    QColor color() const override;
};

#endif // IMAGEDRAWERBORDERBLACK_H
