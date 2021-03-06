/***************************************************************************
    This is the new synchronizer of Smb4K.
                             -------------------
    begin                : Fr Feb 04 2011
    copyright            : (C) 2011-2019 by Alexander Reinholdt
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
#include "smb4ksynchronizer.h"
#include "smb4ksynchronizer_p.h"
#include "smb4knotification.h"
#include "smb4kglobal.h"
#include "smb4kshare.h"

// Qt includes
#include <QTimer>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QApplication>

// KDE includes
#include <KCoreAddons/KShell>

using namespace Smb4KGlobal;

Q_GLOBAL_STATIC(Smb4KSynchronizerStatic, p);


Smb4KSynchronizer::Smb4KSynchronizer(QObject *parent)
: KCompositeJob(parent), d(new Smb4KSynchronizerPrivate)
{
  setAutoDelete(false);
  connect(QCoreApplication::instance(), SIGNAL(aboutToQuit()), SLOT(slotAboutToQuit()));
}


Smb4KSynchronizer::~Smb4KSynchronizer()
{
}


Smb4KSynchronizer *Smb4KSynchronizer::self()
{
  return &p->instance;
}


void Smb4KSynchronizer::synchronize(const SharePtr &share)
{
  if (!isRunning(share))
  {
    // Create a new job, add it to the subjobs and register it
    // with the job tracker.
    Smb4KSyncJob *job = new Smb4KSyncJob(this);
    job->setObjectName(QString("SyncJob_%1").arg(share->canonicalPath()));
    job->setupSynchronization(share);
    
    connect(job, SIGNAL(result(KJob*)), SLOT(slotJobFinished(KJob*)));
    connect(job, SIGNAL(aboutToStart(QString)), SIGNAL(aboutToStart(QString)));
    connect(job, SIGNAL(finished(QString)), SIGNAL(finished(QString)));

    addSubjob(job);
    
    job->start();
  }
}


bool Smb4KSynchronizer::isRunning()
{
  return hasSubjobs();
}


bool Smb4KSynchronizer::isRunning(const SharePtr &share)
{
  bool running = false;

  for (int i = 0; i < subjobs().size(); ++i)
  {
    if (QString::compare(QString("SyncJob_%1").arg(share->canonicalPath()), subjobs().at(i)->objectName()) == 0)
    {
      running = true;
      break;
    }
    else
    {
      continue;
    }
  }
  
  return running;
}


void Smb4KSynchronizer::abort(const SharePtr &share)
{
  if (share && !share.isNull())
  {
    for (KJob *job : subjobs())
    {
      if (QString("SyncJob_%1").arg(share->canonicalPath()) == job->objectName())
      {
        job->kill(KJob::EmitResult);
        break;
      }
    }
  }
  else
  {
    QListIterator<KJob *> it(subjobs());
      
    while (it.hasNext())
    {
      it.next()->kill(KJob::EmitResult);
    }
  }
}


void Smb4KSynchronizer::start()
{
  QTimer::singleShot(0, this, SLOT(slotStartJobs()));
}

/////////////////////////////////////////////////////////////////////////////
//   SLOT IMPLEMENTATIONS
/////////////////////////////////////////////////////////////////////////////

void Smb4KSynchronizer::slotStartJobs()
{
  // FIXME: Not implemented yet. I do not see a use case at the moment.
}


void Smb4KSynchronizer::slotJobFinished(KJob *job)
{
  // Remove the job.
  removeSubjob(job);
}


void Smb4KSynchronizer::slotAboutToQuit()
{
  abort();
}

