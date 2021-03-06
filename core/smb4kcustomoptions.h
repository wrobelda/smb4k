/***************************************************************************
    This class carries custom options
                             -------------------
    begin                : Fr 29 Apr 2011
    copyright            : (C) 2011-2020 by Alexander Reinholdt
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

#ifndef SMB4KCUSTOMOPTIONS_H
#define SMB4KCUSTOMOPTIONS_H

// application specific includes
#include "smb4khost.h"
#include "smb4kshare.h"
#include "smb4kglobal.h"

// Qt includes
#include <QScopedPointer>
#include <QUrl>

// KDE includes
#include <KCoreAddons/KUser>

// forward declarations
class Smb4KCustomOptionsPrivate;

using namespace Smb4KGlobal;

/**
 * This class stores the custom options defined for a certain host
 * or share.
 *
 * @author Alexander Reinholdt <alexander.reinholdt@kdemail.net>
 * @since 1.0.0
 */

class Q_DECL_EXPORT Smb4KCustomOptions
{
  friend class Smb4KCustomOptionsPrivate;
  
  public:
    /**
     * Constructor for a host
     */
    explicit Smb4KCustomOptions(Smb4KHost *host);
                
    /**
     * Constructor for a share
     */
    explicit Smb4KCustomOptions(Smb4KShare *share);
    
    /**
     * Copy constructor
     */
    Smb4KCustomOptions(const Smb4KCustomOptions &options);
    
    /**
     * Empty constructor
     */
    Smb4KCustomOptions();
    
    /**
     * Destructor
     */
    ~Smb4KCustomOptions();
    
    /**
     * Sets the host object. If you already set a network item before,
     * this function will do nothing.
     * 
     * @param host          The host object
     */
    void setHost(Smb4KHost *host);
    
    /**
     * Sets the share object. If you already set a host item before,
     * you can overwrite it with this function if the host names and
     * workgroup names match. This way you can propagate options that 
     * were defined for a host to one of its shares.
     * 
     * @param share         The host object
     */
    void setShare(Smb4KShare *share);
    
    /**
     * Returns the type of the network item for that the options
     * are defined
     * 
     * @returns the type of the network item
     */
    Smb4KGlobal::NetworkItem type() const;
    
    /**
     * Sets the workgroup name.
     * 
     * @param workgroup       The workgroup name
     */
    void setWorkgroupName(const QString &workgroup);
    
    /**
     * Returns the workgroup name.
     * 
     * @returns the workgroup name.
     */
    QString workgroupName() const;

    /**
     * Sets the URL of the network item
     * 
     * @param url             The URL
     */
    void setUrl(const QUrl &url);
    
    /**
     * Returns the URL of the network item
     * 
     * @returns the URL
     */
    QUrl url() const;

    /**
     * Returns the host name.
     *
     * @returns the host name.
     */
    QString hostName() const;

    /**
     * Returns the share name, if appropriate, or an empty string.
     *
     * @returns the share name
     */
    QString shareName() const;
                                                   
    /**
     * Sets the IP address for the network item
     * 
     * @param ip              The IP address
     */
    void setIpAddress(const QString &ip);
    
    /**
     * Returns the IP address of the network item
     * 
     * @returns the IP address
     */
    QString ipAddress() const;
    
    /**
     * Returns TRUE if the IP address is set and FALSE otherwise.
     * 
     * @returns TRUE if the IP address is known.
     */
    bool hasIpAddress() const;
    
    /**
     * Returns the display string. Prefer this over all other alternatives in your
     * GUI.
     * @returns the display string.
     */
    QString displayString() const;

    /**
     * Remount enumeration
     * 
     * @param RemountOnce       Remount the share only next time the application
     *                          is started.
     * @param RemountAlways     Remount the share every time the application is
     *                          started.
     * @param UndefinedRemount  No remount behavior is undefined.
     */
    enum Remount { RemountOnce,
                   RemountAlways,
                   UndefinedRemount };
    
    /**
     * If the network item is of type Share, set if it should be remounted.
     * If the network item is not of type Share, this function does nothing.
     * 
     * @param remount       One entry of the Remount enumeration
     */
    void setRemount(int remount);
    
    /**
     * Returns if the network item should be remounted.
     * 
     * @returns if the network item should be remounted.
     */
    int remount() const;
    
    /**
     * Set if the information about the user that is to be owner of the share 
     * should be used when mounting or not.
     * 
     * @param usage            Boolean that determines whether the uid should be
     *                          used.
     */
    void setUseUser(bool use);
    
    /**
     * Use the information about the user that is to be owner of the share.
     * 
     * @returns TRUE if the uid should be used when mounting.
     */
    bool useUser() const;
    
    /**
     * Set the user who owns the share.
     * @param user    The user
     */
    void setUser(const KUser &user);
    
    /**
     * Returns the user who owns the share.
     * @returns the user
     */
    KUser user() const;
    
    /**
     * Set if the information about the group that is to be owner of the share 
     * should be used when mounting or not.
     * 
     * @param use      Boolean that determines whether the gid should be used.
     */
    void setUseGroup(bool use);
    
    /**
     * Use the information about the group that is to be owner of the share.
     * 
     * @returns TRUE if the gid should be used when mounting.
     */
    bool useGroup() const;
    
    /**
     * Set the group that owns the share.
     * 
     * @param group   The group
     */
    void setGroup(const KUserGroup &group);
    
    /**
     * Returns the group that owns the share.
     * 
     * @returns the group
     */
    KUserGroup group() const;
    
    /**
     * Set if the file mode should be used.
     * 
     * @param use     Boolean that determines whether the file mode should be used.
     */
    void setUseFileMode(bool use);
    
    /**
     * Returns if the file mode should be used.
     * 
     * @return TRUE if the file mode should be used
     */
    bool useFileMode() const;
    
    /**
     * Set the file mode that should be used. The value must be defined in octal.
     * 
     * @param mode    The file mode
     */
    void setFileMode(const QString &mode);
    
    /**
     * Returns the file mode that should be used. The value is returned in octal.
     * 
     * @returns the file mode
     */
    QString fileMode() const;
    
    /**
     * Set if the directory mode should be used.
     * 
     * @param use     Boolean that determines whether the directory mode should be used.
     */
    void setUseDirectoryMode(bool use);
    
    /**
     * Returns if the directory mode should be used.
     * 
     * @return TRUE if the file directory should be used
     */
    bool useDirectoryMode() const;
    
    /**
     * Set the directory mode that should be used. The value must be defined in octal.
     * 
     * @param mode    The directory mode
     */
    void setDirectoryMode(const QString &mode);
    
    /**
     * Returns the directory mode that should be used. The value is returned in octal.
     * 
     * @returns the directory mode
     */
    QString directoryMode() const;
    
#if defined(Q_OS_LINUX)
    /**
     * Set if the server supports the CIFS Unix Extensions. If this setting is set,
     * the parameters that are not needed in the case of support are cleared.
     * 
     * @param supports          Boolean that determines if the server supports
     *                          the CIFS Unix extensions.
     */
    void setCifsUnixExtensionsSupport(bool support);
    
    /**
     * The server supports the CIFS Unix extensions.
     * 
     * @returns TRUE if the server supports the CIFS Unix Extensions.
     */
    bool cifsUnixExtensionsSupport() const;
    
    /**
     * Set if the filesystem port should be used
     * 
     * @param use             Boolean that determines if the filesystem port should
     *                        be used
     */
    void setUseFileSystemPort(bool use);
    
    /**
     * Returns if the filesystem port should be used.
     * 
     * @returns TRUE if the filesystem port should be used
     */
    bool useFileSystemPort() const;
    
    /**
     * Set the port that is to be used with mounting for a single share or all
     * shares of a host.
     * 
     * @param port            The file system port
     */
    void setFileSystemPort(int port);
    
    /**
     * Returns the file system port. The default value is 445.
     * 
     * @returns the file system port
     */
    int fileSystemPort() const;

    /**
     * Set if the SMB protocol version for mounting should be set.
     * 
     * @param use             Boolean that determines if the SMB protocol version
     *                        for mounting should be set
     */
    void setUseSmbMountProtocolVersion(bool use);
    
    /**
     * Returns if the SMB protocol version for mounting should be set.
     * 
     * @returns TRUE if the SMB protocol version for mounting should be set.
     */
    bool useSmbMountProtocolVersion() const;
    
    /**
     * Set the SMB protocol version for mounting.
     * 
     * @param version         The protocol version used for mounting
     */
    void setSmbMountProtocolVersion(int version);
    
    /**
     * Returns the SMB protocol version for mounting.
     * 
     * @returns the SMB protocol version
     */
    int smbMountProtocolVersion() const;
    
    /**
     * Set if the security mode should be used.
     * 
     * @param use             Boolean that determines if the security mode should
     *                        be used
     */
    void setUseSecurityMode(bool use);
    
    /**
     * Returns if the security mode should be used
     * 
     * @returns TRUE if the security mode should be used
     */
    bool useSecurityMode() const;
    
    /**
     * Set the security mode for mounting.
     *
     * @param mode            The security mode
     */
    void setSecurityMode(int mode);

    /**
     * Returns the security mode for mounting a specific share.
     *
     * @returns the security mode
     */
    int securityMode() const;
    
    /**
     * Set if the write access setting should be used.
     * 
     * @param use             Boolean that determines if the write access setting
     *                        should be used
     */
    void setUseWriteAccess(bool use);
    
    /**
     * Returns if the write access setting should be used
     * 
     * @returns TRUE if the write access setting should be used
     */
    bool useWriteAccess() const;    

    /**
     * Set the write access for either a single share or all shares of a host. 
     * 
     * @param access          The write access
     */
    void setWriteAccess(int access);
    
    /**
     * Returns the write access for the share.
     * 
     * @returns the write access
     */
    int writeAccess() const;
#endif

    /**
     * Set the profile this custom options object belongs to. The profile is 
     * meant to distinguish between several network environments, like home
     * and work.
     * 
     * @param profile         The profile name
     */
    void setProfile(const QString &profile);
    
    /**
     * Returns the name of the profile this custom options object belongs to.
     * 
     * @returns the profile name
     */
    QString profile() const;
    
    /**
     * Set whether the SMB port should be used.
     * 
     * @param use             True, if the SMB port should be used
     */
    void setUseSmbPort(bool use);
    
    /**
     * Returns whether the SMB port should be used.
     * 
     * @returns TRUE if the SMB port should be used.
     */
    bool useSmbPort() const;
    
    /**
     * Set the SMB port to use with this host or share.
     * 
     * @param port            The SMB port
     */
    void setSmbPort(int port);
    
    /**
     * Returns the SMB port. The default value is 139.
     * 
     * @returns the SMB port
     */
    int smbPort() const;
    
    /**
     * Sets the useage of Kerberos for this network item.
     * 
     * @param use               Boolean that determines the useage of Kerberos
     */
    void setUseKerberos(bool use);
    
    /**
     * Returns the usage of Kerberos for this network item.
     * 
     * @returns the usage of Kerberos
     */
    bool useKerberos() const;
    
    /**
     * This function sets the MAC address of a host. In case the options 
     * represent a share this is the MAC address of the host that shares 
     * the resource.
     * 
     * @param macAddress        The MAC address of the host
     */
    void setMACAddress(const QString &macAddress);
    
    /**
     * This function returns the MAC address of the host or an empty string if 
     * no MAC address was defined.
     *
     * @returns the MAC address of the host.
     */
    QString macAddress() const;
    
    /**
     * Set whether a magic WOL package should be send to the host that this 
     * network item represents or where this network item is located before scanning 
     * the entire network.
     * 
     * @param send              Boolean that determines if a magic WOL package
     *                          is to be sent.
     */
    void setWOLSendBeforeNetworkScan(bool send);
    
    /**
     * Send a magic WOL package to the host that this network item represents
     * or where this network item is located before scanning the entire network.
     * 
     * @returns TRUE if a magic WOL package should be send on first network
     * scan.
     */
    bool wolSendBeforeNetworkScan() const;
    
    /**
     * Set whether a magic WOL package should be send to the host that this 
     * network item represents or where this network item is located before a 
     * mount attempt.
     * 
     * @param send              Boolean that determines if a magic WOL package
     *                          is to be sent.
     */
    void setWOLSendBeforeMount(bool send);
    
    /**
     * Send a magic WOL package to the host that this network item represents
     * or where this network item is located before a mount attempt.
     * 
     * @returns TRUE if a magic WOL package should be send on first network
     * scan.
     */
    bool wolSendBeforeMount() const;
    
    /**
     * This function returns all custom options in a sorted map. The URL,
     * workgroup and IP address must be retrieved separately if needed.
     *
     * Note that all entries that are set and valid are returned here. This
     * also comprises default values (e.g. the default SMB port). If you need 
     * to check if a certain value is a custom option or not, you need to implement 
     * this check.
     *
     * @returns all valid entries.
     */
    QMap<QString,QString> customOptions() const;
    
    /**
     * Check if there are options defined
     * 
     * @returns TRUE if there are options defined and FALSE otherwise
     */
    bool hasOptions() const;
    
    /**
     * Update this custom options object. You cannot change the workgroup,
     * URL and type with this function.
     * 
     * @param options             The options that are used to update this object
     */
    void update(Smb4KCustomOptions *options);
    
  private:
    const QScopedPointer<Smb4KCustomOptionsPrivate> d;
};

#endif

