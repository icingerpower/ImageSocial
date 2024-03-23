#include "ImageDrawerBorderBlack.h"

//----------------------------------------
ImageDrawerBorderBlack::ImageDrawerBorderBlack()
    : ImageDrawerBorderAbstract()
{
}
//----------------------------------------
QString ImageDrawerBorderBlack::colorName() const
{
    return "black";
}
//----------------------------------------
QColor ImageDrawerBorderBlack::color() const
{
    return QColor(0, 0, 0);
}
//----------------------------------------
