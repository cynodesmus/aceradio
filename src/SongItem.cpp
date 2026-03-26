// Copyright Carl Philipp Klemm, cynodesmus 2026
// SPDX-License-Identifier: GPL-3.0-or-later

#include "SongItem.h"

SongItem::SongItem(const QString & caption, const QString & lyrics) :
    caption(caption),
    lyrics(lyrics),
    cotCaption(true),
    bpm(0) {
    uniqueId = QRandomGenerator::global()->generate64();
}

SongItem::SongItem(const QJsonObject & json) {
    load(json);
}

void SongItem::store(QJsonObject & json) const {
    if (!caption.isEmpty()) {
        json["caption"] = caption;
    }
    if (!lyrics.isEmpty()) {
        json["lyrics"] = lyrics;
    }
    if (uniqueId != 0) {
        json["unique_id"] = QString::number(uniqueId);
    }
    json["use_cot_caption"] = cotCaption;
    if (!vocalLanguage.isEmpty()) {
        json["vocal_language"] = vocalLanguage;
    }
    if (!key.isEmpty()) {
        json["keyscale"] = key;
    }
    if (bpm != 0) {
        json["bpm"] = static_cast<qlonglong>(bpm);
    }
}

void SongItem::load(const QJsonObject & json) {
    caption = json["caption"].toString();
    lyrics  = json["lyrics"].toString();
    if (json.contains("unique_id")) {
        uniqueId = json["unique_id"].toString().toULongLong();
    }
    cotCaption    = json["use_cot_caption"].toBool(true);
    vocalLanguage = json["vocal_language"].toString();
    key           = json["keyscale"].toString();
    bpm           = json["bpm"].toInt(0);
}
