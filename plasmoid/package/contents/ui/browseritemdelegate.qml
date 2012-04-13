/***************************************************************************
    browseritemdelegate.qml - The item delegate for the browser in Smb4K's
    plasmoid
                             -------------------
    begin                : Do Apr 12 2012
    copyright            : (C) 2012 by Alexander Reinholdt
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

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1

//
// Delegate for the list items in the browser
//
PlasmaComponents.ListItem {
  id: delegate
    
  signal itemClicked()
  signal upClicked()
    
  width: browserListView.width
  height: 40

  Row {
    spacing: 5
    Column {
      anchors.verticalCenter: parent.verticalCenter
      QIconItem {
        id: delegateItemIcon
        icon: itemIcon
        width: 22
        height: 22
        MouseArea {
          anchors.fill: parent
          onClicked: {
            delegate.itemClicked()
          }
        }
      }
    }
    Column {
      anchors.verticalCenter: parent.verticalCenter
      Text { 
        text: itemName 
        clip: true
        MouseArea {
          anchors.fill: parent
          onClicked: {
            delegate.itemClicked()
          }
        }
      }
      Text { 
        text: "<font size=\"-1\">"+itemComment+"</font>" 
        clip: true
        MouseArea {
          anchors.fill: parent
          onClicked: {
            delegate.itemClicked()
          }
        }
      }
    }
  }
  QIconItem {
    anchors.verticalCenter: parent.verticalCenter
    anchors.right: parent.right
    anchors.rightMargin: 10
    anchors.leftMargin: 10
    icon: "go-up"
    height: 16
    width: 16
    opacity: 0.2
    visible: itemType > 1 ? true : false
    enabled: itemType > 1 ? true : false
    MouseArea {
      anchors.fill: parent
      hoverEnabled: true
      onEntered: {
        parent.opacity = 1.0
      }
      onExited: {
        parent.opacity = 0.2
      }
      onClicked: {
        delegate.upClicked()
      }
    }        
  }
}