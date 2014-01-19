/***************************************************************************
    smb4kcustomoptionspage  -  The configuration page for the custom 
    options
                             -------------------
    begin                : Sa Jan 19 2013
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

#ifndef SMB4KCUSTOMOPTIONSPAGE_H
#define SMB4KCUSTOMOPTIONSPAGE_H

// Qt includes
#include <QWidget>
#include <QCheckBox>
#include <QEvent>
#include <QGroupBox>

// KDE includes
#include <klistwidget.h>
#include <kactionmenu.h>
#include <kactioncollection.h>
#include <klineedit.h>
#include <knuminput.h>
#include <kcombobox.h>
#include <ktabwidget.h>

// forward declarations
class Smb4KCustomOptions;

/**
 * This configuration page contains the custom options
 * 
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 1.1.0
 */

class Smb4KCustomOptionsPage : public QWidget
{
  Q_OBJECT
  
  public:
    /**
     * Constructor
     */
    explicit Smb4KCustomOptionsPage( QWidget *parent = 0 );
    
    /**
     * Destructor
     */
    virtual ~Smb4KCustomOptionsPage();
    
    /**
     * This enumeration marks the type of the custom options item
     * in the list widget.
     */
    enum ItemType{ Host,
                   Share };

    /**
     * This function inserts a list of custom option items into the list widget.
     *
     * @param list              The list with Smb4KSambaOptions objects
     */
    void insertCustomOptions( const QList<Smb4KCustomOptions *> &list );

    /**
     * This function returns the list of custom option items that are currently
     * in the list widget.
     *
     * @returns the list of custom option items.
     */
    const QList<Smb4KCustomOptions *> getCustomOptions();
    
    /**
     * Returns TRUE if there may be changed custom settings. You must check if
     * this is indeed the case in you code.
     * 
     * @returns TRUE if custom settings may have changed.
     */
    bool customSettingsMaybeChanged() { return m_maybe_changed; }
    
  protected:
    /**
     * Reimplemented from QObject
     */
    bool eventFilter( QObject *obj, QEvent *e );
    
  Q_SIGNALS:
    /**
     * This signal is emitted every time the custom settings potentially were 
     * modified by the user. When this signal is emitted, it does not necessarily 
     * mean that any custom setting changed. It only means that the user edited 
     * one option.
     */
    void customSettingsModified();
    
    /**
     * This signal is emitted when the settings should be reloaded.
     */
    void reloadCustomSettings();

  protected Q_SLOTS:
    /**
     * This slot is invoked when an item is double clicked. It is used
     * to edit the item the user double clicked.
     *
     * @param item            The item that was double clicked.
     */
    void slotEditCustomItem( QListWidgetItem *item );
    
    /**
     * This slot is invoked when the selection in the custom list widget
     * changed.
     */
    void slotItemSelectionChanged();
    
    /**
     * This slot is invoked when the custom context menu for the custom
     * options widget is requested.
     *
     * @param pos             The position where the context menu was requested.
     */
    void slotCustomContextMenuRequested( const QPoint &pos );
    
    /**
     * This slot is connected to the "Edit" action found in the context menu. 
     * It is called when this action is triggered.
     * 
     * @param checked         TRUE if the action is checked and FALSE otherwise.
     */
    void slotEditActionTriggered( bool );
    
    /**
     * This slot is connected to the "Remove" action found in the context menu.
     * It is called when this action is triggered.
     *
     * @param checked         TRUE if the action is checked and FALSE otherwise.
     */
    void slotRemoveActionTriggered( bool );
    
    /**
     * This slot is connected to the "Clear List" action found in the context
     * menu. It is called when this action is triggered.
     * 
     * @param checked         TRUE if the action is checked and FALSE otherwise.
     */
    void slotClearActionTriggered( bool );
    
    /**
     * This slot is connected to the "Undo" action found in the context
     * menu. It is called when this action is triggered.
     * 
     * @param checked         TRUE if the action is checked and FALSE otherwise.
     */
    void slotUndoActionTriggered( bool );
    
    /**
     * This slot is called when a value was changed.
     */
    void slotEntryChanged();
    
    /**
     * Enable the options for sending Wake-On-LAN magic packages, if the MAC
     * address was entered correctly.
     */
    void slotEnableWOLFeatures( const QString &mac_address );
    
  private:
    enum Tabs { SambaTab = 0, WolTab = 1 };
    void clearEditors();
    Smb4KCustomOptions *findOptions( const QString &url );
    void populateEditors( Smb4KCustomOptions *options );
    void commitChanges();
    
    KListWidget *m_custom_options;
    KActionMenu *m_menu;
    KActionCollection *m_collection;
    QGroupBox *m_general_editors;
    KTabWidget *m_tab_widget;
    KLineEdit *m_unc_address;
    KLineEdit *m_ip_address;
    KLineEdit *m_mac_address;
    KIntNumInput *m_smb_port;
#ifndef Q_OS_FREEBSD
    KIntNumInput *m_fs_port;
    KComboBox *m_write_access;
    KComboBox *m_security_mode;
#endif
    KComboBox *m_protocol_hint;
    KComboBox *m_user_id;
    KComboBox *m_group_id;
    QCheckBox *m_kerberos;
    QCheckBox *m_send_before_scan;
    QCheckBox *m_send_before_mount;
    QCheckBox *m_remount_share;
    
    bool m_maybe_changed;
    QList<Smb4KCustomOptions *> m_options_list;
    Smb4KCustomOptions *m_current_options;
    bool m_removed;
};

#endif
