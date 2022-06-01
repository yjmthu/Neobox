#! /bin/bash
script='var allDesktops = desktops();print (allDesktops); for (i=0; i < allDesktops.length; i++){ d = allDesktops[i]; d.wallpaperPlugin = "org.kde.image"; d.currentConfigGroup = Array("Wallpaper", "org.kde.image", "General"); d.writeConfig("Image", "file://'$1'")}'
export DBUS_SESSION_BUS_ADDRESS="unix:path=/run/user/1000/bus"
qdbus org.kde.plasmashell /PlasmaShell org.kde.PlasmaShell.evaluateScript "$script"
exit 0

