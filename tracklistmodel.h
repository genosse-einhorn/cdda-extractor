// Copyright © 2020 Jonas Kümmerlin <jonas@kuemmerlin.eu>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef TRACKLISTMODEL_H
#define TRACKLISTMODEL_H

#include <vector>
#include <QAbstractTableModel>
#include <QImage>

#include "libcdda/toc.h"

class TrackListModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Columns {
        COLUMN_TRACKNO = 0,
        COLUMN_TITLE,
        COLUMN_ARTIST,
        COLUMN_COMPOSER,
        COLUMN_LENGTH,

        COLUMN_FIRST = COLUMN_TRACKNO,
        COLUMN_LAST = COLUMN_LENGTH
    };

    explicit TrackListModel(QObject *parent = 0);

    // Header:
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Editable:
    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Management
    void reset(const std::vector<cdda::toc_track> &tracks);
    void clear();

    void setDevice(const QString &device);
    QString device() const { return m_device; }

    void setAlbumArtist(const QString &artist);
    void setAlbumComposer(const QString &composer);
    void setAlbumTitle(const QString &title);
    void setAlbumGenre(const QString &genre);
    void setAlbumYear(const QString &year);
    void setAlbumDiscNo(const QString &no);
    void setAlbumCover(const QImage &image);
    void setAlbumCatalogNo(const QString &no);

    QString albumArtist() const { return m_albumArtist.size() ? m_albumArtist : tr("Unknown Artist"); }
    QString albumComposer() const { return m_albumComposer.size() ? m_albumComposer : tr("Unknown Composer"); }
    QString albumTitle() const { return m_albumTitle.size() ? m_albumTitle : tr("Unknown Album"); }
    QString albumGenre() const { return m_albumGenre.size() ? m_albumGenre : tr("Unknown Genre"); }
    QString albumYear() const { return m_albumYear; }
    QString albumDiscNo() const { return m_albumDiscNo; }
    QString albumCatalogNo() const { return m_albumCatalogNo; }

    int trackCount() const { return (int)m_data.size(); }
    int trackNo(int i) const { return m_data[i].trackno; }
    bool trackSelected(int i) const { return m_data[i].selected; }
    cdda::block_addr trackBegin(int i) const { return m_data[i].tocdata.start; }
    cdda::block_addr_delta trackLength(int i) const { return m_data[i].tocdata.length; }
    QString trackISRC(int i) const;
    QString trackTitle(int i) const;
    QString trackArtist(int i) const;
    QString trackComposer(int i) const;
    cdda::track_metadata trackMetadata(int i) const;

    int trackIndexForTrackno(int trackno) const;

    void setTrackTitle(int i, const QString &title);
    void setTrackArtist(int i, const QString &artist);
    void setTrackComposer(int i, const QString &composer);

    QString musicbrainzDiscId() const;
    cdda::toc toc() const;

private:

    struct trackdata {
        bool selected;
        int trackno;
        QString title;
        QString artist;
        QString composer;
        QString isrc;

        cdda::toc_track tocdata;
    };

    std::vector<struct trackdata> m_data;

    QString m_device;

    QString m_albumArtist;
    QString m_albumComposer;
    QString m_albumTitle;
    QString m_albumGenre;
    QString m_albumYear;
    QString m_albumDiscNo;
    QString m_albumCatalogNo;

    QImage m_albumCover;
    QByteArray m_albumCoverPng;
};

#endif // TRACKLISTMODEL_H
