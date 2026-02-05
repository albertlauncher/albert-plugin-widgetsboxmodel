// Copyright (c) 2022-2025 Manuel Schneider

#include "resizingqplaintextedit.h"
#include <QApplication>
#include <QPaintEvent>
#include <QPainter>
#include <QSignalBlocker>
#include <QSyntaxHighlighter>
#include <albert/logging.h>
using namespace Qt::StringLiterals;

ResizingQPlainTextEdit::ResizingQPlainTextEdit(QWidget *parent) :
    QPlainTextEdit(parent)
{
    document()->setDocumentMargin(0); // Default: 4
    connect(document()->documentLayout(), &QAbstractTextDocumentLayout::documentSizeChanged,
            this, &ResizingQPlainTextEdit::updateHeight);
}

void ResizingQPlainTextEdit::updateHeight()
{
    // Looks like QPlainTextEdit ceils the linespacing _before_ multiplying with the number of lines
    auto fm = QFontMetricsF(font());
    setFixedHeight(int(document()->size().height()) * int(ceil(fm.lineSpacing()))
                   + 2 * (int)document()->documentMargin()); // see comment above

    // INFO << "contentsMargins()" << contentsMargins();
    // INFO << "document()->documentLayout()->documentSize().height()" << document()->documentLayout()->documentSize().height();
    // INFO << "document()->documentMargin()" << document()->documentMargin();
    // INFO << "document()->size().height()" << document()->size().height();
    // INFO << "fontMetrics().ascent()" << fontMetrics().ascent();
    // INFO << "fontMetrics().capHeight()" << fontMetrics().capHeight();
    // INFO << "fontMetrics().descent()" << fontMetrics().descent();
    // INFO << "fontMetrics().lineSpacing()" << fontMetrics().lineSpacing();
    // INFO << "fontMetrics().xHeight()" << fontMetrics().xHeight();
    // INFO << "viewport()->contentsMargins()" << viewport()->contentsMargins();
    // INFO << "viewport()->size()" << viewport()->size();
    // INFO << "viewport()->sizeHint()" << viewport()->sizeHint();
    // INFO << "viewportMargins()" << viewportMargins();
}

void ResizingQPlainTextEdit::updateFontMarginFix()
{
    ///
    /// Fix for proper text margins.
    ///
    /// The subjective margins (top, left, bottom) should equal, as such the text should be idented
    /// by the distance of the cap line to the top. But the text edit already has some margin
    /// which is accounted for by subtracting the left bearing of a capital '|' from the margin.
    ///

    auto fm = fontMetrics();
    const auto avg_vertical_margin = int((fm.lineSpacing() - fm.capHeight()) / 2.0 + .5);
    auto margin = avg_vertical_margin - int(fm.boundingRect(u'|').width() / 2. + .5);

    // -1 : Workaround scroll area reserves space for new line
    setViewportMargins(margin, 0, margin, -1);

    // CRIT << "fm.height()" << fm.height();
    // CRIT << "fm.lineSpacing()" << fm.lineSpacing();
    // CRIT << "fm.capHeight()" << fm.capHeight();
    // CRIT << "ls - ch" << fm.lineSpacing() - fm.capHeight();
    // CRIT << "(ls - ch)/2" << (fm.lineSpacing() - fm.capHeight())/2;
    // CRIT << "fm.ascent()" << fm.ascent();
    // CRIT << "fm.descent()" << fm.descent();
    // CRIT << "LB |" << fm.leftBearing(u'|');
    // CRIT << "LB M" << fm.leftBearing(u'M');
}

bool ResizingQPlainTextEdit::event(QEvent *event)
{
    if (event->type() == QEvent::FontChange)
    {
        updateHeight();
        updateFontMarginFix();
    }
    return QPlainTextEdit::event(event);
}
