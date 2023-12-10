/*
    MIDI Sequencer C++ library
    Copyright (C) 2006-2023, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef DRUMGRIDMODEL_H
#define DRUMGRIDMODEL_H

#include <QAbstractTableModel>
#include <QStringList>

class DrumGridModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit DrumGridModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    void loadKeyNames();
    void fillSampleData();

    void clearPattern();
    void addPatternData(int key, const QStringList& row);
    void endOfPattern();
    QStringList patternData(int row);
    QString patternKey(int row);
    QString patternHit(int row, int col);
    void updatePatternColumns(int columns);

    static const QString DEFVAL;

public Q_SLOTS:
    void changeCell(const QModelIndex &index);
    void changeCell(const QModelIndex &index, const QString& newValue);

private:
    int m_columns;
    QString m_lastValue;
    QMap<int,QString> m_keyNames;
    QList<QStringList> m_modelData;
    QList<QStringList> m_tempData;
    QList<int> m_keys;
    QList<int> m_tempKeys;
};

#endif /* DRUMGRIDMODEL_H */
