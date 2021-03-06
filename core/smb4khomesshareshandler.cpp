/***************************************************************************
    This class handles the homes shares
                             -------------------
    begin                : Do Aug 10 2006
    copyright            : (C) 2006-2019 by Alexander Reinholdt
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
 *   MA 02110-1335 USA                                                     *
 ***************************************************************************/

// application specific includes
#include "smb4khomesshareshandler.h"
#include "smb4khomesshareshandler_p.h"
#include "smb4kshare.h"
#include "smb4ksettings.h"
#include "smb4kauthinfo.h"
#include "smb4knotification.h"
#include "smb4kprofilemanager.h"

// Qt includes
#include <QFile>
#include <QTextCodec>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include <QApplication>
#include <QPointer>

// KDE includes
#define TRANSLATION_DOMAIN "smb4k-core"
#include <KI18n/KLocalizedString>

Q_GLOBAL_STATIC(Smb4KHomesSharesHandlerStatic, p);


Smb4KHomesSharesHandler::Smb4KHomesSharesHandler(QObject *parent)
: QObject(parent), d(new Smb4KHomesSharesHandlerPrivate)
{
  // First we need the directory.
  QString path = dataLocation();
  
  QDir dir;
  
  if (!dir.exists(path))
  {
    dir.mkpath(path);
  }
  
  d->homesUsers = readUserNames(false);
  
  // 
  // Connections
  // 
  connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), this, SLOT(slotAboutToQuit()));
  connect(Smb4KProfileManager::self(), SIGNAL(activeProfileChanged(QString)), this, SLOT(slotActiveProfileChanged(QString)));
}


Smb4KHomesSharesHandler::~Smb4KHomesSharesHandler()
{
  while (!d->homesUsers.isEmpty())
  {
    delete d->homesUsers.takeFirst();
  }
}


Smb4KHomesSharesHandler *Smb4KHomesSharesHandler::self()
{
  return &p->instance;
}


bool Smb4KHomesSharesHandler::specifyUser(const SharePtr &share, bool overwrite)
{
  Q_ASSERT(share);
  bool success = false;
  
  // Avoid that the dialog is opened although the homes
  // user name has already been defined.
  if (share->isHomesShare() && (share->homeUrl().isEmpty() || overwrite))
  {
    QStringList users = findHomesUsers(share);
    
    QPointer<Smb4KHomesUserDialog> dlg = new Smb4KHomesUserDialog(share, QApplication::activeWindow());
    dlg->setUserNames(users);
    
    if (dlg->exec() == QDialog::Accepted)
    {
      QString login = dlg->login();
      users = dlg->userNames();
      addHomesUsers(share, users);
      
      if (!login.isEmpty())
      {
        // If the login names do not match, clear the password.
        if (!share->login().isEmpty() && QString::compare(share->login(), login) != 0)
        {
          share->setPassword(QString());
        }
        
        // Set the login name.
        share->setLogin(login);        
        success = true;
      }
      
      writeUserNames(d->homesUsers);
    }
    
    delete dlg;
  }
  else
  {
    // The user name has already been set.
    success = true;
  }
  
  return success;
}


QStringList Smb4KHomesSharesHandler::homesUsers(const SharePtr &share)
{
  Q_ASSERT(share);
  QStringList users = findHomesUsers(share);
  return users;
}


const QList<Smb4KHomesUsers *> Smb4KHomesSharesHandler::readUserNames(bool allUsers)
{
  QList<Smb4KHomesUsers *> list;
  
  // Locate the XML file.
  QFile xmlFile(dataLocation()+QDir::separator()+"homes_shares.xml");

  if (xmlFile.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    QXmlStreamReader xmlReader(&xmlFile);

    while (!xmlReader.atEnd())
    {
      xmlReader.readNext();

      if (xmlReader.isStartElement())
      {
        if (xmlReader.name() == "homes_shares" && xmlReader.attributes().value("version") != "1.0")
        {
          xmlReader.raiseError(i18n("%1 is not a version 1.0 file.", xmlFile.fileName()));
          break;
        }
        else
        {
          if (xmlReader.name() == "homes")
          {
            QString profile = xmlReader.attributes().value("profile").toString();
            
            // FIXME: Remove the last check in the if statement with Smb4K > 2.0.
            // It was introduced for migration, because the default profile 
            // (e.g. use of no profiles) was not empty but named "Default"...
            if (allUsers || QString::compare(profile, Smb4KProfileManager::self()->activeProfile(), Qt::CaseSensitive) == 0 ||
                (!Smb4KProfileManager::self()->useProfiles() && QString::compare(profile, "Default", Qt::CaseSensitive) == 0))
            {
              Smb4KHomesUsers *users = new Smb4KHomesUsers();
              users->setProfile(profile);
              users->setShareName(xmlReader.name().toString());
              
              while (!(xmlReader.isEndElement() && xmlReader.name() == "homes"))
              {
                xmlReader.readNext();

                if (xmlReader.isStartElement())
                {
                  if (xmlReader.name() == "host")
                  {
                    users->setHostName(xmlReader.readElementText());
                  }
                  else if (xmlReader.name() == "workgroup")
                  {
                    users->setWorkgroupName(xmlReader.readElementText());
                  }
                  else if (xmlReader.name() == "ip")
                  {
                    users->setHostIP(xmlReader.readElementText());
                  }
                  else if (xmlReader.name() == "users")
                  {
                    QStringList u;
                    
                    while (!(xmlReader.isEndElement() && xmlReader.name() == "users"))
                    {
                      xmlReader.readNext();

                      if (xmlReader.isStartElement() && xmlReader.name() == "user")
                      {
                        u << xmlReader.readElementText();
                      }
                    }
                    
                    users->setUsers(u);
                  }

                  continue;
                }
                else
                {
                  continue;
                }
              }
              
              list << users;   
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
      }
      else
      {
        continue;
      }
    }

    xmlFile.close();

    if (xmlReader.hasError())
    {
      Smb4KNotification::readingFileFailed(xmlFile, xmlReader.errorString());
    }
  }
  else
  {
    if (xmlFile.exists())
    {
      Smb4KNotification::openingFileFailed(xmlFile);
    }
  }
  
  return list;
}


void Smb4KHomesSharesHandler::writeUserNames(const QList<Smb4KHomesUsers *> &list, bool listOnly)
{
  QList<Smb4KHomesUsers *> allUsers;
  
  if (!listOnly)
  {  
    // First read all entries. Then remove all, that belong to
    // the currently active profile.
    allUsers = readUserNames(true);
    
    QMutableListIterator<Smb4KHomesUsers *> it(allUsers);
    
    while (it.hasNext())
    {
      Smb4KHomesUsers *users = it.next();
      
      if (QString::compare(users->profile(), Smb4KProfileManager::self()->activeProfile()) == 0)
      {
        it.remove();
      }
    }
  }
  
  for (Smb4KHomesUsers *users : list)
  {
    allUsers << new Smb4KHomesUsers(*users);
  }
  
  QFile xmlFile(dataLocation()+QDir::separator()+"homes_shares.xml");

  if (!allUsers.isEmpty())
  {
    if (xmlFile.open(QIODevice::WriteOnly|QIODevice::Text))
    {
      QXmlStreamWriter xmlWriter(&xmlFile);
      xmlWriter.setAutoFormatting(true);
      xmlWriter.writeStartDocument();
      xmlWriter.writeStartElement("homes_shares");
      xmlWriter.writeAttribute("version", "1.0");

      for (Smb4KHomesUsers *users : allUsers)
      {
        xmlWriter.writeStartElement("homes");
        
        // FIXME: Remove this block with Smb4K > 2.0 and use the commented line below. 
        // This block was introduced for migration, because the default profile 
        // (i.e. use of no profiles) was not empty but named "Default"...
        if (!Smb4KProfileManager::self()->useProfiles())
        {
          xmlWriter.writeAttribute("profile", Smb4KSettings::self()->activeProfile());
        }
        else
        {
          xmlWriter.writeAttribute("profile", users->profile());
        }
        // xmlWriter.writeAttribute("profile", users->profile());
        xmlWriter.writeTextElement("host", users->hostName());
        xmlWriter.writeTextElement("workgroup", users->workgroupName());
        xmlWriter.writeTextElement("ip", users->hostIP());
        xmlWriter.writeStartElement("users");

        for (const QString &user : users->users())
        {
          xmlWriter.writeTextElement("user", user);
        }

        xmlWriter.writeEndElement();
        xmlWriter.writeEndElement();
      }

      xmlWriter.writeEndDocument();
      xmlFile.close();
    }
    else
    {
      Smb4KNotification::openingFileFailed(xmlFile);
    }
  }
  else
  {
    xmlFile.remove();
  }
  
  while (!allUsers.isEmpty())
  {
    delete allUsers.takeFirst();
  }
}


const QStringList Smb4KHomesSharesHandler::findHomesUsers(const SharePtr &share)
{
  Q_ASSERT(share);
  
  QStringList users;
 
  if (!d->homesUsers.isEmpty())
  {
    for (int i = 0; i < d->homesUsers.size(); ++i)
    {
      if (QString::compare(share->hostName(), d->homesUsers.at(i)->hostName(), Qt::CaseInsensitive) == 0 &&
           QString::compare(share->shareName(), d->homesUsers.at(i)->shareName(), Qt::CaseInsensitive) == 0 &&
           ((d->homesUsers.at(i)->workgroupName().isEmpty() || share->workgroupName().isEmpty()) ||
           QString::compare(share->workgroupName(), d->homesUsers.at(i)->workgroupName(), Qt::CaseInsensitive) == 0))
      {
        users = d->homesUsers.at(i)->users();
        break;
      }
      else
      {
        continue;
      }
    }
  }
  
  return users;
}


void Smb4KHomesSharesHandler::addHomesUsers(const SharePtr &share, const QStringList &users)
{
  Q_ASSERT(share);
  
  bool found = false;
  
  if (!d->homesUsers.isEmpty())
  {
    for (int i = 0; i < d->homesUsers.size(); ++i)
    {
      if (QString::compare(share->hostName(), d->homesUsers.at(i)->hostName(), Qt::CaseInsensitive) == 0 &&
          QString::compare(share->shareName(), d->homesUsers.at(i)->shareName(), Qt::CaseInsensitive) == 0 &&
          ((d->homesUsers.at(i)->workgroupName().isEmpty() || share->workgroupName().isEmpty()) ||
          QString::compare(share->workgroupName(), d->homesUsers.at(i)->workgroupName(), Qt::CaseInsensitive) == 0))
      {
        d->homesUsers[i]->setUsers(users);
        found = true;
        break;
      }
      else
      {
        continue;
      }
    }
  }
  
  if (!found)
  {
    Smb4KHomesUsers *u = new Smb4KHomesUsers(share, users);
    u->setProfile(Smb4KProfileManager::self()->activeProfile());
    d->homesUsers << u;
  }
}


void Smb4KHomesSharesHandler::migrateProfile(const QString& from, const QString& to)
{
  // Read all entries for later conversion.
  QList<Smb4KHomesUsers *> allUsers = readUserNames(true);
  
  // Replace the old profile name with the new one.
  for (int i = 0; i < allUsers.size(); ++i)
  {
    if (QString::compare(allUsers.at(i)->profile(), from, Qt::CaseSensitive) == 0)
    {
      allUsers[i]->setProfile(to);
    }
  }
  
  // Write the new list to the file.
  writeUserNames(allUsers, true);
  
  // Profile settings changed, so invoke the slot.
  slotActiveProfileChanged(Smb4KProfileManager::self()->activeProfile());
  
  // Clear the temporary lists of bookmarks and groups.
  while (!allUsers.isEmpty())
  {
    delete allUsers.takeFirst();
  }
}


void Smb4KHomesSharesHandler::removeProfile(const QString& name)
{
  // Read all entries for later removal.
  QList<Smb4KHomesUsers *> allUsers = readUserNames(true);
  QMutableListIterator<Smb4KHomesUsers *> it(allUsers);
  
  while (it.hasNext())
  {
    Smb4KHomesUsers *user = it.next();
    
    if (QString::compare(user->profile(), name, Qt::CaseSensitive) == 0)
    {
      it.remove();
    }
  }
  
  // Write the new list to the file.
  writeUserNames(allUsers, true);
  
  // Profile settings changed, so invoke the slot.
  slotActiveProfileChanged(Smb4KProfileManager::self()->activeProfile());
  
  // Clear the temporary list of homes users.
  while (!allUsers.isEmpty())
  {
    delete allUsers.takeFirst();
  }
}


/////////////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KHomesSharesHandler::slotAboutToQuit()
{
  writeUserNames(d->homesUsers);
}


void Smb4KHomesSharesHandler::slotActiveProfileChanged(const QString& /*activeProfile*/)
{
  // Clear the list of homes users.
  while (!d->homesUsers.isEmpty())
  {
    delete d->homesUsers.takeFirst();
  }
  
  // Reload the list of homes users.
  d->homesUsers = readUserNames(false);
}

