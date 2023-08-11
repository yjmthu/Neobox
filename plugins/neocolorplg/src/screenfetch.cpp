#include <screenfetch.hpp>

#include <QGuiApplication>
#include <QPainter>
#include <QPaintEvent>
#include <QScreen>

#include <iostream>

ScreenFetch::ScreenFetch()
  : ColorBack(QGuiApplication::primaryScreen()->grabWindow())
{
  // setWindowFlag(Qt::WindowStaysOnTopHint, true);
  setWindowFlag(Qt::Window, true);
  setMouseTracking(true);
}

ScreenFetch::~ScreenFetch() {
  setMouseTracking(false);
}

