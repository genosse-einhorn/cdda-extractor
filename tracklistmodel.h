#ifndef TRACKLISTMODEL_H
#define TRACKLISTMODEL_H

#include <vector>
#include <QAbstractTableModel>

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

    QString albumArtist() const { return m_albumArtist.size() ? m_albumArtist : tr("Unknown Artist"); }
    QString albumComposer() const { return m_albumComposer.size() ? m_albumComposer : tr("Unknown Composer"); }
    QString albumTitle() const { return m_albumTitle.size() ? m_albumTitle : tr("Unknown Album"); }
    QString albumGenre() const { return m_albumGenre.size() ? m_albumGenre : tr("Unknown Genre"); }
    QString albumYear() const { return m_albumYear; }
    QString albumDiscNo() const { return m_albumDiscNo; }

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
};

#endif // TRACKLISTMODEL_H
