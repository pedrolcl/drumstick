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

#ifndef CMDLINEARGS_H
#define CMDLINEARGS_H

#include <QtCore/QString>
#include <QtCore/QUrl>
#include <QtCore/QList>
#include <QtCore/QVariant>
#include <QtCore/QStringList>

/*
 * http://www.gnu.org/software/libc/manual/html_node/Argument-Syntax.html
 *
 * Program Argument Syntax Conventions
 *
 * POSIX recommends these conventions for command line arguments. getopt
 * (see Getopt) and argp_parse (see Argp) make it easy to implement them.
 * Arguments are options if they begin with a hyphen delimiter (‘-’).
 * Multiple options may follow a hyphen delimiter in a single token if the
 * options do not take arguments. Thus, ‘-abc’ is equivalent to ‘-a -b -c’.
 * Option names are single alphanumeric characters (as for isalnum; see
 * Classification of Characters).
 * Certain options require an argument. For example, the ‘-o’ command of the ld
 * command requires an argument—an output file name.
 * An option and its argument may or may not appear as separate tokens.
 * (In other words, the whitespace separating them is optional.) Thus, ‘-o foo’
 * and ‘-ofoo’ are equivalent.
 * Options typically precede other non-option arguments.
 *
 * The implementations of getopt and argp_parse in the GNU C library normally
 * make it appear as if all the option arguments were specified before all the
 * non-option arguments for the purposes of parsing, even if the user of your
 * program intermixed option and non-option arguments. They do this by
 * reordering the elements of the argv array. This behavior is nonstandard;
 * if you want to suppress it, define the _POSIX_OPTION_ORDER environment
 * variable. See Standard Environment.
 * The argument ‘--’ terminates all options; any following arguments are
 * treated as non-option arguments, even if they begin with a hyphen.
 * A token consisting of a single hyphen character is interpreted as an ordinary
 * non-option argument. By convention, it is used to specify input from or
 * output to the standard input and output streams.
 * Options may be supplied in any order, or appear multiple times. The
 * interpretation is left up to the particular application program.
 *
 * GNU adds long options to these conventions. Long options consist of ‘--’
 * followed by a name made of alphanumeric characters and dashes. Option names
 * are typically one to three words long, with hyphens to separate words. Users
 * can abbreviate the option names as long as the abbreviations are unique.
 *
 * To specify an argument for a long option, write ‘--name=value’. This syntax
 * enables a long option to accept an argument that is itself optional.
 */

class CmdLineArgs {
public:
    CmdLineArgs();
    virtual ~CmdLineArgs();
    void setProgram(const QString& prog);
    void setUsage(const QString& cmdline);
    void setStdQtArgs(bool qtargs);
    void parse(const QStringList& args);
    void parse(int argc, char* argv[]);

    QString helpText(const QString& id = QString());
    QString programName() const;
    QString programVersion() const;
    void clear();

    void addOption(const QChar& shrt, const QString& id,
            const QString& desc, const QVariant& def=QVariant());
    void addRequiredArgument(const QString& id, const QString& desc);
    void addOptionalArgument(const QString& id, const QString& desc);
    void addMultipleArgument(const QString& id, const QString& desc);
    QVariant getOption(const QString& id);
    QVariant getArgument(const QString& id);
    QVariantList getArguments(const QString& id);
    QStringList getArgumentIds();

private:
    class Private;
    Private *d;
};


#endif /* CMDLINEARGS_H */
