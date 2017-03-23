/***************************************************************************
    Private classes for the bookmark handler
                             -------------------
    begin                : Sun Mar 20 2011
    copyright            : (C) 2011-2016 by Alexander Reinholdt
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
 *   Free Software Foundation, 51 Franklin Street, Suite 500, Boston,      *
 *   MA 02110-1335, USA                                                    *
 ***************************************************************************/

#ifndef SMB4KBOOKMARKHANDLER_P_H
#define SMB4KBOOKMARKHANDLER_P_H

// application specific includes
#include "smb4kbookmarkhandler.h"

// Qt includes
#include <QString>
#include <QUrl>
#include <QTreeWidget>
#include <QDialog>
#include <QListWidget>
#include <QAction>
#include <QPushButton>

// KDE includes
#include <KCompletion/KLineEdit>
#include <KCompletion/KComboBox>
#include <KWidgetsAddons/KActionMenu>


class Q_DECL_EXPORT Smb4KBookmarkDialog : public QDialog
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
    Smb4KBookmarkDialog(const QList<Smb4KBookmark *> &bookmarks,
                        const QStringList &groups,
                        QWidget *parent);

   /**
    * The destructor
    */
   ~Smb4KBookmarkDialog();
   
   /**
    * Returns the list of bookmarks including all changes that could
    * be made in the bookmark dialog.
    * 
    * @returns the list of bookmarks.
    */
   const QList<Smb4KBookmark *> &bookmarks();

  protected Q_SLOTS:
    /**
     * Called when a bookmark was clicked in the list widget.
     */
    void slotBookmarkClicked(QListWidgetItem *bookmark_item);

    /**
     * Called when the label is edited by the user
     */
    void slotLabelEdited();

    /**
     * Called when the group is edited by the user
     */
    void slotGroupEdited();

    /**
     * Called when the OK button was clicked
     */
    void slotDialogAccepted();

    /**
     * Called when the icon size changed
     */
    void slotIconSizeChanged(int group);
    
  private:
    /**
     * Sets up the view
     */
    void setupView();
    
    /**
     * Load the list of bookmarks and the one of the groups
     */
    void loadLists(const QList<Smb4KBookmark *> &bookmarks, const QStringList &groups);

    /**
     * Finds the bookmark in the list
     */
    Smb4KBookmark *findBookmark(const QUrl &url);
    
    /**
     * Ok push button
     */
    QPushButton *m_ok_button;
    
    /**
     * Cancel push button
     */
    QPushButton *m_cancel_button;

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
    QListWidget *m_widget;

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


class Q_DECL_EXPORT Smb4KBookmarkEditor : public QDialog
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
    explicit Smb4KBookmarkEditor(const QList<Smb4KBookmark *> &bookmarks,
                                 QWidget *parent = 0);

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
    QList<Smb4KBookmark *> editedBookmarks() const;

  protected:
    /**
     * Reimplemented from QObject
     */
    bool eventFilter(QObject *obj,
                     QEvent *e);

  protected Q_SLOTS:
    /**
     * Called when a bookmark was clicked
     */
    void slotItemClicked(QTreeWidgetItem *item, int col);

    /**
     * Called when the context menu was requested
     */
    void slotContextMenuRequested(const QPoint &pos);
    
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
    void slotAddGroupTriggered(bool checked);

    /**
     * Called when the delete action was triggered
     */
    void slotDeleteTriggered(bool checked);

    /**
     * Called when the clear action was triggered
     */
    void slotClearTriggered(bool checked);
    
    /**
     * Called when the Ok button was clicked
     */
    void slotDialogAccepted();
    
    /**
     * Called when the icon size changed
     */
    void slotIconSizeChanged(int group);
    
    void slotAdjust();

  private:
    /**
     * Set up the view
     */
    void setupView();

    /**
     * Load bookmarks
     */
    void loadBookmarks(const QList<Smb4KBookmark *> &bookmarks);

    /**
     * Finds the bookmark in the list
     */
    Smb4KBookmark *findBookmark(const QUrl &url);
    
    /**
     * Ok push button
     */
    QPushButton *m_ok_button;
    
    /**
     * Cancel push button
     */
    QPushButton *m_cancel_button;

    
    /**
     * List of the bookmarks that are being processed
     */
    QList<Smb4KBookmark *> m_bookmarks;

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
    QAction *m_add_group;

    /**
     * Delete action
     */
    QAction *m_delete;

    /**
     * Clear action
     */
    QAction *m_clear;
};


class Smb4KBookmarkHandlerPrivate
{
  public:
    Smb4KBookmarkEditor *editor;
    QList<Smb4KBookmark *> bookmarks;
    QStringList groups;
};


class Smb4KBookmarkHandlerStatic
{
  public:
    Smb4KBookmarkHandler instance;
};

#endif
