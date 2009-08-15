#ifndef CONNECTIONITEM_H_
#define CONNECTIONITEM_H_

#include <QListWidgetItem>
#include "alsaport.h"

const int ConnectionItemType(QListWidgetItem::UserType + 1);

using namespace aseqmm;

class ConnectionItem : public QListWidgetItem
{
public:
	ConnectionItem( QListWidget *parent = 0 ) : 
	    QListWidgetItem( parent, ConnectionItemType ) {}
	ConnectionItem( const QString & text, QListWidget * parent = 0 ) :
	    QListWidgetItem ( text, parent,  ConnectionItemType ) {}
	virtual ~ConnectionItem() {}

	void setPortInfo(PortInfo subs) { m_portInfo = subs; }
	PortInfo getPortInfo() const { return m_portInfo; }

private:
    PortInfo m_portInfo;
};

#endif /*CONNECTIONITEM_H_*/
