/***************************************************************************
 *   Copyright (C) 2017 by A. Reinholdt <alexander.reinholdt@kdemail.net>  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.3
import QtQuick.Layouts 1.3
import QtQml.Models 2.3
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.smb4k.smb4kqmlplugin 2.0


PlasmaComponents.Page {
  id: networkBrowserPage
  anchors.fill: parent
  
  property var parentObject: 0
  
  //
  // Tool bar
  //
  PlasmaComponents.ToolBar {
    id: networkBrowserToolBar
    
    anchors {
      top: parent.top
      left: parent.left
      right: parent.right
    }
    
    tools: PlasmaComponents.ToolBarLayout {
      PlasmaComponents.ToolButton {
        id: rescanButton
        tooltip: i18n("Rescan")
        iconSource: "view-refresh"
        onClicked: {
          rescan()
        }
      }

      PlasmaComponents.ToolButton {
        id: abortButton
        tooltip: i18n("Abort")
        iconSource: "process-stop"
        onClicked: {
          abort()
        }
      }
      
      PlasmaComponents.ToolButton {
        id: upButton
        tooltip: i18n("Go one level up")
        iconSource: "go-up-symbolic"
        onClicked: {
          up()
        }
      }
      
      PlasmaComponents.ToolButton {
        id: mountDialogButton
        tooltip: i18n("Open the mount dialog")
        iconSource: "view-form"
        onClicked: {
          // FIXME: Use Plasma dialog
          iface.openMountDialog()
        }
      }
    }
  }
  
  //
  // Delegate Model (used for sorting)
  //
  DelegateModel {
    id: networkBrowserItemDelegateModel
    
    function lessThan(left, right) {
      var less = false
      
      switch (left.type) {
        case NetworkObject.Workgroup:
          less = (left.workgroupName < right.workgroupName)
          break
        case NetworkObject.Host:
          less = (left.hostName < right.hostName)
          break
        case NetworkObject.Share:
          less = (left.shareName < right.shareName)
          break
        default:
          break          
      }
      return less
    }
    
    function insertPosition(item) {
      var lower = 0
      var upper = items.count
      
      while (lower < upper) {
        var middle = Math.floor(lower + (upper - lower) / 2)
        var result = lessThan(item.model.object, items.get(middle).model.object)
        if (result) {
          upper = middle
        }
        else {
          lower = middle + 1
        }
      }
      return lower
    }
    
    function sort() {
      while (unsortedItems.count > 0) {
        var item = unsortedItems.get(0)
        var index = insertPosition(item)
        
        item.groups = "items"
        items.move(item.itemsIndex, index)
      }
    }
    
    items.includeByDefault: false
    
    groups: [ 
      DelegateModelGroup {
        id: unsortedItems
        name: "unsorted"
      
        includeByDefault: true
      
        onChanged: {
          networkBrowserItemDelegateModel.sort()
        }
      }
    ]

    filterOnGroup: "items"
    
    model: ListModel {}
    
    delegate: NetworkBrowserItemDelegate {
      id: networkBrowserItemDelegate
        
      onItemClicked: {
        networkBrowserListView.currentIndex = DelegateModel.itemsIndex
        networkItemClicked()
      }
        
      onBookmarkClicked: {
        networkBrowserListView.currentIndex = DelegateModel.itemsIndex
        iface.addBookmark(object)
      }
        
      onPreviewClicked: {
        networkBrowserListView.currentIndex = DelegateModel.itemsIndex
        iface.preview(object)
      }
        
      onConfigureClicked: {
        networkBrowserListView.currentIndex = DelegateModel.itemsIndex
        iface.openCustomOptionsDialog(object)
      }
    }
  }
      
  //
  // List view 
  //
  PlasmaExtras.ScrollArea {
    id: networkBrowserScrollArea
    
    anchors {
      top: networkBrowserToolBar.bottom
      left: parent.left
      right: parent.right
      bottom: parent.bottom
    }
    
    ListView {
      id: networkBrowserListView
      anchors.fill: parent
      model: networkBrowserItemDelegateModel
      clip: true
      focus: true
      highlightRangeMode: ListView.StrictlyEnforceRange
//       highlight: Rectangle {
//         color: theme.highlightColor
//         radius: 5
//         opacity: 0.2
//       }
    }
  }
  
  //
  // Connections
  //
  Connections {
    target: iface
    onWorkgroupsChanged: getWorkgroups()
    onHostsChanged: getHosts()
    onSharesChanged: getShares()
    onMountedSharesChanged: shareMountedOrUnmounted()
  }
  
  //
  // Initialization
  //
  Component.onCompleted: {
    if (networkBrowserListView.count == 0) {
      getWorkgroups()
    }
    else {
      // Do nothing
    }
  }  
  
  //
  // Functions
  //
  function rescan() {
    if (parentObject !== null) {
      iface.lookup(parentObject)
    }
    else {
      iface.lookup()
    }
  }
  
  function abort() {
    iface.abortClient()
    iface.abortMounter()
  }
  
  function up() {
    if (parentObject !== null) {
      
      switch (parentObject.type) {
        case NetworkObject.Workgroup:
          networkBrowserListView.currentIndex = -1
          iface.lookup()
          break
        case NetworkObject.Host:
          var object = iface.findNetworkItem(parentObject.parentUrl, parentObject.parentType)
          if (object !== null) {
            parentObject = object
            iface.lookup(object)
          }
          break
        default:
          break
      }
    }
  }
  
  function networkItemClicked() {
    if (networkBrowserListView.currentIndex != -1) {
      var object = networkBrowserItemDelegateModel.items.get(networkBrowserListView.currentIndex).model.object
      
      if (object.type == NetworkObject.Share) {
        if (!object.isPrinter) {
          iface.mountShare(object)
        }
        else {
          iface.print(object)
        }
      }
      else {
        parentObject = object
        iface.lookup(object)
      }
    }
    else {
      // Do nothing
    }
  }
  
  function getWorkgroups() {
    while (networkBrowserItemDelegateModel.model.count != 0) {
      networkBrowserItemDelegateModel.model.remove(0)
    }
    
    if (iface.workgroups.length != 0) {
      for (var i = 0; i < iface.workgroups.length; i++) {
        networkBrowserItemDelegateModel.model.append({"object": iface.workgroups[i]})
      }
    }
    else {
      // Do nothing
    }
    
    networkBrowserListView.currentIndex = 0
  }
  
  function getHosts() {
    var workgroupName = parentObject.workgroupName
    
    while (networkBrowserItemDelegateModel.model.count != 0) {
      networkBrowserItemDelegateModel.model.remove(0)
    }
    
    if (iface.hosts.length != 0) {
      for (var i = 0; i < iface.hosts.length; i++) {
        if (workgroupName.length != 0 && workgroupName == iface.hosts[i].workgroupName) {
          networkBrowserItemDelegateModel.model.append({"object": iface.hosts[i]})
        }
        else {
          // Do nothing
        }
      }
    }
    else {
      // Do nothing
    }
    
    networkBrowserListView.currentIndex = 0
  }
  
  function getShares() {
    var hostName = parentObject.hostName
    
    while (networkBrowserItemDelegateModel.model.count != 0) {
      networkBrowserItemDelegateModel.model.remove(0)
    }
    
    if (iface.shares.length != 0) {
      for (var i = 0; i < iface.shares.length; i++) {
        if (hostName.length != 0 && hostName == iface.shares[i].hostName) {
          networkBrowserItemDelegateModel.model.append({"object": iface.shares[i]})
        }
        else {
          // Do nothing
        }
      }
    }
    else {
      // Do nothing
    }
    
    networkBrowserListView.currentIndex = 0
  }
  
  function shareMountedOrUnmounted() {
    for (var i = 0; i < networkBrowserItemDelegateModel.model.count; i++) {
      var object = networkBrowserItemDelegateModel.model.get(i).object
      
      if (object !== null && object.type == NetworkObject.Share) {
        var mountedShare = iface.findMountedShare(object.url, false)
        
        if (mountedShare !== null) {
          object.isMounted = mountedShare.isMounted
        }
        else {
          object.isMounted = false
        }
        
        networkBrowserItemDelegateModel.model.set(i, {"object": object})
      }
      else {
        // Do nothing
      }
    }
  }
}
