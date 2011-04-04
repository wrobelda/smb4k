/***************************************************************************
    smb4kbookmarkhandler_p  -  Private classes for the bookmark handler
                             -------------------
    begin                : Sun Mar 20 2011
    copyright            : (C) 2011 by Alexander Reinholdt
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
 *   Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,   *
 *   MA  02111-1307 USA                                                    *
 ***************************************************************************/

#ifndef SMB4KBOOKMARKHANDLER_P_H
#define SMB4KBOOKMARKHANDLER_P_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QTreeWidget>
#include <QString>

// KDE includes
#include <kdialog.h>
#include <klistwidget.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <kactionmenu.h>

// application specific includes
#include <smb4kbookmarkhandler.h>


class KDE_EXPORT Smb4KBookmarkDialog : public KDialog
{
  Q_OBJECT

  public:
    /**
     * The constructor
     *
     * @param list            The list of bookmarks that are to be saved
     * 
     * @param groups          The list of available bookmark groups
     *
     * @param parent          The parent widget
     */
    Smb4KBookmarkDialog( const QList<Smb4KBookmark *> &bookmarks,
                         const QStringList &groups,
                         QWidget *parent );

   /**
    * The destructor
    */
   ~Smb4KBookmarkDialog();

  protected slots:
    /**
     * Called when a bookmark was clicked in the list widget.
     */
    void slotBookmarkClicked( QListWidgetItem *bookmark_item );

    /**
     * Called when the label is edited by the user
     */
    void slotLabelEdited();

    /**
     * Called when the group is edited by the user
     */
    void slotGroupEdited();

    /**
     * Called when a button was clicked
     */
    void slotUserClickedButton( KDialog::ButtonCode code );

    /**
     * Called when the icon size changed
     */
    void slotIconSizeChanged( int group );
    
  private:
    /**
     * Sets up the view
     */
    void setupView();

    /**
     * Load the list of bookmarks and the one of the groups
     */
    void loadLists();

    /**
     * Finds the bookmark in the list
     */
    Smb4KBookmark *findBookmark( const QUrl &url );

    /**
     * The list of bookmarks
     */
    QList<Smb4KBookmark *> m_bookmarks;

    /**
     * The list of groups
     */
    QStringList m_groups;

    /**
     * The tree widget for the potential bookmarks
     */
    KListWidget *m_widget;

    /**
     * The widget containing the editors
     */
    QWidget *m_editors;

    /**
     * The label
     */
    KLineEdit *m_label_edit;
    
    /**
     * The groups
     */
    KComboBox *m_group_combo;
};


class KDE_EXPORT Smb4KBookmarkEditor : public KDialog
{
  Q_OBJECT

  public:
    /**
     * The constructor.
     *
     * @param bookmarks   The list of all bookmarks
     *
     * @param parent      The parent of this dialog.
     */
    Smb4KBookmarkEditor( const QList<Smb4KBookmark *> &bookmarks,
                         QWidget *parent = 0 );

    /**
     * The destructor.
     */
    ~Smb4KBookmarkEditor();

    /**
     * Returns the list of bookmarks after the editing. Please note
     * that the editor operates on an interal copy of the bookmarks
     * passed to the constructor to prevent accidental modification.
     *
     * @returns the list of bookmarks
     */
    QList<Smb4KBookmark *> editedBookmarks();

  protected:
    /**
     * Reimplemented from QObject
     */
    bool eventFilter( QObject *obj,
                      QEvent *e );

  protected slots:
    /**
     * Called when a bookmark was clicked
     */
    void slotItemClicked( QTreeWidgetItem *item,
                          int col );

    /**
     * Called when the context menu was requested
     */
    void slotContextMenuRequested( const QPoint &pos );
    
    /**
     * Called when the label is edited by the user
     */
    void slotLabelEdited();

    /**
     * Called when the group is edited by the user
     */
    void slotGroupEdited();

    /**
     * Called when the IP address is edited by the user
     */
    void slotIPEdited();

    /**
     * Called when the login is edited by the user
     */
    void slotLoginEdited();

    /**
     * Called when the add action was triggered
     */
    void slotAddGroupTriggered( bool checked );

    /**
     * Called when the delete action was triggered
     */
    void slotDeleteTriggered( bool checked );

    /**
     * Called when the clear action was triggered
     */
    void slotClearTriggered( bool checked );
    
    /**
     * Called when a button was clicked
     */
    void slotUserClickedButton( KDialog::ButtonCode code );
    
    /**
     * Called when the icon size changed
     */
    void slotIconSizeChanged( int group );

  private:
    /**
     * Set up the view
     */
    void setupView();

    /**
     * Load bookmarks
     */
    void loadBookmarks( const QList<Smb4KBookmark *> &list );

    /**
     * Finds the bookmark in the list
     */
    Smb4KBookmark *findBookmark( const QUrl &url );

    /**
     * List of the bookmarks that are being processed
     */
    QList<Smb4KBookmark> m_bookmarks;

    /**
     * Tree widget
     */
    QTreeWidget *m_tree_widget;

    /**
     * The widget containing the editors
     */
    QWidget *m_editors;

    /**
     * The label
     */
    KLineEdit *m_label_edit;

    /**
     * The IP address
     */
    KLineEdit *m_ip_edit;

    /**
     * The login
     */
    KLineEdit *m_login_edit;

    /**
     * The groups
     */
    KComboBox *m_group_combo;

    /**
     * The list of groups
     */
    QStringList m_groups;

    /**
     * Action menu
     */
    KActionMenu *m_menu;

    /**
     * Add group action
     */
    KAction *m_add_group;

    /**
     * Delete action
     */
    KAction *m_delete;

    /**
     * Clear action
     */
    KAction *m_clear;

    /**
     * The item that's dragged around
     */
    QTreeWidgetItem *m_drag_item;
};


class Smb4KBookmarkHandlerPrivate
{
  public:
    Smb4KBookmarkHandlerPrivate();
    ~Smb4KBookmarkHandlerPrivate();
    Smb4KBookmarkHandler instance;
};

#endif