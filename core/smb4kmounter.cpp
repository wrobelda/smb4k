/***************************************************************************
    smb4kmounter.cpp  -  The core class that mounts the shares.
                             -------------------
    begin                : Die Jun 10 2003
    copyright            : (C) 2003-2009 by Alexander Reinholdt
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
#include <QApplication>
#include <QDir>
#include <QTextStream>
#include <QTextCodec>
#include <QDesktopWidget>
#ifdef __FreeBSD__
#include <QFileInfo>
#endif

// KDE includes
#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>
#include <kmessagebox.h>
#include <kshell.h>
#include <kstandarddirs.h>

// system includes
#ifdef __FreeBSD__
#include <pwd.h>
#include <sys/param.h>
#include <sys/ucred.h>
#include <sys/mount.h>
#else
#include <stdio.h>
#include <mntent.h>
#endif

// Application specific includes
#include <smb4kmounter.h>
#include <smb4kauthinfo.h>
#include <smb4ksambaoptionsinfo.h>
#include <smb4kcoremessage.h>
#include <smb4kglobal.h>
#include <smb4ksambaoptionshandler.h>
#include <smb4kshare.h>
#include <smb4ksettings.h>
#include <smb4kdefs.h>
#include <smb4khomesshareshandler.h>
#include <smb4kmounter_p.h>
#include <smb4kwalletmanager.h>
#include <smb4kprocess.h>

using namespace Smb4KGlobal;

K_GLOBAL_STATIC( Smb4KMounterPrivate, priv );



Smb4KMounter::Smb4KMounter() : QObject()
{
  m_working = false;
  m_timer_id = -1;
  m_timeout = 0;

  connect( kapp, SIGNAL( aboutToQuit() ),
           this,                         SLOT( slotAboutToQuit() ) );

  connect( Smb4KSolidInterface::self(), SIGNAL( buttonPressed( Smb4KSolidInterface::ButtonType ) ),
           this, SLOT( slotHardwareButtonPressed( Smb4KSolidInterface::ButtonType ) ) );

  connect( Smb4KSolidInterface::self(), SIGNAL( wokeUp() ),
           this, SLOT( slotComputerWokeUp() ) );

  connect( Smb4KSolidInterface::self(), SIGNAL( networkStatusChanged( Smb4KSolidInterface::ConnectionStatus ) ),
           this, SLOT( slotNetworkStatusChanged( Smb4KSolidInterface::ConnectionStatus ) ) );
}


Smb4KMounter::~Smb4KMounter()
{
}


Smb4KMounter *Smb4KMounter::self()
{
  return &priv->instance;
}


void Smb4KMounter::init()
{
  m_timeout = Smb4KSettings::checkInterval();
  m_timer_id = startTimer( m_timeout );

  import();

  if ( Smb4KSolidInterface::self()->networkStatus() == Smb4KSolidInterface::Connected ||
       Smb4KSolidInterface::self()->networkStatus() == Smb4KSolidInterface::Unknown )
  {
    priv->setHardwareReason( false );
    triggerRemounts();
  }
  else
  {
    // Do nothing and wait until the network becomes available.
  }
}


void Smb4KMounter::abort( Smb4KShare *share )
{
  Q_ASSERT( share );

  BasicMountThread *thread = NULL;
  
  if ( !share->isHomesShare() )
  {
    thread = m_cache.object( share->unc( QUrl::None ) );
  }
  else
  {
    thread = m_cache.object( share->homeUNC( QUrl::None ) );
  }
  
  if ( thread && thread->process() &&
       (thread->process()->state() == KProcess::Running || thread->process()->state() == KProcess::Starting) )
  {
    if ( Smb4KSettings::alwaysUseSuperUser() ||
         (Smb4KSettings::useForceUnmount() && thread->process()->type() == Smb4KProcess::Unmount) )
    {
      // Find smb4k_kill program
      QString smb4k_kill = KGlobal::dirs()->findResource( "exe", "smb4k_kill" );

      if ( smb4k_kill.isEmpty() )
      {
        Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "smb4k_kill" );
        return;
      }
      else
      {
        // Do nothing
      }

      // Find sudo program
      QString sudo = KStandardDirs::findExe( "sudo" );

      if ( sudo.isEmpty() )
      {
        Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "sudo" );
        return;
      }
      else
      {
        // Do nothing
      }

      Smb4KProcess kill_proc( Smb4KProcess::Kill, this );
      kill_proc.setShellCommand( sudo+" "+smb4k_kill+" "+QString( thread->process()->pid() ) );
      kill_proc.setOutputChannelMode( KProcess::MergedChannels );
      kill_proc.start();
      kill_proc.waitForFinished( -1 );

      // Tell the process that it was just killed/aborted.
      thread->process()->abort();
    }
    else
    {
      thread->process()->abort();
    }
  }
  else
  {
    // Do nothing
  }
}


bool Smb4KMounter::isAborted( Smb4KShare *share )
{
  Q_ASSERT( share );
  
  BasicMountThread *thread = NULL;
  
  if ( !share->isHomesShare() )
  {
    thread = m_cache.object( share->unc( QUrl::None ) );
  }
  else
  {
    thread = m_cache.object( share->homeUNC( QUrl::None ) );
  }
  
  return (thread && thread->process() && thread->process()->isAborted());
}


void Smb4KMounter::abortAll()
{
  if ( !kapp->closingDown() )
  {
    QStringList keys = m_cache.keys();
    
    foreach ( const QString &key, keys )
    {
      BasicMountThread *thread = m_cache.object( key );
      
      if ( thread && thread->process() &&
            (thread->process()->state() == KProcess::Running || thread->process()->state() == KProcess::Starting) )
      {
        if ( Smb4KSettings::alwaysUseSuperUser() ||
            (Smb4KSettings::useForceUnmount() && thread->process()->type() == Smb4KProcess::Unmount) )
        {
          // Find smb4k_kill program
          QString smb4k_kill = KGlobal::dirs()->findResource( "exe", "smb4k_kill" );

          if ( smb4k_kill.isEmpty() )
          {
            Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "smb4k_kill" );
            return;
          }
          else
          {
            // Do nothing
          }

          // Find sudo program
          QString sudo = KStandardDirs::findExe( "sudo" );

          if ( sudo.isEmpty() )
          {
            Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "sudo" );
            return;
          }
          else
          {
            // Do nothing
          }

          Smb4KProcess kill_proc( Smb4KProcess::Kill, this );
          kill_proc.setShellCommand( sudo+" "+smb4k_kill+" "+QString( thread->process()->pid() ) );
          kill_proc.setOutputChannelMode( KProcess::MergedChannels );
          kill_proc.start();
          kill_proc.waitForFinished( -1 );
        }
        else
        {
          // Do nothing
        }
        
        thread->process()->abort();
      }
      else
      {
        // Do nothing
      }
    }
  }
  else
  {
    // priv has already been deleted
  }
}


bool Smb4KMounter::isRunning( Smb4KShare *share )
{
  Q_ASSERT( share );
  
  BasicMountThread *thread = NULL;
  
  if ( !share->isHomesShare() )
  {
    thread = m_cache.object( share->unc( QUrl::None ) );
  }
  else
  {
    thread = m_cache.object( share->homeUNC( QUrl::None ) );
  }
  
  return (thread && thread->process() && thread->process()->state() == KProcess::Running);
}


void Smb4KMounter::triggerRemounts()
{
  if ( Smb4KSettings::remountShares() || priv->hardwareReason() )
  {
    QList<Smb4KSambaOptionsInfo *> list = Smb4KSambaOptionsHandler::self()->sharesToRemount();

    for ( int i = 0; i < list.size(); ++i )
    {
      QList<Smb4KShare *> mounted_shares = findShareByUNC( list.at( i )->unc() );

      if ( !mounted_shares.isEmpty() )
      {
        bool mount = true;

        for ( int j = 0; j < mounted_shares.size(); ++j )
        {
          if ( !mounted_shares.at( j )->isForeign() )
          {
            mount = false;
            break;
          }
          else
          {
            continue;
          }
        }

        if ( mount )
        {
          // First of all initialize the wallet manager.
          Smb4KWalletManager::self()->init( 0 );

          // Mount the share.
          Smb4KShare share( list.at( i )->unc() );
          share.setWorkgroupName( list.at( i )->workgroupName() );
          share.setHostIP( list.at( i )->ip() );

          mountShare( &share );
        }
        else
        {
          // Do nothing
        }
      }
      else
      {
        // First of all initialize the wallet manager.
        Smb4KWalletManager::self()->init( 0 );

        // Mount the share.
        Smb4KShare share( list.at( i )->unc() );
        share.setWorkgroupName( list.at( i )->workgroupName() );
        share.setHostIP( list.at( i )->ip() );

        mountShare( &share );
      }
    }
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::import()
{
  QList<Smb4KShare> mounted_shares;
  
#ifndef __FreeBSD__

  // Prefer reading from /proc/mounts, because it carries more information.
  // If /proc/mounts does not exist, fall back to using the getmntent() system
  // call.
  if ( !QFile::exists( "/proc/mounts" ) )
  {
    // FIXME: Implement check if we need to import something at all.
    // This seems to be more complicate than I initially thought. I tried
    // KDirWatch and QFileInfo, but they did nothing, because the modification
    // time of the symlink target of /proc/mounts does not change.
    
    QFile file( "/proc/mounts" );
    
    // Read /proc/mounts.
    if ( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
    {
      QTextStream ts( &file );
      // Note: With Qt 4.3 this seems to be obsolete, but we'll
      // keep it for now.
      ts.setCodec( "UTF-8" );
      ts.setAutoDetectUnicode( true );

      QString line;

      // Since we are operating on a symlink, we need to read
      // the file this way. Using ts.atEnd() won't work, because
      // it would immediately return TRUE.
      while ( 1 )
      {
        line = ts.readLine( 0 );

        if ( !line.isEmpty() )
        {
          if ( line.contains( " cifs ", Qt::CaseSensitive ) )
          {
            // Create a new share object and put all available
            // information into it.
            Smb4KShare share;
            share.setUNC( line.section( " cifs ", 0, 0 ).trimmed().section( " ", 0, 0 ).trimmed() );
            share.setPath( line.section( " cifs ", 0, 0 ).trimmed().section( " ", 1, 1 ).trimmed() );
            share.setWorkgroupName( line.section( "domain=", 1, 1 ).section( ",", 0, 0 ).trimmed() );
            share.setHostIP( line.section( "addr=", 1, 1 ).section( ",", 0, 0 ).trimmed() );
            share.setFileSystem( Smb4KShare::CIFS );
            share.setIsMounted( true );
            QString login = line.section( "username=", 1, 1 ).section( ",", 0, 0 ).trimmed();
            share.setLogin( !login.isEmpty() ? login : "guest" ); // Work around empty 'username=' entries
            
            mounted_shares += share;
            continue;
          }
          else
          {
            continue;
          }
        }
        else
        {
          break;
        }
      }

      file.close();
    }
    else
    {
      Smb4KCoreMessage::error( ERROR_OPENING_FILE, file.fileName() );
      return;
    }    
  }
  else
  {
    FILE *file = setmntent( "/etc/mtab", "r" );
    struct mntent *entry = NULL;
    
    while ( (entry = getmntent( file )) != NULL )
    {
      if ( !strncmp( entry->mnt_type, "cifs", strlen( entry->mnt_type ) + 1 ) )
      {
        Smb4KShare share;
        share.setUNC( QString( entry->mnt_fsname ) );
        share.setPath( QString( entry->mnt_dir ) );
        share.setFileSystem( Smb4KShare::CIFS );
        share.setIsMounted( true );
        QString login( hasmntopt( entry, "user" ) );
        share.setLogin( !login.isEmpty() ? login : "guest" ); // Work around empty 'user=' entries
       
        mounted_shares += share;
      }
      else
      {
        continue;
      }
    }
  }

#else

//   struct statfs *buf;
//   int count = getmntinfo( &buf, 0 );
// 
//   if ( count == 0 )
//   {
//     int err_code = errno;
//     Smb4KCoreMessage::error( ERROR_IMPORTING_SHARES, QString::null, strerror( err_code ) );
//     return;
//   }
// 
//   for ( int i = 0; i < count; ++i )
//   {
//     if ( !strcmp( buf[i].f_fstypename, "smbfs" ) )
//     {
//       QByteArray path( buf[i].f_mntonname );
// 
//       Smb4KShare *existing_share = findShareByPath( path );
//       Smb4KShare *share = NULL;
// 
//       if ( existing_share )
//       {
//         share = new Smb4KShare( *existing_share );
// 
//         if ( share->fileSystem() != Smb4KShare::SMBFS )
//         {
//           QFileInfo info( QString( buf[i].f_mntonname )+"/." );
// 
//           share->setFileSystem( Smb4KShare::SMBFS );
//           share->setUID( info.ownerId() );
//           share->setGID( info.groupId() );
//         }
//         else
//         {
//           // Do nothing
//         }
// 
//       }
//       else
//       {
//         QString share_name( buf[i].f_mntfromname );
// 
//         QFileInfo info( QString( buf[i].f_mntonname )+"/." );
// 
//         share = new Smb4KShare( share_name );
//         share->setPath( path );
//         share->setFileSystem ( !strcmp( buf[i].f_fstypename, "smbfs" ) ?
//                                Smb4KShare::SMBFS :
//                                Smb4KShare::Unknown );
//         share->setUID( info.ownerId() );
//         share->setGID( info.groupId() );
//         share->setIsMounted( true );
//       }
// 
//       // Test if share is broken
//       if ( (existing_share && !existing_share->isInaccessible()) || !existing_share )
//       {
//         check( share );
//       }
//       else
//       {
//         // Since new_share is a copy of existing_share, we do not need to do
//         // anything here.
//       }
// 
//       shares.append( share );
//     }
//   }
// 
//   // Apparently, under FreeBSD we do not need to delete
//   // the pointer (see manual page).

#endif

  // Check which shares were unmounted, emit the unmounted() signal
  // on each of the unmounted shares and remove them from the global
  // list.
  bool found = false;
  
  for ( int i = 0; i < mountedSharesList()->size(); ++i )
  {
    for ( int j = 0; j < mounted_shares.size(); ++j )
    {
      // Check the mount point, since that is unique.
      if ( QString::compare( mountedSharesList()->at( i )->canonicalPath(), mounted_shares.at( j ).canonicalPath() ) == 0 ||
           QString::compare( mountedSharesList()->at( i )->path(), mounted_shares.at( j ).path() ) == 0 )
      {
        found = true;
        break;
      }
      else
      {
        continue;
      }
    }
    
    if ( !found )
    {
      mountedSharesList()->at( i )->setIsMounted( false );
      emit unmounted( mountedSharesList()->at( i ) );
      removeMountedShare( mountedSharesList()->at( i ) );
    }
    else
    {
      // Do nothing
    }
    
    found = false;
  }

  // Now add additional information to the shares in the temporary
  // list and insert them to the global list. At the same time, remove
  // the old entry from the list. Also, emit either the updated() or
  // the mounted() signal.
  for ( int i = 0; i < mounted_shares.size(); ++i )
  {
    Smb4KShare *mounted_share = findShareByPath( mounted_shares.at( i ).canonicalPath() );
    
    // Check the share.
    if ( !mounted_share || (mounted_share && !mounted_share->isInaccessible()) )
    {
      check( &mounted_shares[i] );
    }
    else
    {
      mounted_shares[i].setInaccessible( true );
    }
    
    // Is this a mount that was done by the user or by
    // someone else (or the system).
    if ( (mounted_shares.at( i ).uid() == getuid() && mounted_shares.at( i ).gid() == getgid()) ||
         (!mounted_shares.at( i ).isInaccessible() &&
          (QString::fromUtf8( mounted_shares.at( i ).path() ).startsWith( Smb4KSettings::mountPrefix().path() ) ||
           QString::fromUtf8( mounted_shares.at( i ).canonicalPath() ).startsWith( QDir::homePath() ))) ||
         (!mounted_shares.at( i ).isInaccessible() &&
          (QString::fromUtf8( mounted_shares.at( i ).canonicalPath() ).startsWith( QDir( Smb4KSettings::mountPrefix().path() ).canonicalPath() ) ||
           QString::fromUtf8( mounted_shares.at( i ).canonicalPath() ).startsWith( QDir::home().canonicalPath() ))) )
    {
      mounted_shares[i].setForeign( false );
    }
    else
    {
      mounted_shares[i].setForeign( true );
    }
    
    // Get the host that shares this resource and check if we
    // need to set the IP address or workgroup/domain.
    Smb4KHost *host = findHost( mounted_shares.at( i ).hostName(), mounted_shares.at( i ).workgroupName() );
    
    if ( host )
    {
      // Set the IP address if necessary.
      if ( mounted_shares.at( i ).hostIP().isEmpty() || QString::compare( host->ip(), mounted_shares.at( i ).hostIP() ) != 0 )
      {
        mounted_shares[i].setHostIP( host->ip() );
      }
      else
      {
        // Do nothing
      }
      
      // Set the workgroup/domain name if necessary.
      if ( mounted_shares.at( i ).workgroupName().isEmpty() )
      {
        mounted_shares[i].setWorkgroupName( host->workgroupName() );
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
    
    // Now we decide whether we need to emit the update() or the mounted() 
    // signal.
    // It is important for the later processing of the shares that have
    // been unmounted in the meantime that we remove all shares from
    // the global list that have been processed.
    if ( mounted_share )
    {
      // This share was previouly mounted.
      removeMountedShare( mounted_share );

      Smb4KShare *new_share = new Smb4KShare( mounted_shares[i] );
      addMountedShare( new_share );
      emit updated( new_share );
    }
    else
    {
      // This is a new share.
      Smb4KShare *new_share = new Smb4KShare( mounted_shares[i] );
      addMountedShare( new_share );
      emit mounted( new_share );
    }
  }
}


void Smb4KMounter::mountShare( Smb4KShare *share )
{
  Q_ASSERT( share );

  // Find smb4k_mount program
  QString smb4k_mount = KGlobal::dirs()->findResource( "exe", "smb4k_mount" );

  if ( smb4k_mount.isEmpty() )
  {
    Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "smb4k_mount" );
    return;
  }
  else
  {
    // Do nothing
  }

  // Find the sudo program
  QString sudo = KStandardDirs::findExe( "sudo" );

  // Check that sudo is installed in the case it is needed.
  if ( Smb4KSettings::alwaysUseSuperUser() && sudo.isEmpty() )
  {
    Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "sudo" );
    return;
  }
  else
  {
    // Do nothing
  }
  
  QList<Smb4KShare *> list;

  if ( share->isHomesShare() )
  {
    QWidget *parent = 0;

    if ( kapp )
    {
      if ( kapp->activeWindow() )
      {
        parent = kapp->activeWindow();
      }
      else
      {
        parent = kapp->desktop();
      }
    }
    else
    {
      // Do nothing
    }

    if ( !Smb4KHomesSharesHandler::self()->specifyUser( share, parent ) )
    {
      return;
    }
    else
    {
      // Do nothing
    }
    
    list = findShareByUNC( share->homeUNC( QUrl::None ) );
  }
  else
  {
    list = findShareByUNC( share->unc( QUrl::None ) );
  }
  
  // Before doing anything else let's check that the
  // share has not already been mounted by the user:
  for ( int i = 0; i != list.size(); ++i )
  {
    if ( !list.at( i )->isForeign() )
    {
      return;
    }
    else
    {
      continue;
    }
  }
  
  // Assemble the mount point and create it.
  QString path;
  path += Smb4KSettings::mountPrefix().path();
  path += QDir::separator();
  path += (Smb4KSettings::forceLowerCaseSubdirs() ? share->hostName().toLower() : share->hostName());
  path += QDir::separator();
  
  if ( !share->isHomesShare() )
  {
    path += (Smb4KSettings::forceLowerCaseSubdirs() ? share->shareName().toLower() : share->shareName());
  }
  else
  {
    path += (Smb4KSettings::forceLowerCaseSubdirs() ? share->login().toLower() : share->login());
  }

  QDir dir( QDir::cleanPath( path ) );

  if ( !dir.mkpath( dir.path() ) )
  {
    Smb4KCoreMessage::error( ERROR_MKDIR_FAILED, dir.path() );
    return;
  }
  else
  {
    share->setPath( dir.path() );
  }

  // Get the authentication information.
  Smb4KAuthInfo authInfo( share );
  Smb4KWalletManager::self()->readAuthInfo( &authInfo );

  // Set the login and the file system for the share.
#ifndef __FreeBSD__
  share->setFileSystem( Smb4KShare::CIFS );
#else
  share->setFileSystem( Smb4KShare::SMBFS );
#endif
  share->setLogin( QString::fromUtf8( authInfo.login() ) );

  // Compile the command
  QString command;
  Smb4KSambaOptionsInfo *options_info  = Smb4KSambaOptionsHandler::self()->findItem( share, true );
  QMap<QString, QString> global_options = Smb4KSambaOptionsHandler::self()->globalSambaOptions();

  if ( Smb4KSettings::alwaysUseSuperUser() )
  {
    command += sudo;
    command += " "+smb4k_mount;
  }
  else
  {
    command += smb4k_mount;
  }

#ifndef __FreeBSD__
  command += " -o";
  command += " ";

  // Workgroup
  command += !share->workgroupName().trimmed().isEmpty() ?
             QString( "domain=%1" ).arg( KShell::quoteArg( share->workgroupName() ) ) :
             "";
  
  // Host IP
  command += !share->hostIP().trimmed().isEmpty() ?
             QString( ",ip=%1" ).arg( share->hostIP() ) :
             "";

  // User
  command += !authInfo.login().isEmpty() ?
             QString( ",user=%1" ).arg( QString::fromUtf8( authInfo.login() ) ) :
             ",guest";
             
  // Client's and server's NetBIOS name
  // According to the manual page, this is only needed when port 139
  // is used. So, we only pass the NetBIOS name in that case.  
  if ( Smb4KSettings::remoteFileSystemPort() == 139 || (options_info && options_info->port() == 139) )
  {
    // The client's NetBIOS name.
    if ( !Smb4KSettings::netBIOSName().isEmpty() )
    {
      command += QString( ",netbiosname=%1" ).arg( KShell::quoteArg( Smb4KSettings::netBIOSName() ) );
    }
    else
    {
      if ( !global_options["netbios name"].isEmpty() )
      {
        command += QString( ",netbiosname=%1" ).arg( KShell::quoteArg( global_options["netbios name"] ) );
      }
      else
      {
        // Do nothing
      }
    }

    // The server's NetBIOS name.
    command += ",servern="+KShell::quoteArg( share->hostName() );
  }
  else
  {
    // Do nothing
  }
  
  // UID
  command += QString( ",uid=%1" ).arg( options_info ? options_info->uid() : (uid_t)Smb4KSettings::userID().toInt() );
  
  // GID
  command += QString( ",gid=%1" ).arg( options_info ? options_info->gid() : (gid_t)Smb4KSettings::groupID().toInt() );
  
  // Client character set
  switch ( Smb4KSettings::clientCharset() )
  {
    case Smb4KSettings::EnumClientCharset::default_charset:
    {
      if ( !global_options["unix charset"].isEmpty() )
      {
        command += QString( ",iocharset=%1" ).arg( global_options["unix charset"].toLower() );
      }
      else
      {
        // Do nothing
      }
      break;
    }
    default:
    {
      command += QString( ",iocharset=%1" ).arg( Smb4KSettings::self()->clientCharsetItem()->label() );
      break;
    }
  }
  
  // Port
  command += QString( ",port=%1" ).arg( (options_info && options_info->port() != -1) ?
                                        options_info->port() : Smb4KSettings::remoteFileSystemPort() );
                                        
  // Write access
  if ( options_info )
  {
    switch ( options_info->writeAccess() )
    {
      case Smb4KSambaOptionsInfo::ReadWrite:
      {
        command += ",rw";
        break;
      }
      case Smb4KSambaOptionsInfo::ReadOnly:
      {
        command += ",ro";
        break;
      }
      default:
      {
        switch ( Smb4KSettings::writeAccess() )
        {
          case Smb4KSettings::EnumWriteAccess::ReadWrite:
          {
            command += ",rw";
            break;
          }
          case Smb4KSettings::EnumWriteAccess::ReadOnly:
          {
            command += ",ro";
            break;
          }
          default:
          {
            break;
          }
        }
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
        command += ",rw";
        break;
      }
      case Smb4KSettings::EnumWriteAccess::ReadOnly:
      {
        command += ",ro";
        break;
      }
      default:
      {
        break;
       }
    }
  }
  
  // File mask
  command += !Smb4KSettings::fileMask().isEmpty() ? QString( ",file_mode=%1" ).arg( Smb4KSettings::fileMask() ) : "";

  // Directory mask
  command += !Smb4KSettings::directoryMask().isEmpty() ? QString( ",dir_mode=%1" ).arg( Smb4KSettings::directoryMask() ) : "";

  // Permission checks
  command += Smb4KSettings::permissionChecks() ? ",perm" : ",noperm";

  // Client controls IDs
  command += Smb4KSettings::clientControlsIDs() ? ",setuids" : ",nosetuids";
  
  // Server inode numbers
  command += Smb4KSettings::serverInodeNumbers() ? ",serverino" : ",noserverino";

  // Inode data caching
  command += Smb4KSettings::noInodeDataCaching() ? ",directio" : "";
  
  // Translate reserved characters
  command += Smb4KSettings::translateReservedChars() ? ",mapchars" : ",nomapchars";
  
  // Locking
  command += Smb4KSettings::noLocking() ? ",nolock" : "";

  // Global custom options provided by the user
  command += !Smb4KSettings::customCIFSOptions().isEmpty() ? Smb4KSettings::customCIFSOptions() : "";
  
  // Fix existing comma, if necessary.
  if ( command.endsWith( "," ) )
  {
    command.truncate( command.length() - 1 );
  }
  else
  {
    // Do nothing
  }
#else
  // Workgroup
  command += !share->workgroupName().isEmpty() ?
             QString( " -W %1" ).arg( KShell::quoteArg( share->workgroupName() ) :
             "";
  
  // Host IP
  command += !share->hostIP().isEmpty() ? QString( " -I %1" ).arg( share->hostIP() ) : "";

  // Do not ask for a password. Use ~/.nsmbrc instead.
  command += " -N";
  
  // UID
  command += QString( " -u %1" ).arg( options_info ? options_info->uid() : (uid_t)Smb4KSettings::userID().toInt() );

  // GID
  command += QString( " -g %1" ).arg( options_info ?  options_info->gid() : (uid_t)Smb4KSettings::groupID().toInt() );
  
  // Client character set and server codepage
  QString charset, codepage;

  switch ( Smb4KSettings::clientCharset() )
  {
    case Smb4KSettings::EnumClientCharset::default_charset:
    {
      charset = global_options["unix charset"].toLower(); // maybe empty
      break;
    }
    default:
    {
      charset = Smb4KSettings::self()->clientCharsetItem()->label();
      break;
    }
  }
  
  switch ( Smb4KSettings::serverCodepage() )
  {
    case Smb4KSettings::EnumServerCodepage::default_codepage:
    {
      codepage = global_options["dos charset"].toLower(); // maybe empty
      break;
    }
    default:
    {
      codepage = Smb4KSettings::self()->serverCodepageItem()->label();
      break;
    }
  }
  
  command += (!charset.isEmpty() && !codepage.isEmpty()) ? QString( " -E %1:%2" ).arg( charset, codepage ) : "";

  // File mask
  command += !Smb4KSettings::fileMask().isEmpty() ? QString( " -f %1" ).arg( Smb4KSettings::fileMask() ) : "";

  // Directory mask
  command += !Smb4KSettings::directoryMask().isEmpty() : QString( " -d %1" ).arg( Smb4KSettings::directoryMask() ) : "";
#endif
  command += " --";
#ifndef __FreeBSD__
  if ( !share->isHomesShare() )
  {
    command += " "+KShell::quoteArg( share->unc() );
  }
  else
  {
    command += " "+KShell::quoteArg( share->homeUNC() );
  }
#else
  if ( options_info )
  {
    share->setPort( options_info->port() != -1 ? options_info->port() : Smb4KSettings::remoteSMBPort() );
  }
  else
  {
    share->setPort( Smb4KSettings::remoteSMBPort() );
  }

  if ( !share->isHomesShare() )
  {
    command += " "+KShell::quoteArg( share->unc( QUrl::RemoveScheme|QUrl::RemovePassword ) );
  }
  else
  {
    command += " "+KShell::quoteArg( share->homeUNC( QUrl::RemoveScheme|QUrl::RemovePassword ) );
  }
#endif
  command += " "+KShell::quoteArg( share->path() );

  // Start mounting the share.
  if ( m_cache.size() == 0 )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );
    m_working = true;
    // State was set above.
    emit stateChanged();
  }
  else
  {
    // Already running
  }

  emit aboutToStart( share, MountShare );

  MountThread *thread = new MountThread( share, this );
  
  if ( !share->isHomesShare() )
  {
    m_cache.insert( share->unc( QUrl::None ), thread );
  }
  else
  {
    m_cache.insert( share->homeUNC( QUrl::None ), thread );
  }

  connect( thread, SIGNAL( finished() ),
           this,   SLOT( slotThreadFinished() ) );
  connect( thread, SIGNAL( mounted( Smb4KShare * ) ),
           this,   SLOT( slotShareMounted( Smb4KShare * ) ) );
  connect( thread, SIGNAL( authError( Smb4KShare * ) ),
           this,   SLOT( slotAuthError( Smb4KShare * ) ) );
  connect( thread, SIGNAL( badShareName( Smb4KShare * ) ),
           this,   SLOT( slotBadShareNameError( Smb4KShare * ) ) );
           
  thread->start();
  thread->mount( &authInfo, command );
}


void Smb4KMounter::unmountShare( Smb4KShare *share, bool force, bool silent )
{
  // FIXME: Do not forget to set share->setMounted( false ) in slotShareUnmounted()
  // before the unmounted() signal is emitted!
  
  Q_ASSERT( share );

  // Find the smb4k_umount program
  QString smb4k_umount = KGlobal::dirs()->findResource( "exe", "smb4k_umount" );

  if ( smb4k_umount.isEmpty() )
  {
    Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "smb4k_umount" );
    return;
  }
  else
  {
    // Do nothing
  }

  // Find the sudo program
  QString sudo = KStandardDirs::findExe( "sudo" );

  // Check that sudo is installed in the case it is needed.
  if ( ((force && Smb4KSettings::useForceUnmount()) || Smb4KSettings::alwaysUseSuperUser()) &&
       sudo.isEmpty() )
  {
    Smb4KCoreMessage::error( ERROR_COMMAND_NOT_FOUND, "sudo" );
    return;
  }
  else
  {
    // Do nothing
  }

  // Copy the share to avoid problems with 'homes' shares.
  Smb4KShare internal_share( *share );

  if ( internal_share.isForeign() && !Smb4KSettings::unmountForeignShares() )
  {
    if ( !silent )
    {
      Smb4KCoreMessage::error( ERROR_UNMOUNTING_NOT_ALLOWED );
    }
    else
    {
      // Do nothing
    }

    return;
  }
  else
  {
    // Do nothing
  }

  if ( force )
  {
    QWidget *parent = 0;

    if ( kapp )
    {
      if ( kapp->activeWindow() )
      {
        parent = kapp->activeWindow();
      }
      else
      {
        parent = kapp->desktop();
      }
    }
    else
    {
      // Do nothing
    }

    // Ask the user, if he/she really wants to force the unmounting.
    if ( KMessageBox::questionYesNo( parent, i18n( "<qt>Do you really want to force the unmounting of this share?</qt>" ), QString(), KStandardGuiItem::yes(), KStandardGuiItem::no(), "Dont Ask Forced", KMessageBox::Notify ) == KMessageBox::No )
    {
      return;
    }
    else
    {
      // Do nothing
    }

    // If 'force' is TRUE but the user choose to not force
    // unmounts, throw an error here and exit.
    if ( !Smb4KSettings::useForceUnmount() )
    {
      if ( !silent )
      {
        Smb4KCoreMessage::error( ERROR_FEATURE_NOT_ENABLED );
      }
      else
      {
        // Do nothing
      }

      return;
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

  // Compile the command.
  QString command;

  if ( (force && Smb4KSettings::useForceUnmount()) || Smb4KSettings::alwaysUseSuperUser() )
  {
    command += sudo;
    command += " "+smb4k_umount;
  }
  else
  {
    command += smb4k_umount;
  }

#ifdef __linux__
  if ( force && Smb4KSettings::useForceUnmount() )
  {
    command += " -l"; // lazy unmount
  }
  else
  {
    // Do nothing
  }
#endif

  command += " --";
  command += " "+KShell::quoteArg( share->canonicalPath() );
  
  // Start unmounting the share.
  if ( m_cache.size() == 0 )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );
    m_working = true;
    // State was set above.
    emit stateChanged();
  }
  else
  {
    // Already running
  }

  emit aboutToStart( share, UnmountShare );
  
  UnmountThread *thread = new UnmountThread( share, this );
  m_cache.insert( share->unc( QUrl::None ), thread );
  
  connect( thread, SIGNAL( finished() ),
           this,   SLOT( slotThreadFinished() ) );
  connect( thread, SIGNAL( unmounted( Smb4KShare * ) ),
           this,   SLOT( slotShareUnmounted( Smb4KShare * ) ) );
  
  thread->start();
  thread->unmount( command );
}


void Smb4KMounter::unmountAllShares()
{
  // Never use while( !mountedSharesList().isEmpty() ) {} here,
  // because then the mounter will loop indefinitely when the
  // unmounting of a share fails.
  QListIterator<Smb4KShare *> it( *mountedSharesList() );

  while ( it.hasNext() )
  {
    unmountShare( it.next(), false, true );
  }
}


void Smb4KMounter::prepareForShutdown()
{
  slotAboutToQuit();
}


void Smb4KMounter::check( Smb4KShare *share )
{
  if ( share )
  {
    CheckThread thread( share, this );
    thread.start();
    thread.check();
    thread.wait( 100 );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::saveSharesForRemount()
{
  if ( (Smb4KSettings::remountShares() && priv->aboutToQuit()) || priv->hardwareReason() )
  {
    for ( int i = 0; i < mountedSharesList()->size(); ++i )
    {
      if ( !mountedSharesList()->at( i )->isForeign() )
      {
        Smb4KSambaOptionsHandler::self()->addRemount( mountedSharesList()->at( i ) );
      }
      else
      {
        Smb4KSambaOptionsHandler::self()->removeRemount( mountedSharesList()->at( i ) );
      }
    }

    Smb4KSambaOptionsHandler::self()->sync();
  }
  else
  {
    if ( !Smb4KSettings::remountShares() )
    {
      Smb4KSambaOptionsHandler::self()->clearRemounts();
    }
    else
    {
      // Do nothing
    }
  }
}


void Smb4KMounter::timerEvent( QTimerEvent * )
{
  if ( !kapp->startingUp() && !m_working )
  {
    // Import the mounted shares.
    import();
  }
  else
  {
    // Do nothing and wait until the application started up.
  }

  if ( m_timeout != Smb4KSettings::checkInterval() )
  {
    m_timeout = Smb4KSettings::checkInterval();
    killTimer( m_timer_id );
    m_timer_id = startTimer( m_timeout );
  }
  else
  {
    // Do nothing
  }
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////


void Smb4KMounter::slotAboutToQuit()
{
  // Tell the application it is about to quit.
  priv->setAboutToQuit();

  // Abort any actions.
  abortAll();

  // Save the shares that need to be remounted.
  saveSharesForRemount();

  // Unmount the shares if the user chose to do so.
  if ( Smb4KSettings::unmountSharesOnExit() )
  {
    unmountAllShares();
  }
  else
  {
    // Do nothing
  }

  // Clean up the mount prefix.
  QDir dir;
  dir.cd( Smb4KSettings::mountPrefix().path() );
  QStringList dirs = dir.entryList( QDir::Dirs|QDir::NoDotAndDotDot, QDir::NoSort );

  QList<Smb4KShare *> inaccessible = findInaccessibleShares();

  // Remove all directories from the list that belong to
  // inaccessible shares.
  for ( int i = 0; i < inaccessible.size(); ++i )
  {
    int index = dirs.indexOf( inaccessible.at( i )->hostName(), 0 );

    if ( index != -1 )
    {
      dirs.removeAt( index );
      continue;
    }
    else
    {
      continue;
    }
  }

  // Now it is save to remove all empty directories.
  for ( int i = 0; i < dirs.size(); ++i )
  {
    dir.cd( dirs.at( i ) );

    QStringList subdirs = dir.entryList( QDir::Dirs|QDir::NoDotAndDotDot, QDir::NoSort );

    for ( int k = 0; k < subdirs.size(); ++k )
    {
      dir.rmdir( subdirs.at( k ) );
    }

    dir.cdUp();
    dir.rmdir( dirs.at( i ) );
  }
}


void Smb4KMounter::slotThreadFinished()
{
  QStringList keys = m_cache.keys();

  foreach ( const QString &key, keys )
  {
    BasicMountThread *thread = m_cache.object( key );

    if ( thread->isFinished() )
    {
      switch ( thread->process()->type() )
      {
        case Smb4KProcess::Mount:
        {
          emit finished( thread->share(), MountShare );
          break;
        }
        case Smb4KProcess::Unmount:
        {
          emit finished( thread->share(), UnmountShare );
          break;
        }
        default:
        {
          break;
        }
      }

      m_cache.remove( key );
    }
    else
    {
      // Do nothing
    }
  }

  if ( m_cache.size() == 0 )
  {
    m_working = false;
    m_state = MOUNTER_STOP;
    emit stateChanged();
    QApplication::restoreOverrideCursor();
  }
  else
  {
    // Still running
  }
}


void Smb4KMounter::slotShareMounted( Smb4KShare *share )
{
  Q_ASSERT( share );
  
  // Check that we actually mounted the share and emit 
  // the mounted() signal if we found it.
  FILE *file = setmntent( "/etc/mtab", "r" );
  struct mntent *entry = NULL;
  bool share_mounted = false;

  while ( (entry = getmntent( file )) != NULL )
  {
    if ( !strncmp( entry->mnt_type, "cifs", strlen( entry->mnt_type ) + 1 ) )
    {
      QString name( entry->mnt_fsname );
          
      if ( QString::compare( share->unc(), name, Qt::CaseInsensitive ) == 0 )
      {
        share_mounted = true;
        break;
      }
      else
      {
        continue;
      }
    }
    else
    {
      continue;
    }
  }
  
  if ( share_mounted )
  {
    // Remove the remount flag and write the options
    // to the disk.
    if ( Smb4KSambaOptionsHandler::self()->findItem( share ) != NULL )
    {
      Smb4KSambaOptionsHandler::self()->removeRemount( share );
      Smb4KSambaOptionsHandler::self()->sync();
    }
    else
    {
      // Do nothing
    }

    // Check the usage, etc.
    check( share );

    // Create a new share object and add it to the list
    // of mounted shares.
    Smb4KShare *new_share = new Smb4KShare( *share );
  
    if ( share->isHomesShare() )
    {
      new_share->setUNC( share->homeUNC( QUrl::None ) );
    }
    else
    {
      // Do nothing
    }
  
    addMountedShare( new_share );
  
    // Finally, emit the mounted() signal.
    emit mounted( new_share );
  }
  else
  {
    // Do nothing. Mounting failed.
  }
}


void Smb4KMounter::slotShareUnmounted( Smb4KShare *share )
{
  Q_ASSERT( share );
  
  // Check that we actually unmounted the share and emit 
  // the mounted() signal if it is really gone.
  FILE *file = setmntent( "/etc/mtab", "r" );
  struct mntent *entry = NULL;
  bool share_unmounted = true;

  while ( (entry = getmntent( file )) != NULL )
  {
    if ( !strncmp( entry->mnt_type, "cifs", strlen( entry->mnt_type ) + 1 ) )
    {
      QString name( entry->mnt_fsname );
          
      if ( QString::compare( share->unc(), name, Qt::CaseInsensitive ) == 0 )
      {
        share_unmounted = false;
        break;
      }
      else
      {
        continue;
      }
    }
    else
    {
      continue;
    }
  }
  
  if ( share_unmounted )
  {
    // Clean up the mount prefix.
    if ( qstrncmp( share->canonicalPath(),
        QDir( Smb4KSettings::mountPrefix().path() ).canonicalPath().toUtf8(),
        QDir( Smb4KSettings::mountPrefix().path() ).canonicalPath().toUtf8().length() ) == 0 )
    {
      QDir dir( share->canonicalPath() );

      if ( dir.rmdir( dir.canonicalPath() ) )
      {
        dir.cdUp();
        dir.rmdir( dir.canonicalPath() );
      }
      else
      {
        // Do nothing
      }
    }
    else
    {
      // Do nothing here. Do not remove any paths that are outside the
      // mount prefix.
    }
    
    // Find the share in the list, emit the unmounted() signal and finally
    // remove the share from the list of mounted shares.
    Smb4KShare *mounted_share = findShareByPath( share->path() );
    
    if ( mounted_share )
    {
      mounted_share->setIsMounted( false );
      emit unmounted( mounted_share );
      removeMountedShare( mounted_share );
    }
    else
    {
      // Do nothing
    }
  }
  else
  {
    // Do nothing. The unmounting failed.
  }
}


void Smb4KMounter::slotAuthError( Smb4KShare *share )
{
  Q_ASSERT( share );
  
  Smb4KAuthInfo authInfo( share );

  if ( Smb4KWalletManager::self()->showPasswordDialog( &authInfo, 0 ) )
  {
    mountShare( share );
  }
  else
  {
    // Do nothing
  }
}


void Smb4KMounter::slotBadShareNameError( Smb4KShare *share )
{
  Q_ASSERT( share );
  
  // Since we are working on a copy of the initial share, we
  // can change the share name without changing the original
  // share object in any way.
  QString name = static_cast<QString>( share->shareName() ).replace( "_", " " );
  share->setShareName( name );
  mountShare( share );  
}


void Smb4KMounter::slotHardwareButtonPressed( Smb4KSolidInterface::ButtonType type )
{
  switch ( type )
  {
    case Smb4KSolidInterface::SleepButton:
    {
      if ( Smb4KSettings::unmountWhenSleepButtonPressed() )
      {
        priv->setHardwareReason( true );
        abortAll();
        saveSharesForRemount();
        unmountAllShares();
        priv->setHardwareReason( false );
      }
      else
      {
        // Do nothing
      }

      break;
    }
    case Smb4KSolidInterface::LidButton:
    {
      if ( Smb4KSettings::unmountWhenLidButtonPressed() )
      {
        priv->setHardwareReason( true );
        abortAll();
        saveSharesForRemount();
        unmountAllShares();
        priv->setHardwareReason( false );
      }
      else
      {
        // Do nothing
      }

      break;
    }
    case Smb4KSolidInterface::PowerButton:
    {
      if ( Smb4KSettings::unmountWhenPowerButtonPressed() )
      {
        priv->setHardwareReason( true );
        abortAll();
        saveSharesForRemount();
        unmountAllShares();
        priv->setHardwareReason( false );
      }
      else
      {
        // Do nothing
      }
    }
    default:
    {
      break;
    }
  }
}


void Smb4KMounter::slotComputerWokeUp()
{
  // Only trigger a remount here, if the network connection is
  // established. If the computer is still disconnected,
  // slotNetworkStatusChanged() will initiate the remounting.
  switch ( Smb4KSolidInterface::self()->networkStatus() )
  {
    case Smb4KSolidInterface::Connected:
    case Smb4KSolidInterface::Unknown:
    {
      priv->setHardwareReason( true );
      triggerRemounts();
      priv->setHardwareReason( false );
      break;
    }
    default:
    {
      break;
    }
  }
}


void Smb4KMounter::slotNetworkStatusChanged( Smb4KSolidInterface::ConnectionStatus status )
{
  switch ( status )
  {
    case Smb4KSolidInterface::Connected:
    {
      priv->setHardwareReason( true );
      triggerRemounts();
      priv->setHardwareReason( false );
      break;
    }
    case Smb4KSolidInterface::Disconnected:
    {
      priv->setHardwareReason( true );
      abortAll();
      saveSharesForRemount();
      unmountAllShares();
      priv->setHardwareReason( false );
      break;
    }
    case Smb4KSolidInterface::Unknown:
    {
      priv->setHardwareReason( true );
      triggerRemounts();
      priv->setHardwareReason( false );
      break;
    }
    default:
    {
      break;
    }
  }
}

#include "smb4kmounter.moc"
