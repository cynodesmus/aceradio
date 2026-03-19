/*
 * Copyright Carl Philipp Klemm, cynodesmus 2026
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef CLICKABLESLIDER_H
#define CLICKABLESLIDER_H

#include <QMouseEvent>
#include <QSlider>
#include <QStyleOptionSlider>

class ClickableSlider : public QSlider {
    Q_OBJECT
  public:
    explicit ClickableSlider(QWidget * parent = nullptr);
    explicit ClickableSlider(Qt::Orientation orientation, QWidget * parent = nullptr);

  protected:
    void mousePressEvent(QMouseEvent * event) override;

  private:
    int pixelPosToRangeValue(const QPoint & pos);
};

#endif  // CLICKABLESLIDER_H
