#pragma once
#include <QString>
#include <QRandomGenerator>
#include <cstdint>

class SongItem {
public:
    QString caption;
    QString lyrics;
    uint64_t uniqueId; // Unique identifier for tracking across playlist changes
    QString file;
    QString vocalLanguage; // Language override for vocal generation (ISO 639 code or empty)

    inline SongItem(const QString &caption = "", const QString &lyrics = "")
        : caption(caption), lyrics(lyrics) {
        // Generate a unique ID using cryptographically secure random number
        uniqueId = QRandomGenerator::global()->generate64();
    }
};
