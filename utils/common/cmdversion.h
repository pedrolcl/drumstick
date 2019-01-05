#ifndef CMDVERSION_H
#define CMDVERSION_H

#include <QString>

#define LITERAL(s) #s
#define STRINGIFY(s) LITERAL(s)
const QString PGM_VERSION(STRINGIFY(VERSION));

#endif // CMDVERSION_H
