#include "ImageDrawerBorderRed.h"

//----------------------------------------
ImageDrawerBorderRed::ImageDrawerBorderRed()
    : ImageDrawerBorderAbstract()
{
}
//----------------------------------------
QString ImageDrawerBorderRed::colorName() const
{
    return "red";
}
//----------------------------------------
QColor ImageDrawerBorderRed::color() const
{
    return Qt::red;
}
//----------------------------------------
