/*
    MIDI Sequencer C++ library
    Copyright (C) 2006-2015, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "drumgridmodel.h"
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#else
#include <QtGui>
#endif

const QString DEFVAL("f");

DrumGridModel::DrumGridModel(QObject *parent)
    : QAbstractTableModel(parent),
    m_columns(16),
    m_lastValue(DEFVAL)
{
    m_modelData.clear();
    m_keys.clear();
    loadKeyNames();
}

void DrumGridModel::loadKeyNames()
{
    m_keyNames.clear();
    m_keyNames[46] = QLatin1String("Open HH");
    m_keyNames[42] = QLatin1String("Closed HH");
    m_keyNames[39] = QLatin1String("Hand Claps");
    m_keyNames[38] = QLatin1String("Snare Drum");
    m_keyNames[36] = QLatin1String("Bass Drum");
}

void DrumGridModel::fillSampleData()
{
    beginInsertRows(QModelIndex(), 0, 4);
    m_modelData.insert(0, QString(",p,,,,p,,,,p,,,,p,,").split(","));
    m_modelData.insert(1, QString("f,,,p,f,,,p,f,,,p,f,,,p").split(","));
    m_modelData.insert(2, QString(",,,,,,,,,,,,,4,4,").split(","));
    m_modelData.insert(3, QString(",,,,f,,,,,,,,f,,,").split(","));
    m_modelData.insert(4, QString("f,,,,p,,,,f,,,,p,,,").split(","));
    m_keys.insert(0, 46);
    m_keys.insert(1, 42);
    m_keys.insert(2, 39);
    m_keys.insert(3, 38);
    m_keys.insert(4, 36);
    endInsertRows();
}

int DrumGridModel::rowCount(const QModelIndex & /* parent */) const
{
    return m_modelData.size();
}

int DrumGridModel::columnCount(const QModelIndex & /* parent */) const
{
    return m_columns;
}

QVariant DrumGridModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();
    return m_modelData[index.row()][index.column()].trimmed();
}

QVariant DrumGridModel::headerData(int section ,
                                Qt::Orientation orientation,
                                int role) const
{
    if (role == Qt::DisplayRole) {
        if ( orientation == Qt::Horizontal ) {
            int m = section % 10;
            if (m == 9)
                m = 0;
            else
                m++;
            return QString::number(m);
        } else {
            int k = m_keys.value(section, 0);
            if (m_keyNames.contains(k))
                return m_keyNames[k];
        }
    }
    return QVariant();
}

void DrumGridModel::changeCell(const QModelIndex &index)
{
    QString after, before = m_modelData[index.row()][index.column()];
    if (before.isEmpty())
        after = m_lastValue;
    else
        after = QString();
    changeCell(index, after);
}

void DrumGridModel::changeCell(const QModelIndex &index, const QString& newValue)
{
    m_lastValue = m_modelData[index.row()][index.column()] = newValue;
    emit dataChanged(index, index);
}

QStringList DrumGridModel::patternData(int row)
{
    return m_modelData.value(row, QStringList());
}

QString DrumGridModel::patternKey(int row)
{
    return QString::number(m_keys.value(row, 0));
}

void DrumGridModel::clearPattern()
{
    beginRemoveRows(QModelIndex(), 0, rowCount());
    m_modelData.clear();
    m_keys.clear();
    m_tempData.clear();
    m_tempKeys.clear();
    endRemoveRows();
}

void DrumGridModel::addPatternData(int key, const QStringList& row)
{
    m_tempKeys.prepend(key);
    m_tempData.prepend(row);
}

void DrumGridModel::endOfPattern()
{
    beginInsertRows(QModelIndex(), 0, m_tempData.size());
    m_keys = m_tempKeys;
    m_modelData = m_tempData;
    endInsertRows();
}

void DrumGridModel::updatePatternColumns(int columns)
{
    int diff = m_columns - columns;
    if (diff == 0)
        return;
    if (diff > 0) {
        beginRemoveColumns(QModelIndex(), columns, m_columns-1);
        for(int i=0; i<m_modelData.size(); ++i)
            while( m_modelData[i].size() > columns )
                m_modelData[i].removeLast();
    } else {
        beginInsertColumns(QModelIndex(), columns, columns-diff-1);
        for(int i=0; i<m_modelData.size(); ++i)
            while( m_modelData[i].size() < columns )
                m_modelData[i].append(QString());
    }
    m_columns = columns;
    if (diff > 0)
        endRemoveColumns();
    else
        endInsertColumns();
}

QString DrumGridModel::patternHit(int row, int col)
{
    if ((col < m_columns) && (row < m_modelData.size()))
        return m_modelData[row][col].trimmed();
    return QString();
}
