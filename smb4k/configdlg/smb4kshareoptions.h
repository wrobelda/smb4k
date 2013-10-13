/***************************************************************************
    smb4kshareoptions  -  The configuration page for the settings of
    Smb4K regarding share management
                             -------------------
    begin                : Sa Nov 15 2003
    copyright            : (C) 2003-2010 by Alexander Reinholdt
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

#ifndef SMB4KSHAREOPTIONS_H
#define SMB4KSHAREOPTIONS_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// Qt includes
#include <QWidget>

/**
 * This is the configuration tab for the settings that are
 * used to manage the mounted shares.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class Smb4KShareOptions : public QWidget
{
  Q_OBJECT

  public:
    /**
     * The constructor.
     *
     * @param parent          The parent of this widget
     */
    Smb4KShareOptions( QWidget *parent = 0 );

    /**
     * The destructor.
     */
    ~Smb4KShareOptions();
};
#endif