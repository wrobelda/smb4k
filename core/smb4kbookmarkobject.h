/***************************************************************************
    smb4kbookmarkobject -  This class derives from QObject and
    encapsulates a bookmark item. It is for use with QtQuick.
                             -------------------
    begin                : Fr Mai 11 2013
    copyright            : (C) 2013 by Alexander Reinholdt
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

#ifndef SMB4KBOOKMARKOBJECT_H
#define SMB4KBOOKMARKOBJECT_H

// application specific includes
#include "smb4kbookmark.h"

// Qt includes
#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QScopedPointer>
#include <QtGui/QIcon>

// KDE includes
#include <kdemacros.h>
#include <kurl.h>

// forward declarations
class Smb4KBookmarkObjectPrivate;

/**
 * This class derives from QObject and makes the main functions of a 
 * bookmark available. Its main purpose is to be used with QtQuick 
 * and Plasma.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 1.1.0
 */

class KDE_EXPORT Smb4KBookmarkObject : public QObject
{
  Q_OBJECT
  
  friend class Smb4KBookmarkObjectPrivate;
  
  Q_PROPERTY( QString workgroupName READ workgroupName WRITE setWorkgroupName NOTIFY changed )
  Q_PROPERTY( QString unc READ unc CONSTANT )
  Q_PROPERTY( QString label READ label WRITE setLabel NOTIFY changed )
  Q_PROPERTY( QString description READ description CONSTANT )
  Q_PROPERTY( QUrl url READ url WRITE setURL NOTIFY changed )
  Q_PROPERTY( QIcon icon READ icon WRITE setIcon NOTIFY changed )
  Q_PROPERTY( QString groupName READ groupName WRITE setGroupName NOTIFY changed )
  Q_PROPERTY( bool isGroup READ isGroup WRITE setGroup NOTIFY changed )
  
  public:
    /**
     * Constructor
     */
    explicit Smb4KBookmarkObject( Smb4KBookmark *bookmark, QObject *parent = 0 );
    
    /**
     * Constructor for a bookmark group
     */
    explicit Smb4KBookmarkObject( const QString &groupName, QObject *parent = 0 );
    
    /**
     * Empty constructor
     */
    explicit Smb4KBookmarkObject( QObject *parent = 0 );
    
    /**
     * Destructor
     */
    virtual ~Smb4KBookmarkObject();
    
    /**
     * This function returns the workgroup where the bookmarked
     * share is located.
     * 
     * @returns the workgroup name
     */
    QString workgroupName() const;
    
    /**
     * Set the workgroup of the bookmark object.
     * 
     * @param name      The workgroup name
     */
    void setWorkgroupName( const QString &name );
    
    /**
     * This function returns the UNC of the bookmarked share.
     * 
     * @returns the UNC
     */
    QString unc() const;
    
    /**
     * This function returns the optional label of the bookmarked
     * share.
     *
     * @returns the label
     */
    QString label() const;
    
    /**
     * Set the label of the bookmarked share.
     * 
     * @param label     The label
     */
    void setLabel( const QString &label );
    
    /**
     * This function returns the description of this object. In case of a bookmark
     * group, this functions returns the group name like the @see group() function.
     * In case of a bookmark, the return value depends on the choice of the user. 
     * It is either the UNC or the label.
     * 
     * @returns the description of the bookmark or the name of the bookmark group
     */
    QString description() const;
    
    /**
     * This function returns the URL of this bookmark.
     * 
     * @returns the URL
     */
    KUrl url() const;
    
    /**
     * Set the URL of this bookmark.
     * 
     * @param url       The URL
     */
    void setURL( const KUrl &url );
    
    /**
     * This function returns the icon of the bookmark.
     *
     * @returns the icon
     */
    QIcon icon() const;
    
    /**
     * Set the icon of the bookmark.
     * 
     * @param icon      The icon
     */
    void setIcon( const QIcon &icon );
    
    /**
     * This function returns the name of the group the bookmark is
     * in.
     * 
     * @returns the group
     */
    QString groupName() const;
    
    /**
     * Set the name of the group this bookmark is in.
     * 
     * @param name      The group name
     */
    void setGroupName( const QString &name );
    
    /**
     * This function returns TRUE if this object represents a bookmark
     * group.
     * 
     * @returns TRUE if this object is a bookmark group
     */
    bool isGroup() const;
    
    /**
     * For a bookmark group @p group has to be set to TRUE.
     * 
     * @param group     TRUE for a bookmark group and FALSE otherwise
     */
    void setGroup( bool group );
    
  Q_SIGNALS:
    /**
     * This signal is emitted if something changed.
     */
    void changed();
    
  private:
    const QScopedPointer<Smb4KBookmarkObjectPrivate> d;
};

#endif

