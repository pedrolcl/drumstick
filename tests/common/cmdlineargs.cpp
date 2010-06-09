/*
    Command line arguments parser
    Copyright (C) 2010, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
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

#include "cmdlineargs.h"
#include "cmdversion.h"
#include <QtCore/QFileInfo>
#include <QtCore/QTextStream>
#include <QtCore/QIODevice>
#include <QtCore/QDebug>

CmdLineArgs::CmdLineArgs() :
    m_stdQtArgs(false)
{
    addOption( QLatin1Char('h'), QLatin1String("help"),
               QLatin1String("Print this help message") );
    addOption( QLatin1Char('V'), QLatin1String("version"),
               QLatin1String("Print the program version number") );
}

CmdLineArgs::~CmdLineArgs()
{
    clear();
}

QStringList CmdLineArgs::getArgumentIds()
{
    return m_arguments.keys();
}

CmdLineArgs::CmdLineOption
CmdLineArgs::option(const QString & id)
{
    foreach(const CmdLineOption& opt, m_options) {
        if (opt.m_id == id)
            return opt;
    }
    return CmdLineOption();
}

CmdLineArgs::CmdLineOption
CmdLineArgs::option(const QChar & shrt)
{
    foreach(const CmdLineOption& opt, m_options) {
        if (opt.m_shrt == shrt)
            return opt;
    }
    return CmdLineOption();
}

void CmdLineArgs::parse(const QStringList& args)
{
    bool helpneeded(false);
    QTextStream cerr(stderr, QIODevice::WriteOnly);
    QStringList errs;
    QStringList rems;
    foreach(const CmdLineOption& opt, m_options) {
        if (opt.m_shrt.isNull())
            rems += opt.m_id;
    }
    for (int i=0; i<args.count(); ++i) {
        CmdLineOption opt;
        QString s = args.at(i);
        if (s.isNull())
            continue;
        if (s.startsWith("--")) {
            QStringList lst = s.mid(2).split('=');
            opt = option(lst.first());
            if (opt.m_id.isNull()) {
                errs += s;
            } else {
                if (lst.count() > 1)
                    m_arguments.insertMulti(opt.m_id, lst.at(1));
                else
                    m_arguments.insertMulti(opt.m_id, QVariant(true));
            }
        } else if (s.startsWith('-')) {
            for(int j=1; j<s.length(); ++j) {
                opt = option(s[j]);
                if (opt.m_id.isNull()) {
                    errs += s;
                    break;
                } else {
                    if (opt.m_def.isValid()) {
                        QString arg = s.mid(j+1).trimmed();
                        if (arg.isNull())
                            if ( i<args.count()-1) {
                                i++;
                                arg = args.at(i);
                            } else {
                                errs += s;
                            }
                        m_arguments.insertMulti(opt.m_id, arg);
                        break;
                    } else {
                        m_arguments.insertMulti(opt.m_id, QVariant(true));
                    }
                }
            }
        } else if (!rems.isEmpty()) {
            QString id = rems.first();
            opt = option(id);
            m_arguments.insertMulti(id, s);
            if (!opt.m_mult)
                rems.takeFirst();
        } else {
            errs += s;
        }
        helpneeded |= (opt.m_id == QLatin1String("help"));
        if (opt.m_id == QLatin1String("version")) {
            cerr << m_progName << " version: " << PGM_VERSION << endl;
            cerr.flush();
            exit(0);
        }
    }
    if (errs.count() > 0) {
        cerr << "Wrong argument(s): " << errs.join(", ") << endl;
        helpneeded = true;
    }
    foreach(const CmdLineOption& opt, m_options)
        if (opt.m_req && !m_arguments.contains(opt.m_id)) {
            cerr << "Required argument: " << opt.m_id << endl;
            helpneeded = true;
        }
    if (helpneeded) {
        cerr << m_progName << " version: " << PGM_VERSION << endl;
        cerr << helpText() << endl;
        cerr.flush();
        exit(0);
    }
}

void CmdLineArgs::parse(int argc, char *argv[])
{
    if (argc > 0) {
        QStringList lst;
        QFileInfo info(argv[0]);
        m_progName = info.baseName();
        for(int i=1; i<argc; ++i)
            lst.append(QString(argv[i]));
        parse(lst);
    }
}

QVariant CmdLineArgs::getOption(const QString & id)
{
    if (m_arguments.contains(id))
        return m_arguments.values(id).first();
    else
        foreach(const CmdLineOption& opt, m_options) {
            if (opt.m_id == id)
                return opt.m_def;
        }
    return QVariant();
}

void CmdLineArgs::addOption( const QChar& shrt, const QString & id,
                             const QString & desc, const QVariant & def )
{
    CmdLineOption opt;
    opt.m_shrt = shrt;
    opt.m_id = id;
    opt.m_desc = desc;
    opt.m_def = def;
    opt.m_req = false;
    opt.m_mult = false;
    m_options.append(opt);
}

void CmdLineArgs::addRequiredArgument(const QString & id, const QString & desc)
{
    CmdLineOption opt;
    opt.m_id = id;
    opt.m_desc = desc;
    opt.m_req = true;
    opt.m_mult = false;
    m_options.append(opt);
}

void CmdLineArgs::addOptionalArgument(const QString & id, const QString & desc)
{
    CmdLineOption opt;
    opt.m_id = id;
    opt.m_desc = desc;
    opt.m_req = false;
    opt.m_mult = false;
    m_options.append(opt);
}

void CmdLineArgs::addMultipleArgument(const QString & id, const QString & desc)
{
    CmdLineOption opt;
    opt.m_id = id;
    opt.m_desc = desc;
    opt.m_req = false;
    opt.m_mult = true;
    m_options.append(opt);
}

QString CmdLineArgs::helpText(const CmdLineArgs::CmdLineOption& opt)
{
    QString out(QLatin1String("  "));
    if (opt.m_shrt.isNull())
        out += opt.m_id;
    else {
        out += QLatin1Char('-')
            + opt.m_shrt
            + QLatin1String(",--")
            + opt.m_id;
        if (opt.m_def.isValid()) {
            out += "=";
            if (opt.m_def.type() == QVariant::Int)
                out += QLatin1Char('n');
            else
                out += QLatin1Char('x');
        }
    }
    while(out.size() < 20)
        out += QLatin1Char(' ');
    out += opt.m_desc;
    if (!opt.m_def.isNull()) {
        out += QLatin1String(" (default=");
        out += opt.m_def.toString();
        out += QLatin1Char(')');
    }
    out += QLatin1Char('\n');
    return out;
}

QString CmdLineArgs::helpText(const QString & id)
{
    QString out = QString("Usage: %1 %2\noptions:\n")
                    .arg(m_progName)
                    .arg(m_usage);
    foreach(const CmdLineOption& opt, m_options) {
        if (id.isNull()) {
            out += helpText(opt);
        } else {
            if (opt.m_id == id)
                return helpText(opt);
        }
    }
    if (m_stdQtArgs)
        out += QLatin1String("This program also accepts Qt4 standard options\n");
    return out;
}

void CmdLineArgs::clear()
{
    m_options.clear();
    m_arguments.clear();
}

QVariant CmdLineArgs::getArgument(const QString & id)
{
    if (m_arguments.contains(id))
        return m_arguments.values(id).first();
    return QVariant();
}

QVariantList CmdLineArgs::getArguments(const QString & id)
{
    if (m_arguments.contains(id))
        return m_arguments.values(id);
    return QVariantList();
}

QString CmdLineArgs::programVersion() const
{
    return PGM_VERSION;
}
