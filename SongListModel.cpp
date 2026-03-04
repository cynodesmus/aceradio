#include "SongListModel.h"
#include <QTime>
#include <QRandomGenerator>
#include <QDebug>

SongListModel::SongListModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int SongListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return songList.size();
}

QVariant SongListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= songList.size())
        return QVariant();
    
    const SongItem &song = songList[index.row()];
    
    switch (role) {
    case Qt::DisplayRole:
    case CaptionRole:
        return song.caption;
    case LyricsRole:
        return song.lyrics;
    default:
        return QVariant();
    }
}

bool SongListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= songList.size())
        return false;
    
    SongItem &song = songList[index.row()];
    
    switch (role) {
    case CaptionRole:
        song.caption = value.toString();
        break;
    case LyricsRole:
        song.lyrics = value.toString();
        break;
    default:
        return false;
    }
    
    emit dataChanged(index, index, {role});
    return true;
}

Qt::ItemFlags SongListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

void SongListModel::addSong(const SongItem &song)
{
    beginInsertRows(QModelIndex(), songList.size(), songList.size());
    songList.append(song);
    endInsertRows();
}

void SongListModel::removeSong(int index)
{
    if (index >= 0 && index < songList.size()) {
        beginRemoveRows(QModelIndex(), index, index);
        songList.removeAt(index);
        endRemoveRows();
    }
}

SongItem SongListModel::getSong(int index) const
{
    if (index >= 0 && index < songList.size()) {
        return songList[index];
    }
    return SongItem();
}

int SongListModel::findNextIndex(int currentIndex, bool shuffle) const
{
    if (songList.isEmpty())
        return -1;
    
    if (shuffle) {
        // Simple random selection for shuffle mode
        QRandomGenerator generator;
        return generator.bounded(songList.size());
    }
    
    // Sequential playback
    int nextIndex = currentIndex + 1;
    if (nextIndex >= songList.size()) {
        nextIndex = 0; // Loop back to beginning
    }
    
    return nextIndex;
}
