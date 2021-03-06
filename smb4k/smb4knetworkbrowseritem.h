/***************************************************************************
    smb4knetworkbrowseritem  -  Smb4K's network browser list item.
                             -------------------
    begin                : Mo Jan 8 2007
    copyright            : (C) 2007-2020 by Alexander Reinholdt
    email                : alexander.reinholdt@kdemail.net
 ***************************************************************************/

/***************************************************************************
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful, but   *
 *   WITHOUT ANY WARRANTY; without even the implied warranty of            *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   General Public License for more details.                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc., 51 Franklin Street, Suite 500, Boston,*
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifndef SMB4KNETWORKBROWSERITEM_H
#define SMB4KNETWORKBROWSERITEM_H

// application specific includes
#include "core/smb4kglobal.h"

// Qt includes
#include <QTreeWidgetItem>
#include <QTreeWidget>

// forward declarations
class Smb4KNetworkBrowser;

/**
 * This class provides the items for the network neighborhood browser
 * of Smb4K.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 */

class Smb4KNetworkBrowserItem : public QTreeWidgetItem
{
  public:
    /**
     * The constructor for toplevel items.
     *
     * @param parent        The parent tree widget
     * @param item          The network item
     */
    Smb4KNetworkBrowserItem(QTreeWidget *parent, const NetworkItemPtr &item);

    /**
     * The constructor for child items.
     *
     * @param parent        The parent tree widget item.
     * @param item          The network item
     */
    Smb4KNetworkBrowserItem(QTreeWidgetItem *parent, const NetworkItemPtr &item);

    /**
     * The destructor.
     */
    virtual ~Smb4KNetworkBrowserItem();

    /**
     * Columns of the item.
     */
    enum Columns{ Network = 0,
                  Type = 1,
                  IP = 2,
                  Comment = 3 };

    /**
     * This function is provided for convenience. It returns a pointer to 
     * the Smb4KWorkgroup object if it is present or NULL if it is not.
     *
     * @returns a pointer to the workgroup item or NULL.
     */
    WorkgroupPtr workgroupItem();

    /**
     * This function is provided for convenience. It returns a pointer to 
     * the Smb4KHost object if it is present or NULL if it is not.
     *
     * @returns a pointer to the host item or NULL.
     */
    HostPtr hostItem();

    /**
     * This function is provided for convenience. It returns a pointer to 
     * the Smb4KShare object if it is present or NULL if it is not.
     *
     * @returns a pointer to the share item or NULL.
     */
    SharePtr shareItem();
    
    /**
     * This function returns the encapsulated network item.
     * 
     * @returns a pointer to the encapsulated Smb4KBasicNetworkItem object
     * or NULL if there is no item defined (this should never happen).
     */
    const NetworkItemPtr &networkItem();
    
    /**
     * This function updates the internal network item.
     */
    void update();
    
  private:
    /**
     * The network item
     */
    NetworkItemPtr m_item;
};

#endif
