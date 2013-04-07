/***************************************************************************
    smb4klaptopsupportoptionspage  -  The configuration page for the laptop
    support of Smb4K
                             -------------------
    begin                : Mi Sep 17 2008
    copyright            : (C) 2008-2013 by Alexander Reinholdt
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

#ifndef SMB4KLAPTOPSUPPORTOPTIONSPAGE_H
#define SMB4KLAPTOPSUPPORTOPTIONSPAGE_H

// Qt includes
#include <QWidget>

/**
 * This class provides the configuration page for the laptop
 * support in Smb4K.
 *
 * @author Alexander Reinholdt <dustpuppy@users.berlios.de>
 */

class Smb4KLaptopSupportOptionsPage : public QWidget
{
  Q_OBJECT

  public:
    /**
     * The constructor
     */
    explicit Smb4KLaptopSupportOptionsPage( QWidget *parent = 0 );

    /**
     * The destructor
     */
    ~Smb4KLaptopSupportOptionsPage();
};

#endif