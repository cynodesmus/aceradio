/*
 * Copyright Carl Philipp Klemm, cynodesmus 2026
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <cstdint>
#include <QRandomGenerator>
#include <QString>

class SongItem {
  public:
    QString  caption;
    QString  lyrics;
    uint64_t uniqueId;
    QString  file;
    QString  vocalLanguage;
    bool     cotCaption;
    QString  json;

    inline SongItem(const QString & caption = "", const QString & lyrics = "") :
        caption(caption),
        lyrics(lyrics),
        cotCaption(true) {
        // Generate a unique ID using a cryptographically secure random number
        uniqueId = QRandomGenerator::global()->generate64();
    }
};
