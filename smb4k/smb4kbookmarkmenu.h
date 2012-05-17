/***************************************************************************
    smb4kbookmarkmenu  -  Bookmark menu
                             -------------------
    begin                : Sat Apr 02 2011
    copyright            : (C) 2011-2012 by Alexander Reinholdt
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

#ifndef SMB4KBOOKMARKMENU_H
#define SMB4KBOOKMARKMENU_H

// Qt includes
#include <QAction>
#include <QActionGroup>

// KDE includes
#include <kactionmenu.h>
#include <kactioncollection.h>
#include <kshortcut.h>

// forward declarations
class Smb4KBookmark;
class Smb4KShare;


class Smb4KBookmarkMenu : public KActionMenu
{
  Q_OBJECT

  public:
    /**
     * Enumeration
     */
    enum Type { MainWindow,
                SystemTray };
    
    /**
     * Constructor
     */
    explicit Smb4KBookmarkMenu( int type,
                                QWidget *parentWidget = 0,
                                QObject *parent = 0 );

    /**
     * Destructor
     */
    ~Smb4KBookmarkMenu();

    /**
     * Returns the pointer to the "Add Bookmark" action or NULL, if
     * it is not present.
     *
     * @returns the pointer to the "Add Bookmark" action.
     */
    QAction *addBookmarkAction();

    /**
     * Force the menu to be set up again. This should be called if 
     * the settings changed and the handling of bookmarks might be
     * affected.
     */
    void refreshMenu();

  protected slots:
    /**
     * Called when a bookmark has been triggered
     */
    void slotActionTriggered( QAction *action );

    /**
     * Called when the list bookmarks has been updated
     */
    void slotBookmarksUpdated();
    
    /**
     * Called when a bookmark was mounted
     */
    void slotDisableBookmark( Smb4KShare *share );

    /**
     * Called when a bookmark was unmounted
     */
    void slotEnableBookmark( Smb4KShare *share );

  private:
    /**
     * Set up the menu
     */
    void setupMenu( bool setup_all = true );
    
    /**
     * Type
     */
    int m_type;

    /**
     * Widget that should be used as parent
     */
    QWidget *m_parent_widget;
    
    /**
     * The bookmarks
     */
    KActionCollection *m_action_collection;

    /**
     * The bookmark groups
     */
    QActionGroup *m_groups;

    /**
     * The bookmarks
     */
    QActionGroup *m_bookmarks;
};

#endif
