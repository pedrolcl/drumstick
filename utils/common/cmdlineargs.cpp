/*
    Command line arguments parser
    Copyright (C) 2010-2015, Pedro Lopez-Cabanillas <plcl@users.sf.net>

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

class CmdLineArgs::Private {
public:
    Private(): m_stdQtArgs(false) {}

    struct CmdLineOption {
        QChar m_shrt;
        QString m_id;
        QString m_desc;
        QVariant m_def;
        bool m_req;
        bool m_mult;
    };

    CmdLineOption option(const QString & id);
    CmdLineOption option(const QChar & shrt);
    QString helpText(const CmdLineOption& opt);

    QList<CmdLineOption> m_options;
    QMultiMap<QString, QVariant> m_arguments;
    QString m_progName;
    QString m_usage;
    bool m_stdQtArgs;
};

CmdLineArgs::Private::CmdLineOption
CmdLineArgs::Private::option(const QString & id)
{
    foreach(const CmdLineOption& opt, m_options) {
        if (opt.m_id == id)
            return opt;
    }
    return CmdLineOption();
}

CmdLineArgs::Private::CmdLineOption
CmdLineArgs::Private::option(const QChar & shrt)
{
    foreach(const CmdLineOption& opt, m_options) {
        if (opt.m_shrt == shrt)
            return opt;
    }
    return CmdLineOption();
}

QString
CmdLineArgs::Private::helpText(const CmdLineArgs::Private::CmdLineOption& opt)
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

CmdLineArgs::CmdLineArgs() :
    d(new Private)
{
    addOption( QLatin1Char('h'), QLatin1String("help"),
               QLatin1String("Print this help message") );
    addOption( QLatin1Char('V'), QLatin1String("version"),
               QLatin1String("Print the program version number") );
}

CmdLineArgs::~CmdLineArgs()
{
    clear();
    delete d;
}

QStringList CmdLineArgs::getArgumentIds()
{
    return d->m_arguments.keys();
}

void CmdLineArgs::parse(const QStringList& args)
{
    bool helpneeded(false);
    QTextStream cerr(stderr, QIODevice::WriteOnly);
    QStringList errs;
    QStringList rems;
    foreach(const Private::CmdLineOption& opt, d->m_options) {
        if (opt.m_shrt.isNull())
            rems += opt.m_id;
    }
    for (int i=0; i<args.count(); ++i) {
        Private::CmdLineOption opt;
        QString s = args.at(i);
        if (s.isNull())
            continue;
        if (s.startsWith("--")) {
            QStringList lst = s.mid(2).split('=');
            opt = d->option(lst.first());
            if (opt.m_id.isNull()) {
                errs += s;
            } else {
                if (lst.count() > 1)
                    d->m_arguments.insertMulti(opt.m_id, lst.at(1));
                else
                    d->m_arguments.insertMulti(opt.m_id, QVariant(true));
            }
        } else if (s.startsWith('-')) {
            for(int j=1; j<s.length(); ++j) {
                opt = d->option(s[j]);
                if (opt.m_id.isNull()) {
                    errs += s;
                    break;
                } else {
                    if (opt.m_def.isValid()) {
                        QString arg = s.mid(j+1).trimmed();
                        if (arg.isNull()) {
                            if ( i<args.count()-1) {
                                i++;
                                arg = args.at(i);
                            } else {
                                errs += s;
                            }
                        }
                        d->m_arguments.insertMulti(opt.m_id, arg);
                        break;
                    } else {
                        d->m_arguments.insertMulti(opt.m_id, QVariant(true));
                    }
                }
            }
        } else if (!rems.isEmpty()) {
            QString id = rems.first();
            opt = d->option(id);
            d->m_arguments.insertMulti(id, s);
            if (!opt.m_mult)
                rems.takeFirst();
        } else {
            errs += s;
        }
        helpneeded |= (opt.m_id == QLatin1String("help"));
        if (opt.m_id == QLatin1String("version")) {
            cerr << d->m_progName << " version: " << PGM_VERSION << endl;
            cerr.flush();
            exit(0);
        }
    }
    if (errs.count() > 0) {
        cerr << "Wrong argument(s): " << errs.join(", ") << endl;
        helpneeded = true;
    }
    foreach(const Private::CmdLineOption& opt, d->m_options)
        if (opt.m_req && !d->m_arguments.contains(opt.m_id)) {
            cerr << "Required argument: " << opt.m_id << endl;
            helpneeded = true;
        }
    if (helpneeded) {
        cerr << d->m_progName << " version: " << PGM_VERSION << endl;
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
        d->m_progName = info.baseName();
        for(int i=1; i<argc; ++i)
            lst.append(QString(argv[i]));
        parse(lst);
    }
}

QVariant CmdLineArgs::getOption(const QString & id)
{
    if (d->m_arguments.contains(id))
        return d->m_arguments.values(id).first();
    else
        foreach(const Private::CmdLineOption& opt, d->m_options) {
            if (opt.m_id == id)
                return opt.m_def;
        }
    return QVariant();
}

void CmdLineArgs::addOption( const QChar& shrt, const QString & id,
                             const QString & desc, const QVariant & def )
{
    Private::CmdLineOption opt;
    opt.m_shrt = shrt;
    opt.m_id = id;
    opt.m_desc = desc;
    opt.m_def = def;
    opt.m_req = false;
    opt.m_mult = false;
    d->m_options.append(opt);
}

void CmdLineArgs::addRequiredArgument(const QString & id, const QString & desc)
{
    Private::CmdLineOption opt;
    opt.m_id = id;
    opt.m_desc = desc;
    opt.m_req = true;
    opt.m_mult = false;
    d->m_options.append(opt);
}

void CmdLineArgs::addOptionalArgument(const QString & id, const QString & desc)
{
    Private::CmdLineOption opt;
    opt.m_id = id;
    opt.m_desc = desc;
    opt.m_req = false;
    opt.m_mult = false;
    d->m_options.append(opt);
}

void CmdLineArgs::addMultipleArgument(const QString & id, const QString & desc)
{
    Private::CmdLineOption opt;
    opt.m_id = id;
    opt.m_desc = desc;
    opt.m_req = false;
    opt.m_mult = true;
    d->m_options.append(opt);
}

void CmdLineArgs::clear()
{
    d->m_options.clear();
    d->m_arguments.clear();
}

QVariant CmdLineArgs::getArgument(const QString & id)
{
    if (d->m_arguments.contains(id))
        return d->m_arguments.values(id).first();
    return QVariant();
}

QVariantList CmdLineArgs::getArguments(const QString & id)
{
    if (d->m_arguments.contains(id))
        return d->m_arguments.values(id);
    return QVariantList();
}

QString
CmdLineArgs::helpText(const QString & id)
{
    QString out = QString("Usage: %1 %2\noptions:\n")
                    .arg(d->m_progName)
                    .arg(d->m_usage);
    foreach(const Private::CmdLineOption& opt, d->m_options) {
        if (id.isNull()) {
            out += d->helpText(opt);
        } else {
            if (opt.m_id == id)
                return d->helpText(opt);
        }
    }
    if (d->m_stdQtArgs)
        out += QLatin1String("This program also accepts Qt standard options\n");
    return out;
}

QString CmdLineArgs::programVersion() const
{
    return PGM_VERSION;
}

void CmdLineArgs::setProgram(const QString& prog)
{
    d->m_progName = prog;
}

void CmdLineArgs::setUsage(const QString& cmdline)
{
    d->m_usage = cmdline;
}

void CmdLineArgs::setStdQtArgs(bool qtargs)
{
    d->m_stdQtArgs = qtargs;
}

QString CmdLineArgs::programName() const
{
    return d->m_progName;
}
