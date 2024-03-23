#ifndef IMAGEDRAWERBORDERGOLD_H
#define IMAGEDRAWERBORDERGOLD_H

#include "ImageDrawerBorderAbstract.h"

class ImageDrawerBorderGold
        : public ImageDrawerBorderAbstract
{
public:
    ImageDrawerBorderGold();
    QString colorName() const override;
    QColor color() const override;
};

#endif // IMAGEDRAWERBORDERGOLD_H
