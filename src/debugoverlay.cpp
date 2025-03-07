// Copyright (c) 2024 Manuel Schneider

#include "debugoverlay.h"
#include "albert/logging.h"
#include "primitives.h"
#include <QPainter>
#include <QWidget>
#include <QEvent>
#include <QPaintEvent>

void DebugOverlay::recursiveInstallEventFilter(QWidget *widget)
{
    widget->installEventFilter(this);
    for (auto child : widget->findChildren<QWidget*>())
        recursiveInstallEventFilter(child);
}

bool DebugOverlay::eventFilter(QObject *object, QEvent *event)
{
    if (QEvent::Resize == event->type())
    {
        if (auto *w = dynamic_cast<QWidget*>(object); w)
            INFO << objectName()
                 << "geometry" << w->geometry()
                 << "sizeHint" << w->sizeHint()
                 << "minimumSizeHint" << w->minimumSizeHint()
                 << "minimumSize" << w->minimumSize()
                 << "maximumSize" << w->maximumSize();
    }

    else if (event->type() == QEvent::Paint)
    {
        auto *pe = static_cast<QPaintEvent*>(event);
        CRIT<< "Paint event on " << object << pe->rect();
        if (auto *w = dynamic_cast<QWidget*>(object); w)
            drawOverlay(w);
    }

    return false;
}

void DebugOverlay::drawOverlay(QWidget *widget)
{
    INFO << "Drawing overlay on " << widget << widget->rect();

    QPainter painter(widget);

    // drawCheckerboard(painter, widget->rect(), QColor(255,0,0,16), Qt::transparent, 5);

    drawDebugRect(painter, widget->rect(),
                  QString("%1 rect").arg(widget->objectName()),
                  Qt::red, Qt::gray);

    drawDebugRect(painter, widget->contentsRect(),
                  QString("%1 contentsRect").arg(widget->objectName()),
                  Qt::red, Qt::green);

    drawDebugRect(painter, widget->childrenRect(),
                  QString("%1 childrenRect").arg(widget->objectName()),
                  Qt::red, Qt::blue);
}
