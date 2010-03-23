/***************************************************************************
    smb4kcustomoptionsdialog  -  With this dialog the user can define
    custom Samba options for hosts or shares.
                             -------------------
    begin                : So Jun 25 2006
    copyright            : (C) 2006-2010 by Alexander Reinholdt
    email                : dustpuppy@users.berlios.de
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

// Qt includes
#include <QGridLayout>
#include <QLabel>
#include <QList>

// KDE includes
#include <klocale.h>
#include <kapplication.h>
#include <klineedit.h>
#include <kdebug.h>
#include <kstandardguiitem.h>
#include <kuser.h>

// application specific includes
#include "smb4kcustomoptionsdialog.h"
#include <core/smb4kglobal.h>
#include <core/smb4kcore.h>
#include <core/smb4ksambaoptionsinfo.h>
#include <core/smb4ksambaoptionshandler.h>
#include <core/smb4ksettings.h>
#include <core/smb4khost.h>
#include <core/smb4kshare.h>
#include <core/smb4khomesshareshandler.h>

using namespace Smb4KGlobal;


Smb4KCustomOptionsDialog::Smb4KCustomOptionsDialog( Smb4KHost *host, QWidget *parent )
: KDialog( parent ), m_type( Host ), m_host( host ), m_share( NULL )
{
  setAttribute( Qt::WA_DeleteOnClose, true );

  setCaption( i18n( "Custom Options" ) );
  setButtons( User1|Ok|Cancel );
  setDefaultButton( Ok );
  setButtonGuiItem( User1, KStandardGuiItem::defaults() );

  m_initialized = true;

  setupDialog();

  setMinimumWidth( sizeHint().width() > 350 ? sizeHint().width() : 350 );

  KConfigGroup group( Smb4KSettings::self()->config(), "CustomOptionsDialog" );
  restoreDialogSize( group );
}


Smb4KCustomOptionsDialog::Smb4KCustomOptionsDialog( Smb4KShare *share, QWidget *parent )
: KDialog( parent ), m_type( Share ), m_host( NULL ), m_share( share )
{
  setAttribute( Qt::WA_DeleteOnClose, true );

  setCaption( i18n( "Custom Options" ) );
  setButtons( User1|Ok|Cancel );
  setDefaultButton( Ok );
  setButtonGuiItem( User1, KStandardGuiItem::defaults() );

  if ( !m_share->isHomesShare() )
  {
    m_initialized = true;
  }
  else
  {
    // We do not use parent as parent for the "Specify User"
    // dialog, so that the behavior is uniform.
    QWidget *p = 0;

    if ( kapp )
    {
      p = kapp->activeWindow();
    }

    m_initialized = Smb4KHomesSharesHandler::self()->specifyUser( m_share, p );
  }

  setupDialog();

  setMinimumSize( (sizeHint().width() > 350 ? sizeHint().width() : 350), sizeHint().height() );

  setInitialSize( QSize( minimumWidth(), minimumHeight() ) );

  KConfigGroup group( Smb4KSettings::self()->config(), "CustomOptionsDialog" );
  restoreDialogSize( group );
}


Smb4KCustomOptionsDialog::~Smb4KCustomOptionsDialog()
{
}


void Smb4KCustomOptionsDialog::setupDialog()
{
  // The Smb4KSambaOptionsInfo object:
  Smb4KSambaOptionsInfo *info = NULL;

  // These are the input widgets we need below:
  m_port_input = NULL;
  m_kerberos = NULL;
  m_proto_input = NULL;
  m_uid_input = NULL;
  m_gid_input = NULL;
#ifndef Q_OS_FREEBSD
  m_rw_input = NULL;
#endif

  // Set up the widget:
  QWidget *main_widget = new QWidget( this );
  setMainWidget( main_widget );

  QGridLayout *layout = new QGridLayout( main_widget );
  layout->setSpacing( 5 );
  layout->setMargin( 0 );

  QLabel *location_label = new QLabel( m_type == Host ? i18n( "Host:" ) : i18n( "Share:" ), main_widget );
  KLineEdit *location    = new KLineEdit( m_type == Host ?
                           m_host->hostName() :
                           m_share->unc(),
                           main_widget );
  location->setReadOnly( true );

  // The widgets will be put into the layout below.

  // Here comes the item-dependent stuff:
  switch ( m_type )
  {
    case Host:
    {
      QLabel *port_label = new QLabel( Smb4KSettings::self()->remoteSMBPortItem()->label(), main_widget );
      m_port_input = new KIntNumInput( -1, main_widget );
      m_port_input->setMinimumWidth( 200 );
      m_port_input->setMinimum( Smb4KSettings::self()->remoteSMBPortItem()->minValue().toInt() );
      m_port_input->setMaximum( Smb4KSettings::self()->remoteSMBPortItem()->maxValue().toInt() );

      QLabel *protocol_label = new QLabel( i18n( "Protocol:" ), main_widget );
      m_proto_input = new KComboBox( false, main_widget );
      m_proto_input->setMinimumWidth( 200 );

      QStringList protocol_items;
      protocol_items.append( i18n( "automatic" ) );
      protocol_items.append( "RPC" );
      protocol_items.append( "RAP" );
      protocol_items.append( "ADS" );

      m_proto_input->insertItems( 0, protocol_items );

      m_kerberos = new QCheckBox( i18n( "Try to authenticate with Kerberos (Active Directory)" ), main_widget );

      layout->addWidget( location_label, 0, 0, 0 );
      layout->addWidget( location, 0, 1, 0 );
      layout->addWidget( port_label, 1, 0, 0 );
      layout->addWidget( m_port_input, 1, 1, 0 );
      layout->addWidget( protocol_label, 2, 0, 0 );
      layout->addWidget( m_proto_input, 2, 1, 0 );
      layout->addWidget( m_kerberos, 3, 0, 1, 2, 0 );

      info = Smb4KSambaOptionsHandler::self()->findItem( m_host );
      
      // Set the port.
      if ( info && info->port() != -1 )
      {
        m_port_input->setValue( info->port() );
      }
      else
      {
        m_port_input->setValue( Smb4KSettings::remoteSMBPort() );
      }
      
      // Set the protocol.
      if ( info && info->protocol() != Smb4KSambaOptionsInfo::UndefinedProtocol )
      {
        switch ( info->protocol() )
        {
          case Smb4KSambaOptionsInfo::Automatic:
          {
            m_proto_input->setCurrentItem( i18n( "automatic" ), false );
            break;
          }
          case Smb4KSambaOptionsInfo::RPC:
          {
            m_proto_input->setCurrentItem( "RPC", false );
            break;
          }
          case Smb4KSambaOptionsInfo::RAP:
          {
            m_proto_input->setCurrentItem( "RAP", false );
            break;
          }
          case Smb4KSambaOptionsInfo::ADS:
          {
            m_proto_input->setCurrentItem( "ADS", false );

            break;
          }
          case Smb4KSambaOptionsInfo::UndefinedProtocol:
          default:
          {
            break;
          }
        }
      }
      else
      {
        switch ( Smb4KSettings::protocolHint() )
        {
          case Smb4KSettings::EnumProtocolHint::Automatic:
          {
            m_proto_input->setCurrentItem( i18n( "automatic" ), false );
            break;
          }
          case Smb4KSettings::EnumProtocolHint::RPC:
          {
            m_proto_input->setCurrentItem( "RPC", false );
            break;
          }
          case Smb4KSettings::EnumProtocolHint::RAP:
          {
            m_proto_input->setCurrentItem( "RAP", false );
            break;
          }
          case Smb4KSettings::EnumProtocolHint::ADS:
          {
            m_proto_input->setCurrentItem( "ADS", false );
            break;
          }
          default:
          {
            break;
          }
        }
      }
      
      // Set Kerberos usage.
      if ( info && info->useKerberos() != Smb4KSambaOptionsInfo::UndefinedKerberos )
      {
        switch ( info->useKerberos() )
        {
          case Smb4KSambaOptionsInfo::UseKerberos:
          {
            m_kerberos->setChecked( true );
            break;
          }
          case Smb4KSambaOptionsInfo::NoKerberos:
          {
            m_kerberos->setChecked( false );
            break;
          }
          case Smb4KSambaOptionsInfo::UndefinedKerberos:
          default:
          {
            break;
          }
        }
      }
      else
      {
        m_kerberos->setChecked( Smb4KSettings::useKerberos() );
      }

      // Connections:
      connect( m_port_input,  SIGNAL( valueChanged( int ) ),
               this,          SLOT( slotPortChanged( int ) ) );

      connect( m_kerberos,    SIGNAL( toggled( bool ) ),
               this,          SLOT( slotKerberosToggled( bool ) ) );

      connect( m_proto_input, SIGNAL( activated( const QString & ) ),
               this,          SLOT( slotProtocolChanged( const QString & ) ) );

      break;
    }
    case Share:
    {
#ifndef Q_OS_FREEBSD
      QLabel *port_label = new QLabel( Smb4KSettings::self()->remoteFileSystemPortItem()->label(), main_widget );
      m_port_input = new KIntNumInput( -1, main_widget );
      m_port_input->setMinimumWidth( 200 );
      m_port_input->setMinimum( Smb4KSettings::self()->remoteFileSystemPortItem()->minValue().toInt() );
      m_port_input->setMaximum( Smb4KSettings::self()->remoteFileSystemPortItem()->maxValue().toInt() );

      QLabel *permission_label = new QLabel( i18n( "Write access:" ), main_widget );
      m_rw_input = new KComboBox( false, main_widget );
      m_rw_input->setMinimumWidth( 200 );

      QStringList write_access_entries;
      write_access_entries.append( i18n( "read-write" ) );
      write_access_entries.append( i18n( "read-only" ) );

      m_rw_input->insertItems( 0, write_access_entries );

      QLabel *uid_label = new QLabel( i18n( "User ID:" ), main_widget );
      m_uid_input = new KComboBox( main_widget );
      m_uid_input->setMinimumWidth( 200 );

      QList<KUser> user_list = KUser::allUsers();
      QStringList uids;

      for ( int i = 0; i < user_list.size(); ++i )
      {
        uids.append( QString( "%1 (%2)" ).arg( user_list.at( i ).loginName() )
                                         .arg( user_list.at( i ).uid() ) );
      }

      uids.sort();

      m_uid_input->addItems( uids );

      QLabel *gid_label = new QLabel( i18n( "Group ID:" ), main_widget );
      m_gid_input = new KComboBox( main_widget );
      m_gid_input->setMinimumWidth( 200 );

      QList<KUserGroup> group_list = KUserGroup::allGroups();
      QStringList gids;

      for ( int i = 0; i < group_list.size(); ++i )
      {
        gids.append( QString( "%1 (%2)" ).arg( group_list.at( i ).name() )
                                         .arg( group_list.at( i ).gid() ) );
      }

      gids.sort();

      m_gid_input->addItems( gids );

      layout->addWidget( location_label, 0, 0, 0 );
      layout->addWidget( location, 0, 1, 0 );
      layout->addWidget( port_label, 1, 0, 0 );
      layout->addWidget( m_port_input, 1, 1, 0 );
      layout->addWidget( uid_label, 2, 0, 0 );
      layout->addWidget( m_uid_input, 2, 1, 0 );
      layout->addWidget( gid_label, 3, 0, 0 );
      layout->addWidget( m_gid_input, 3, 1, 0 );
      layout->addWidget( permission_label, 4, 0, 0 );
      layout->addWidget( m_rw_input, 4, 1, 0 );
#else
      QLabel *port_label = new QLabel( Smb4KSettings::self()->remoteSMBPortItem()->label(), main_widget );
      m_port_input = new KIntNumInput( -1, main_widget );
      m_port_input->setMinimumWidth( 200 );
      m_port_input->setMinimum( Smb4KSettings::self()->remoteSMBPortItem()->minValue().toInt() );
      m_port_input->setMaximum( Smb4KSettings::self()->remoteSMBPortItem()->maxValue().toInt() );

      QLabel *uid_label = new QLabel( i18n( "User ID:" ), main_widget );
      m_uid_input = new KComboBox( main_widget );
      m_uid_input->setMinimumWidth( 200 );

      QList<KUser> user_list = KUser::allUsers();
      QStringList uids;

      for ( int i = 0; i < user_list.size(); ++i )
      {
        uids.append( QString( "%1 (%2)" ).arg( user_list.at( i ).loginName() )
                                         .arg( user_list.at( i ).uid() ) );
      }

      uids.sort();

      m_uid_input->addItems( uids );

      QLabel *gid_label = new QLabel( i18n( "Group ID:" ), main_widget );
      m_gid_input = new KComboBox( main_widget );
      m_gid_input->setMinimumWidth( 200 );

      QList<KUserGroup> group_list = KUserGroup::allGroups();
      QStringList gids;

      for ( int i = 0; i < group_list.size(); ++i )
      {
        gids.append( QString( "%1 (%2)" ).arg( group_list.at( i ).name() )
                                         .arg( group_list.at( i ).gid() ) );
      }

      gids.sort();

      m_gid_input->addItems( gids );

      layout->addWidget( location_label, 0, 0, 0 );
      layout->addWidget( location, 0, 1, 0 );
      layout->addWidget( port_label, 1, 0, 0 );
      layout->addWidget( m_port_input, 1, 1, 0 );
      layout->addWidget( uid_label, 2, 0, 0 );
      layout->addWidget( m_uid_input, 2, 1, 0 );
      layout->addWidget( gid_label, 3, 0, 0 );
      layout->addWidget( m_gid_input, 3, 1, 0 );
#endif

      info = Smb4KSambaOptionsHandler::self()->findItem( m_share );
      
      // Set the port.
      if ( info && info->port() != -1 )
      {
        m_port_input->setValue( info->port() );
      }
      else
      {
#ifndef Q_OS_FREEBSD
        m_port_input->setValue( Smb4KSettings::remoteFileSystemPort() );
#else
        m_port_input->setValue( Smb4KSettings::remoteSMBPort() );
#endif
      }
      
      // Set the user ID.
      QString user_text;
      
      if ( info && info->uid() != (K_UID)Smb4KSettings::userID().toInt() )
      {
        KUser user( (K_UID)info->uid() );
        user_text = QString( "%1 (%2)" ).arg( user.loginName() ).arg( user.uid() );
      }
      else
      {
        KUser user( (K_UID)Smb4KSettings::userID().toInt() );
        user_text = QString( "%1 (%2)" ).arg( user.loginName() ).arg( user.uid() );
      }
      
      int user_index = m_uid_input->findText( user_text );
      m_uid_input->setCurrentIndex( user_index );
      
      // Set the GID.
      QString group_text;
      
      if ( info && info->gid() != (K_GID)Smb4KSettings::groupID().toInt() )
      {
        KUserGroup group( (K_GID)info->gid() );
        group_text = QString( "%1 (%2)" ).arg( group.name() ).arg( group.gid() );
      }
      else
      {
        KUserGroup group( (K_GID)Smb4KSettings::groupID().toInt() );
        group_text = QString( "%1 (%2)" ).arg( group.name() ).arg( group.gid() );
      }
      
      int group_index = m_gid_input->findText( group_text );
      m_gid_input->setCurrentIndex( group_index );
      
#ifndef Q_OS_FREEBSD
      // Set the write access.
      if ( info && info->writeAccess() != Smb4KSambaOptionsInfo::UndefinedWriteAccess )
      {
        switch( info->writeAccess() )
        {
          case Smb4KSambaOptionsInfo::ReadWrite:
          {
            m_rw_input->setCurrentItem( i18n( "read-write" ), false );
            break;
          }
          case Smb4KSambaOptionsInfo::ReadOnly:
          {
            m_rw_input->setCurrentItem( i18n( "read-only" ), false );
            break;
          }
          case Smb4KSambaOptionsInfo::UndefinedWriteAccess:
          default:
          {
            break;
          }
        }        
      }
      else
      {
        switch ( Smb4KSettings::writeAccess() )
        {
          case Smb4KSettings::EnumWriteAccess::ReadWrite:
          {
            m_rw_input->setCurrentItem( i18n( "read-write" ), false );
            break;
          }
          case Smb4KSettings::EnumWriteAccess::ReadOnly:
          {
            m_rw_input->setCurrentItem( i18n( "read-only" ), false );
            break;
          }
          default:
          {
            break;
          }
        }
      }
#endif

      // Connections:
      connect( m_port_input,  SIGNAL( valueChanged( int ) ),
               this,          SLOT( slotPortChanged( int ) ) );

      connect( m_uid_input,   SIGNAL( activated( const QString & ) ),
               this,          SLOT( slotUIDChanged( const QString & ) ) );

      connect( m_gid_input,   SIGNAL( activated( const QString & ) ),
               this,          SLOT( slotGIDChanged( const QString & ) ) );

#ifndef Q_OS_FREEBSD
      connect( m_rw_input,    SIGNAL( activated( const QString & ) ),
               this,          SLOT( slotWriteAccessChanged( const QString & ) ) );
#endif

      break;
    }
    default:
    {
      // This should not happen...
      break;
    }
  }

  // Enable the buttons:
  enableButton( Ok, false );
  enableButton( User1, !hasDefaultSettings() );

  // Connect the buttons:
  connect( this,          SIGNAL( okClicked() ),
           this,          SLOT( slotOKButtonClicked() ) );

  connect( this,          SIGNAL( user1Clicked() ),
           this,          SLOT( slotDefaultButtonClicked() ) );
}


bool Smb4KCustomOptionsDialog::hasDefaultSettings()
{
  bool has_default_settings = true;
  
  switch ( m_type )
  {
    case Host:
    {
      // Port
      if ( m_port_input->value() != Smb4KSettings::remoteSMBPort() )
      {
        if ( has_default_settings )
        {
          has_default_settings = false;
        }
        else
        {
          // Do nothing
        }
      }
      else
      {
        // Do nothing
      }
      
      // Protocol
      switch ( Smb4KSettings::protocolHint() )
      {
        case Smb4KSettings::EnumProtocolHint::Automatic:
        {
          if ( QString::compare( m_proto_input->currentText(), i18n( "automatic" ), Qt::CaseInsensitive ) != 0 )
          {
            if ( has_default_settings )
            {
              has_default_settings = false;
            }
            else
            {
              // Do nothing
            }            
          }
          else
          {
            // Do nothing
          }
          break;
        }
        case Smb4KSettings::EnumProtocolHint::RPC:
        {
          if ( QString::compare( m_proto_input->currentText(), "RPC", Qt::CaseInsensitive ) != 0 )
          {
            if ( has_default_settings )
            {
              has_default_settings = false;
            }
            else
            {
              // Do nothing
            }            
          }
          else
          {
            // Do nothing
          }
          break;
        }
        case Smb4KSettings::EnumProtocolHint::RAP:
        {
          if ( QString::compare( m_proto_input->currentText(), "RAP", Qt::CaseInsensitive ) != 0 )
          {
            if ( has_default_settings )
            {
              has_default_settings = false;
            }
            else
            {
              // Do nothing
            }            
          }
          else
          {
            // Do nothing
          }
          break;
        }
        case Smb4KSettings::EnumProtocolHint::ADS:
        {
          if ( QString::compare( m_proto_input->currentText(), "ADS", Qt::CaseInsensitive ) != 0 )
          {
            if ( has_default_settings )
            {
              has_default_settings = false;
            }
            else
            {
              // Do nothing
            }            
          }
          else
          {
            // Do nothing
          }
          break;
        }
        default:
        {
          break;
        }
      }
      
      // Kerberos
      if ( m_kerberos->isChecked() != Smb4KSettings::useKerberos() )
      {
        if ( has_default_settings )
        {
          has_default_settings = false;
        }
        else
        {
          // Do nothing
        }    
      }
      else
      {
        // Do nothing
      }
      
      break;
    }
    case Share:
    {
      // Port
#ifndef Q_OS_FREEBSD
      if ( m_port_input->value() != Smb4KSettings::remoteFileSystemPort() )
#else
      if ( m_port_input->value() != Smb4KSettings::remoteSMBPort() )
#endif
      {
        if ( has_default_settings )
        {
          has_default_settings = false;
        }
        else
        {
          // Do nothing
        }
      }
      else
      {
        // Do nothing
      }
      
      // User ID
      if ( (K_UID)Smb4KSettings::userID().toInt() != (K_UID)m_uid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() )
      {
        if ( has_default_settings )
        {
          has_default_settings = false;
        }
        else
        {
          // Do nothing
        }
      }
      else
      {
        // Do nothing
      }
      
      // Group ID
      if ( (K_GID)Smb4KSettings::groupID().toInt() != (K_GID)m_gid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() )
      {
        if ( has_default_settings )
        {
          has_default_settings = false;
        }
        else
        {
          // Do nothing
        }
      }
      else
      {
        // Do nothing
      }
      
#ifndef Q_OS_FREEBSD
      // Write access
      switch ( Smb4KSettings::writeAccess() )
      {
        case Smb4KSettings::EnumWriteAccess::ReadWrite:
        {
          if ( QString::compare( m_rw_input->currentText(), i18n( "read-write" ), Qt::CaseInsensitive ) != 0 )
          {
            if ( has_default_settings )
            {
              has_default_settings = false;
            }
            else
            {
              // Do nothing
            }
          }
          else
          {
            // Do nothing
          }
          break;
        }
        case Smb4KSettings::EnumWriteAccess::ReadOnly:
        {
          if ( QString::compare( m_rw_input->currentText(), i18n( "read-only" ), Qt::CaseInsensitive ) != 0 )
          {
            if ( has_default_settings )
            {
              has_default_settings = false;
            }
            else
            {
              // Do nothing
            }
          }
          else
          {
            // Do nothing
          }
          break;
        }
        default:
        {
          break;
        }
      }
#endif
      
      break;
    }
    default:
    {
      break;
    }
  }
  
  return has_default_settings;
}


bool Smb4KCustomOptionsDialog::hasInitialSettings()
{
  bool has_initial_settings = true;
  
  switch ( m_type )
  {
    case Host:
    {
      Smb4KSambaOptionsInfo *info = Smb4KSambaOptionsHandler::self()->findItem( m_host );
      
      // Port
      if ( info && info->port() != -1 )
      {
        if ( m_port_input->value() != info->port() )
        {
          if ( has_initial_settings )
          {
            has_initial_settings = false;
          }
          else
          {
            // Do nothing
          }
        }
        else
        {
          // Do nothing
        }
      }
      else
      {
        if ( m_port_input->value() != Smb4KSettings::remoteSMBPort() )
        {
          if ( has_initial_settings )
          {
            has_initial_settings = false;
          }
          else
          {
            // Do nothing
          }
        }
        else
        {
          // Do nothing
        }
      }
      
      // Protocol.
      if ( info && info->protocol() != Smb4KSambaOptionsInfo::UndefinedProtocol )
      {
        switch ( info->protocol() )
        {
          case Smb4KSambaOptionsInfo::Automatic:
          {
            if ( QString::compare( m_proto_input->currentText(), i18n( "automatic" ), Qt::CaseInsensitive ) != 0 )
            {
              if ( has_initial_settings )
              {
                has_initial_settings = false;
              }
              else
              {
                // Do nothing
              }
            }
            else
            {
              // Do nothing
            }
            break;
          }
          case Smb4KSambaOptionsInfo::RPC:
          {
            if ( QString::compare( m_proto_input->currentText(), "RPC", Qt::CaseInsensitive ) != 0 )
            {
              if ( has_initial_settings )
              {
                has_initial_settings = false;
              }
              else
              {
                // Do nothing
              }
            }
            else
            {
              // Do nothing
            }
            break;
          }
          case Smb4KSambaOptionsInfo::RAP:
          {
            if ( QString::compare( m_proto_input->currentText(), "RAP", Qt::CaseInsensitive ) != 0 )
            {
              if ( has_initial_settings )
              {
                has_initial_settings = false;
              }
              else
              {
                // Do nothing
              }
            }
            else
            {
              // Do nothing
            }
            break;
          }
          case Smb4KSambaOptionsInfo::ADS:
          {
            if ( QString::compare( m_proto_input->currentText(), "ADS", Qt::CaseInsensitive ) != 0 )
            {
              if ( has_initial_settings )
              {
                has_initial_settings = false;
              }
              else
              {
                // Do nothing
              }
            }
            else
            {
              // Do nothing
            }
            break;
          }
          default:
          {
            break;
          }
        }
      }
      else
      {
        switch ( Smb4KSettings::protocolHint() )
        {
          case Smb4KSettings::EnumProtocolHint::Automatic:
          {
            if ( QString::compare( m_proto_input->currentText(), i18n( "automatic" ), Qt::CaseInsensitive ) != 0 )
            {
              if ( has_initial_settings )
              {
                has_initial_settings = false;
              }
              else
              {
                // Do nothing
              }
            }
            else
            {
              // Do nothing
            }
            break;
          }
          case Smb4KSettings::EnumProtocolHint::RPC:
          {
            if ( QString::compare( m_proto_input->currentText(), "RPC", Qt::CaseInsensitive ) != 0 )
            {
              if ( has_initial_settings )
              {
                has_initial_settings = false;
              }
              else
              {
                // Do nothing
              }
            }
            else
            {
              // Do nothing
            }
            break;
          }
          case Smb4KSettings::EnumProtocolHint::RAP:
          {
            if ( QString::compare( m_proto_input->currentText(), "RAP", Qt::CaseInsensitive ) != 0 )
            {
              if ( has_initial_settings )
              {
                has_initial_settings = false;
              }
              else
              {
                // Do nothing
              }
            }
            else
            {
              // Do nothing
            }
            break;
          }
          case Smb4KSettings::EnumProtocolHint::ADS:
          {
            if ( QString::compare( m_proto_input->currentText(), "ADS", Qt::CaseInsensitive ) != 0 )
            {
              if ( has_initial_settings )
              {
                has_initial_settings = false;
              }
              else
              {
                // Do nothing
              }
            }
            else
            {
              // Do nothing
            }
            break;
          }
          default:
          {
            break;
          }
        }
      }
      
      // Kerberos usage
      if ( info && info->useKerberos() != Smb4KSambaOptionsInfo::UndefinedKerberos )
      {
        switch ( info->useKerberos() )
        {
          case Smb4KSambaOptionsInfo::UseKerberos:
          {
            if ( !m_kerberos->isChecked() )
            {
              if ( has_initial_settings )
              {
                has_initial_settings = false;
              }
              else
              {
                // Do nothing
              }
            }
            else
            {
              // Do nothing
            }
            break;
          }
          case Smb4KSambaOptionsInfo::NoKerberos:
          {
            if ( m_kerberos->isChecked() )
            {
              if ( has_initial_settings )
              {
                has_initial_settings = false;
              }
              else
              {
                // Do nothing
              }
            }
            else
            {
              // Do nothing
            }
            break;
          }
          default:
          {
            break;
          }
        }
      }
      else
      {
        if ( m_kerberos->isChecked() != Smb4KSettings::useKerberos() )
        {
          if ( has_initial_settings )
          {
            has_initial_settings = false;
          }
          else
          {
            // Do nothing
          }
        }
        else
        {
          // Do nothing
        }
      }
      break;
    }
    case Share:
    {
      Smb4KSambaOptionsInfo *info = Smb4KSambaOptionsHandler::self()->findItem( m_share );
      
      // Set the port.
      if ( info && info->port() != -1 )
      {
        if ( m_port_input->value() != info->port() )
        {
          if ( has_initial_settings )
          {
            has_initial_settings = false;
          }
          else
          {
            // Do nothing
          }
        }
        else
        {
          // Do nothing
        }
      }
      else
      {
#ifndef Q_OS_FREEBSD
        if ( m_port_input->value() != Smb4KSettings::remoteFileSystemPort() )
#else
        if ( m_port_input->value() != Smb4KSettings::remoteSMBPort() )
#endif
        {
          if ( has_initial_settings )
          {
            has_initial_settings = false;
          }
          else
          {
            // Do nothing
          }
        }
        else
        {
          // Do nothing
        }
      }
      
      // User ID
      if ( info && info->uid() != (K_UID)Smb4KSettings::userID().toInt() )
      {
        if ( (K_UID)m_uid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() != info->uid() )
        {
          if ( has_initial_settings )
          {
            has_initial_settings = false;
          }
          else
          {
            // Do nothing
          }
        }
        else
        {
          // Do nothing
        }
      }
      else
      {
        if ( (K_UID)m_uid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() != (K_UID)Smb4KSettings::userID().toInt() )
        {
          if ( has_initial_settings )
          {
            has_initial_settings = false;
          }
          else
          {
            // Do nothing
          }
        }
        else
        {
          // Do nothing
        }
      }
      
      // Group ID
      if ( info && info->gid() != (K_GID)Smb4KSettings::groupID().toInt() )
      {
        if ( (K_GID)m_gid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() != (K_GID)info->uid() )
        {
          if ( has_initial_settings )
          {
            has_initial_settings = false;
          }
          else
          {
            // Do nothing
          }
        }
        else
        {
          // Do nothing
        }
      }
      else
      {
        if ( (K_GID)m_gid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() != (K_GID)Smb4KSettings::groupID().toInt() )
        {
          if ( has_initial_settings )
          {
            has_initial_settings = false;
          }
          else
          {
            // Do nothing
          }
        }
        else
        {
          // Do nothing
        }
      }
      
#ifndef Q_OS_FREEBSD
      // Write access
      if ( info && info->writeAccess() != Smb4KSambaOptionsInfo::UndefinedWriteAccess )
      {
        switch( info->writeAccess() )
        {
          case Smb4KSambaOptionsInfo::ReadWrite:
          {
            if ( QString::compare( m_rw_input->currentText(), i18n( "read-write" ), Qt::CaseInsensitive ) != 0 )
            {
              if ( has_initial_settings )
              {
                has_initial_settings = false;
              }
              else
              {
                // Do nothing
              }
            }
            else
            {
              // Do nothing
            }
            break;
          }
          case Smb4KSambaOptionsInfo::ReadOnly:
          {
            if ( QString::compare( m_rw_input->currentText(), i18n( "read-only" ), Qt::CaseInsensitive ) != 0 )
            {
              if ( has_initial_settings )
              {
                has_initial_settings = false;
              }
              else
              {
                // Do nothing
              }
            }
            else
            {
              // Do nothing
            }
            break;
          }
          default:
          {
            break;
          }
        }        
      }
      else
      {
        switch ( Smb4KSettings::writeAccess() )
        {
          case Smb4KSettings::EnumWriteAccess::ReadWrite:
          {
            if ( QString::compare( m_rw_input->currentText(), i18n( "read-write" ), Qt::CaseInsensitive ) != 0 )
            {
              if ( has_initial_settings )
              {
                has_initial_settings = false;
              }
              else
              {
                // Do nothing
              }
            }
            else
            {
              // Do nothing
            }
            break;
          }
          case Smb4KSettings::EnumWriteAccess::ReadOnly:
          {
            if ( QString::compare( m_rw_input->currentText(), i18n( "read-only" ), Qt::CaseInsensitive ) != 0 )
            {
              if ( has_initial_settings )
              {
                has_initial_settings = false;
              }
              else
              {
                // Do nothing
              }
            }
            else
            {
              // Do nothing
            }
            break;
          }
          default:
          {
            break;
          }
        }
      }
#endif      

      break;
    }
    default:
    {
      break;
    }
  }
  
  return has_initial_settings;
}

/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KCustomOptionsDialog::slotPortChanged( int /*val*/ )
{
  enableButton( User1, !hasDefaultSettings() );
  enableButton( Ok, !hasInitialSettings() );
}


void Smb4KCustomOptionsDialog::slotProtocolChanged( const QString &/*protocol*/ )
{
  enableButton( User1, !hasDefaultSettings() );
  enableButton( Ok, !hasInitialSettings() );
}


void Smb4KCustomOptionsDialog::slotKerberosToggled( bool /*on*/ )
{
  enableButton( User1, !hasDefaultSettings() );
  enableButton( Ok, !hasInitialSettings() );
}


void Smb4KCustomOptionsDialog::slotWriteAccessChanged( const QString &/*rw*/ )
{
  enableButton( User1, !hasDefaultSettings() );
  enableButton( Ok, !hasInitialSettings() );
}


void Smb4KCustomOptionsDialog::slotUIDChanged( const QString &/*u*/ )
{
  enableButton( User1, !hasDefaultSettings() );
  enableButton( Ok, !hasInitialSettings() );
}


void Smb4KCustomOptionsDialog::slotGIDChanged( const QString &/*g*/ )
{
  enableButton( User1, !hasDefaultSettings() );
  enableButton( Ok, !hasInitialSettings() );
}


void Smb4KCustomOptionsDialog::slotOKButtonClicked()
{
  if ( hasDefaultSettings() )
  {
    // Remove the item, since it uses the default settings.
    switch ( m_type )
    {
      case Host:
      {
        Smb4KSambaOptionsHandler::self()->removeItem( m_host, true );
        break;
      }
      case Share:
      {
        Smb4KSambaOptionsHandler::self()->removeItem( m_share, true );
        break;
      }
      default:
      {
        break;
      }
    }
  }
  else
  {
    // Save the item with the changed settings.
    switch ( m_type )
    {
      case Host:
      {
        Smb4KSambaOptionsInfo *info = Smb4KSambaOptionsHandler::self()->findItem( m_host, true );
        
        if ( !info )
        {
          // Create a new object
          info = new Smb4KSambaOptionsInfo( m_host );
        }
        else
        {
          // Do nothing
        }
        
        // Put in the information.
        info->setPort( m_port_input->value() );

        if ( QString::compare( m_proto_input->currentText(), i18n( "automatic" ), Qt::CaseInsensitive ) == 0 )
        {
          info->setProtocol( Smb4KSambaOptionsInfo::Automatic );
        }
        else if ( QString::compare( m_proto_input->currentText(), "rpc", Qt::CaseInsensitive ) == 0 )
        {
          info->setProtocol( Smb4KSambaOptionsInfo::RPC );
        }
        else if ( QString::compare( m_proto_input->currentText(), "rap", Qt::CaseInsensitive ) == 0 )
        {
          info->setProtocol( Smb4KSambaOptionsInfo::RAP );
        }
        else if ( QString::compare( m_proto_input->currentText(), "ads", Qt::CaseInsensitive ) == 0 )
        {
          info->setProtocol( Smb4KSambaOptionsInfo::ADS );
        }
        else
        {
          info->setProtocol( Smb4KSambaOptionsInfo::UndefinedProtocol );
        }

        if ( m_kerberos->isChecked() )
        {
          info->setUseKerberos( Smb4KSambaOptionsInfo::UseKerberos );
        }
        else
        {
          info->setUseKerberos( Smb4KSambaOptionsInfo::NoKerberos );
        }
        
        // Add the item.
        Smb4KSambaOptionsHandler::self()->addItem( info, true );
          
        break;
      }
      case Share:
      {
        Smb4KSambaOptionsInfo *info = Smb4KSambaOptionsHandler::self()->findItem( m_share, true );
        
        if ( !info )
        {
          // Create a new object
          info = new Smb4KSambaOptionsInfo( m_share );
        }
        else
        {
          // Do nothing
        }
        
        // Put in the information.
        info->setPort( m_port_input->value() );
        info->setUID( (K_UID)m_uid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() );
        info->setGID( (K_GID)m_gid_input->currentText().section( "(", 1, 1 ).section( ")", 0, 0 ).toInt() );
        
#ifndef Q_OS_FREEBSD
        if ( QString::compare( m_rw_input->currentText(), i18n( "read-write" ), Qt::CaseInsensitive ) == 0 )
        {
          info->setWriteAccess( Smb4KSambaOptionsInfo::ReadWrite );
        }
        else if ( QString::compare( m_rw_input->currentText(), i18n( "read-only" ), Qt::CaseInsensitive ) == 0 )
        {
          info->setWriteAccess( Smb4KSambaOptionsInfo::ReadOnly );
        }
        else
        {
          info->setWriteAccess( Smb4KSambaOptionsInfo::UndefinedWriteAccess );
        }
#endif

        // Add the item.
        Smb4KSambaOptionsHandler::self()->addItem( info, true );
        
        break;
      }
      default:
      {
        break;
      }
    }
  }
  
  KConfigGroup group( Smb4KSettings::self()->config(), "Dialogs" );
  saveDialogSize( group, KConfigGroup::Normal );
}


void Smb4KCustomOptionsDialog::slotDefaultButtonClicked()
{
  switch ( m_type )
  {
    case Host:
    {
      m_port_input->setValue( Smb4KSettings::remoteSMBPort() );
      m_kerberos->setChecked( Smb4KSettings::useKerberos() );
      
      switch ( Smb4KSettings::protocolHint() )
      {
        case Smb4KSettings::EnumProtocolHint::Automatic:
        {
          m_proto_input->setCurrentItem( i18n( "automatic" ), false );
          break;
        }
        case Smb4KSettings::EnumProtocolHint::RPC:
        {
          m_proto_input->setCurrentItem( "RPC", false );
          break;
        }
        case Smb4KSettings::EnumProtocolHint::RAP:
        {
          m_proto_input->setCurrentItem( "RAP", false );
          break;
        }
        case Smb4KSettings::EnumProtocolHint::ADS:
        {
          m_proto_input->setCurrentItem( "ADS", false );
          break;
        }
        default:
        {
          break;
        }
      }

      break;
    }
    case Share:
    {
#ifndef Q_OS_FREEBSD
      m_port_input->setValue( Smb4KSettings::remoteFileSystemPort() );
#else
      m_port_input->setValue( Smb4KSettings::remoteSMBPort() );
#endif

      KUser user( (K_UID)Smb4KSettings::userID().toInt() );
      QString user_text = QString( "%1 (%2)" ).arg( user.loginName() ).arg( user.uid() );
      int user_index = m_uid_input->findText( user_text );
      m_uid_input->setCurrentIndex( user_index );

      KUserGroup group( (K_GID)Smb4KSettings::groupID().toInt() );
      QString group_text = QString( "%1 (%2)" ).arg( group.name() ).arg( group.gid() );
      int group_index = m_gid_input->findText( group_text );
      m_gid_input->setCurrentIndex( group_index );

#ifndef Q_OS_FREEBSD
      switch ( Smb4KSettings::writeAccess() )
      {
        case Smb4KSettings::EnumWriteAccess::ReadWrite:
        {
          m_rw_input->setCurrentItem( i18n( "read-write" ), false );
          break;
        }
        case Smb4KSettings::EnumWriteAccess::ReadOnly:
        {
          m_rw_input->setCurrentItem( i18n( "read-only" ), false );
          break;
        }
        default:
        {
          break;
        }
      }
#endif

      break;
    }
    default:
    {
      break;
    }
  }

  enableButton( User1, !hasDefaultSettings() );
  enableButton( Ok, !hasInitialSettings() );
}

#include "smb4kcustomoptionsdialog.moc"
