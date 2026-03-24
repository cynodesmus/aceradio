/*
 * Copyright Carl Philipp Klemm, cynodesmus 2026
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <cstdint>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QString>

class SongItem {
  public:
    QString      caption;
    QString      lyrics;
    unsigned int bpm;
    QString      key;
    QString      vocalLanguage;
    bool         cotCaption;

    uint64_t uniqueId;
    QString  file;
    QString  json;
    QString  originalJson;
    QString  enhancedJson;

    SongItem(const QString & caption = "", const QString & lyrics = "");
    SongItem(const QJsonObject & json);

    void store(QJsonObject & json) const;
    void load(const QJsonObject & json);
};
