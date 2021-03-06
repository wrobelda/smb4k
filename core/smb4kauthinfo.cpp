/***************************************************************************
    This class provides a container for the authentication data.
                             -------------------
    begin                : Sa Feb 28 2004
    copyright            : (C) 2004-2019 by Alexander Reinholdt
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

// application specific includes
#include "smb4kauthinfo.h"
#include "smb4khost.h"
#include "smb4kshare.h"

// Qt includes
#include <QHostAddress>
#include <QDebug>

// KDE includes
#include <KI18n/KLocalizedString>


class Smb4KAuthInfoPrivate
{
  public:
    QUrl url;
    QString workgroup;
    NetworkItem type;
    bool homesShare;
    QHostAddress ip;
};


Smb4KAuthInfo::Smb4KAuthInfo(Smb4KBasicNetworkItem* item)
: d(new Smb4KAuthInfoPrivate)
{
  d->type = item->type();

  switch (d->type)
  {
    case Host:
    {
      Smb4KHost *host = static_cast<Smb4KHost *>(item);
      
      if (host)
      {
        d->url = host->url();
        d->workgroup = host->workgroupName();
        d->homesShare = false;
        d->ip.setAddress(host->ipAddress());
      }
      
      break;
    }
    case Share:
    {
      Smb4KShare *share = static_cast<Smb4KShare *>(item);
      
      if (share)
      {
        d->url = !share->isHomesShare() ? share->homeUrl() : share->url();
        d->workgroup = share->workgroupName();
        d->homesShare = share->isHomesShare();
        d->ip.setAddress(share->hostIpAddress());
      }
      
      break;
    }
    default:
    {
      break;
    }
  }
}


Smb4KAuthInfo::Smb4KAuthInfo()
: d(new Smb4KAuthInfoPrivate)
{
  d->type = UnknownNetworkItem;
  d->homesShare = false;
  d->url.clear();
  d->workgroup.clear();
  d->ip.clear();
}


Smb4KAuthInfo::Smb4KAuthInfo(const Smb4KAuthInfo &i)
: d(new Smb4KAuthInfoPrivate)
{
  *d = *i.d;
}


Smb4KAuthInfo::~Smb4KAuthInfo()
{
}


void Smb4KAuthInfo::setWorkgroupName(const QString &workgroup)
{
  d->workgroup = workgroup;
}


QString Smb4KAuthInfo::workgroupName() const
{
  return d->workgroup;
}


void Smb4KAuthInfo::setUrl(const QUrl &url)
{
  d->url = url;
  d->url.setScheme("smb");

  // Set the type.
  if (!d->url.path().isEmpty() && d->url.path().length() > 1 && !d->url.path().endsWith('/'))
  {
    d->type = Share;
  }
  else
  {
    d->type = Host;
  }
  
  // Determine whether this is a homes share.
  d->homesShare = (QString::compare(d->url.path().remove('/'), "homes", Qt::CaseSensitive) == 0);
}


void Smb4KAuthInfo::setUrl(const QString &url)
{
  QUrl tempUrl(url, QUrl::TolerantMode);
  tempUrl.setScheme("smb");
  setUrl(tempUrl);
}



QUrl Smb4KAuthInfo::url() const
{
  return d->url;
}


QString Smb4KAuthInfo::hostName() const
{
  return d->url.host().toUpper();
}



QString Smb4KAuthInfo::shareName() const
{
  if (d->url.path().startsWith('/'))
  {
    return d->url.path().remove(0, 1);
  }

  return d->url.path();
}


void Smb4KAuthInfo::setUserName(const QString &username)
{
  d->url.setUserName(username);

  if (d->homesShare)
  {
    d->url.setPath(username);
  }
}


QString Smb4KAuthInfo::userName() const
{
  return d->url.userName();
}


void Smb4KAuthInfo::setPassword(const QString &passwd)
{
  d->url.setPassword(passwd);
}


QString Smb4KAuthInfo::password() const
{
  return d->url.password();
}


Smb4KGlobal::NetworkItem Smb4KAuthInfo::type() const
{
  return d->type;
}


bool Smb4KAuthInfo::isHomesShare() const
{
  return d->homesShare;
}


void Smb4KAuthInfo::setIpAddress(const QString &ip)
{
  d->ip.setAddress(ip);
}


QString Smb4KAuthInfo::ipAddress() const
{
  return d->ip.toString();
}


QString Smb4KAuthInfo::displayString() const
{
  return i18n("%1 on %2", shareName(), hostName());
}


