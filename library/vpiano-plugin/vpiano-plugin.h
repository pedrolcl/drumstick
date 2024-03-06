/*
    Virtual Piano Widget for Qt
    Copyright (C) 2008-2024, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along 
    with this program; If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef VPIANOPLUGIN_H
#define VPIANOPLUGIN_H

#include <QDesignerCustomWidgetInterface>

/**
 * @file vpiano-plugin.h
 * PianoKeybdPlugin class definition (Qt Designer plugin)
 */

/**
 * @class QDesignerCustomWidgetInterface
 * @brief The QDesignerCustomWidgetInterface class enables Qt Designer to access and construct custom widgets.
 * @see https://doc.qt.io/qt-5/qdesignercustomwidgetinterface.html
 */

class PianoKeybdPlugin : public QObject,
                         public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetInterface")
    Q_INTERFACES(QDesignerCustomWidgetInterface)

public:
    explicit PianoKeybdPlugin(QObject *parent = nullptr);

    bool isContainer() const override;
    bool isInitialized() const override;
    QIcon icon() const override;
    QString domXml() const override;
    QString group() const override;
    QString includeFile() const override;
    QString name() const override;
    QString toolTip() const override;
    QString whatsThis() const override;
    QWidget *createWidget(QWidget *parent) override;
    void initialize(QDesignerFormEditorInterface *core) override;

private:
    bool initialized = false;
};

#endif // VPIANOPLUGIN_H
