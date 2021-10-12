# Fnotifystat

Fnotifystat is a program that dumps the file system activity in a given period of time.

fnotifystat command line options:

* -c dump cumulative totals over all time rather than totals over each sample period.
* -d strip full directory path off the filenames.
* -D order stats by unique device.
* -h show help.
* -i specify pathnames to include on path events.
* -I order stats by unique device and inode.
* -n no stats, just -v verbose mode only.
* -p only monitor files touched by process with pid PID.
* -P sort stats by pid first, then by totals and filename.
* -s disable scaling of file counts.
* -t only display the top busiest files.
* -T show timestamp.
* -v verbose mode, dump all file activity.
* -x specify pathnames to exclude on path events. 


# Examples:

```
sudo fnotifystat  -c -i /proc 15 1  
Total   Open  Close   Read  Write   PID  Process         Pathname
 36.0    9.0    9.0   18.0    0.0   9007 firefox         /proc/9007/mountinfo
 15.0    0.0    0.0   15.0    0.0  10192 vmstat          /proc/meminfo
 15.0    0.0    0.0   15.0    0.0  10192 vmstat          /proc/stat
 15.0    0.0    0.0   15.0    0.0  10192 vmstat          /proc/vmstat
  8.0    2.0    2.0    4.0    0.0   2395 unity-settings- /proc/2395/mounts
  8.0    2.0    2.0    4.0    0.0   1233 irqbalance      /proc/interrupts
  8.0    2.0    2.0    4.0    0.0   1233 irqbalance      /proc/stat
  6.0    2.0    2.0    2.0    0.0   1233 irqbalance      /proc/irq/1/smp_affinity
  6.0    2.0    2.0    2.0    0.0   1233 irqbalance      /proc/irq/12/smp_affinity
  6.0    2.0    2.0    2.0    0.0   1233 irqbalance      /proc/irq/26/smp_affinity
  6.0    2.0    2.0    2.0    0.0   1233 irqbalance      /proc/irq/29/smp_affinity
  6.0    2.0    2.0    2.0    0.0   1233 irqbalance      /proc/irq/30/smp_affinity
  6.0    2.0    2.0    2.0    0.0   1233 irqbalance      /proc/irq/31/smp_affinity
  6.0    2.0    2.0    2.0    0.0   1233 irqbalance      /proc/irq/8/smp_affinity
  6.0    2.0    2.0    2.0    0.0   1233 irqbalance      /proc/irq/9/smp_affinity
```

```
sudo fnotifystat -n -i /sys,/proc 1 5
12/01/15 17:21:41   0.0  (O---)  1233 irqbalance      /proc/interrupts
12/01/15 17:21:41   0.0  (--R-)  1233 irqbalance      /proc/interrupts
12/01/15 17:21:41   0.0  (-CR-)  1233 irqbalance      /proc/interrupts
12/01/15 17:21:41   0.0  (O---)  1233 irqbalance      /proc/stat
12/01/15 17:21:41   0.0  (--R-)  1233 irqbalance      /proc/stat
12/01/15 17:21:41   0.0  (-CR-)  1233 irqbalance      /proc/stat
12/01/15 17:21:41   0.0  (O-R-)  1233 irqbalance      /proc/irq/29/smp_affinity
12/01/15 17:21:41   0.0  (-C--)  1233 irqbalance      /proc/irq/29/smp_affinity
12/01/15 17:21:41   0.0  (OCR-)  1233 irqbalance      /proc/irq/26/smp_affinity
12/01/15 17:21:41   0.0  (OCR-)  1233 irqbalance      /proc/irq/30/smp_affinity
12/01/15 17:21:41   0.0  (OCR-)  1233 irqbalance      /proc/irq/31/smp_affinity
12/01/15 17:21:41   0.0  (O---)  1233 irqbalance      /proc/irq/1/smp_affinity
12/01/15 17:21:41   0.0  (-CR-)  1233 irqbalance      /proc/irq/1/smp_affinity
12/01/15 17:21:41   0.0  (OCR-)  1233 irqbalance      /proc/irq/8/smp_affinity
12/01/15 17:21:41   0.0  (OCR-)  1233 irqbalance      /proc/irq/9/smp_affinity
12/01/15 17:21:41   0.0  (OCR-)  1233 irqbalance      /proc/irq/12/smp_affinity
12/01/15 17:21:42   0.0  (OCR-)  1178 thermald        /sys/devices/virtual/thermal/thermal_zone1/temp
```

```
sudo fnotifystat -p firefox,rhythmbox  -d 60 1
Total   Open  Close   Read  Write  PID  Process         Pathname
 32.9    0.2    0.2   32.5    0.0  24558 rhythmbox       green_valley_beyond_the_horizon_part_i.mp3
 10.7    0.0    0.0   10.7    0.0  24558 rhythmbox       the_eternal_song_of_the_ocean_waves_part_ii.mp3
  9.4    4.5    4.9    0.0    0.0  24558 rhythmbox       localtime
  2.4    0.0    0.0    2.4    0.0  13278 rhythmbox       maps
  1.4    0.1    0.1    1.2    0.0  13278 rhythmbox       locale.alias
  1.1    0.6    0.6    0.0    0.0  13278 rhythmbox       ld.so.cache
  0.9    0.1    0.1    0.6    0.0  13278 rhythmbox       cpuinfo
  0.8    0.2    0.3    0.3    0.0  24558 rhythmbox       Last.fm-symbolic.svg
  0.7    0.0    0.0    0.7    0.0   9007 firefox         cert8.db
  0.6    0.0    0.0    0.5    0.0  13278 rhythmbox       WebpageIcons.db
  0.6    0.1    0.1    0.0    0.4  13278 rhythmbox       gtbplugin.log
  0.4    0.1    0.1    0.0    0.3  13278 rhythmbox       o1dplugin.log
  0.4    0.1    0.1    0.1    0.0  13278 rhythmbox       meminfo
  0.4    0.1    0.1    0.1    0.0  13278 rhythmbox       cpuinfo_max_freq
  0.4    0.1    0.1    0.1    0.0  13278 rhythmbox       libtotem-plparser-mini.so.18.1.0
  0.4    0.0    0.0    0.3    0.0  13278 rhythmbox       locale.dir
  0.3    0.0    0.0    0.3    0.0  13278 rhythmbox       index.theme
  0.3    0.1    0.1    0.1    0.0   9007 firefox         mountinfo
  0.3    0.1    0.1    0.1    0.0  13278 rhythmbox       plugins
  0.3    0.1    0.1    0.1    0.0  13278 rhythmbox       plugins
  0.3    0.0    0.0    0.2    0.0  13278 rhythmbox       loaders.cache
  0.2    0.1    0.1    0.1    0.0  24558 rhythmbox       defaults.list
  0.2    0.1    0.1    0.1    0.0  13278 rhythmbox       locale.alias
  0.2    0.1    0.1    0.1    0.0  24558 rhythmbox       machine-id
  0.2    0.1    0.1    0.1    0.0  13278 rhythmbox       .Xauthority
  0.2    0.0    0.0    0.1    0.0  13278 rhythmbox       gtk-widgets.css
  0.2    0.0    0.0    0.1    0.0  24558 rhythmbox       Symphonic Landscapes
  0.2    0.0    0.0    0.1    0.0  13278 rhythmbox       index.theme
  0.1    0.0    0.0    0.0    0.1   9007 firefox         1993CA2B36AC7E9152A509CF5052647AA65D23AC
  0.1    0.0    0.0    0.1    0.0  13278 rhythmbox       enchant
  0.1    0.0    0.0    0.1    0.0  13278 rhythmbox       .icons
  0.1    0.0    0.0    0.1    0.0  13278 rhythmbox       apps
  0.1    0.0    0.0    0.1    0.0  13278 rhythmbox       apps
  0.1    0.0    0.0    0.1    0.0  13278 rhythmbox       apps
  0.1    0.0    0.0    0.0    0.1   9007 firefox         cookies.sqlite-wal
  0.1    0.0    0.0    0.1    0.0  13278 rhythmbox       ld-2.19.so
  0.1    0.0    0.0    0.1    0.0  13278 rhythmbox       fd
  0.1    0.0    0.0    0.1    0.0  24558 rhythmbox       shm
  0.1    0.0    0.0    0.0    0.1  13278 rhythmbox       user
  0.1    0.1    0.0    0.0    0.1  24558 rhythmbox       user
  0.1    0.0    0.1    0.0    0.1  24558 rhythmbox       user (deleted)
  0.1    0.0    0.0    0.1    0.0  13278 rhythmbox       yelp
  0.1    0.0    0.0    0.1    0.0  13278 rhythmbox       enchant
  0.1    0.0    0.0    0.1    0.0  13278 rhythmbox       modules
  0.1    0.0    0.0    0.1    0.0  13278 rhythmbox       XLC_LOCALE
  0.1    0.0    0.0    0.1    0.0  24558 rhythmbox       applications
  0.1    0.0    0.0    0.1    0.0  13278 rhythmbox       icons
  0.1    0.0    0.0    0.1    0.0  13278 rhythmbox       index.theme
  0.1    0.1    0.1    0.0    0.0  13278 rhythmbox       icon-theme.cache
  0.1    0.0    0.0    0.1    0.0  13278 rhythmbox       pixmaps
  0.1    0.0    0.0    0.1    0.0  13278 rhythmbox       gtk-widgets-borders.css
  0.1    0.0    0.0    0.1    0.0  13278 rhythmbox       icons
  0.1    0.0    0.0    0.1    0.0  13278 rhythmbox       status
  0.1    0.0    0.0    0.1    0.0  13278 rhythmbox       status
  0.1    0.0    0.0    0.1    0.0  13278 rhythmbox       status
  0.1    0.0    0.0    0.1    0.0  13278 rhythmbox       icons
  0.1    0.0    0.0    0.1    0.0  13278 rhythmbox       status
  0.1    0.0    0.0    0.1    0.0  13278 rhythmbox       actions
  0.1    0.0    0.0    0.1    0.0  13278 rhythmbox       status
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       settings.ini
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       nsswitch.conf
  0.1    0.0    0.0    0.0    0.0  24558 rhythmbox       client.conf
  0.1    0.0    0.0    0.0    0.0  24558 rhythmbox       .Xauthority
  0.1    0.0    0.0    0.0    0.0  24558 rhythmbox       cookie
  0.1    0.0    0.0    0.0    0.0  24558 rhythmbox       applications
  0.1    0.0    0.0    0.0    0.0  24558 rhythmbox       mimeapps.list
  0.1    0.0    0.0    0.0    0.0  24558 rhythmbox       wine
  0.1    0.0    0.0    0.0    0.0  24558 rhythmbox       Programs
  0.1    0.0    0.0    0.0    0.0  24558 rhythmbox       DHMB
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       icons
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       apps
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       apps
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libbz2.so.1.0.4
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libc-2.19.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libdbus-1.so.3.8.9
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libdl-2.19.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libexpat.so.1.6.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libgcc_s.so.1
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libgcrypt.so.11.8.3
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libgcrypt.so.20.0.2
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libglib-2.0.so.0.4302.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libgpg-error.so.0.13.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       liblzma.so.5.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libm-2.19.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libnsl-2.19.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libnss_compat-2.19.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libnss_files-2.19.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libnss_nis-2.19.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libpcre.so.3.13.1
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libpng12.so.0.51.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libpthread-2.19.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libresolv-2.19.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       librt-2.19.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libselinux.so.1
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libudev.so.1.6.2
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libuuid.so.1.3.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libz.so.1.2.8
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libnpgoogletalk.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libnpo1d.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       filesystems
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       online
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libflashplayer.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libnpjp2.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       javafx.properties
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libyelp.so.0.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       librhythmbox-itms-detection-plugin.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libtotem-cone-plugin.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libtotem-gmp-plugin.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libtotem-mully-plugin.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libtotem-narrowspace-plugin.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libenchant_aspell.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libenchant_hspell.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libenchant_ispell.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libenchant_myspell.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libpixbufloader-png.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       giomodule.cache
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libdconfsettings.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libgvfsdbus.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libunico.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libcanberra-gtk3-module.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       liboverlay-scrollbar.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libunity-gtk-module.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libgvfscommon.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libICE.so.6.3.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libSM.so.6.0.1
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libX11-xcb.so.1.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libX11.so.6.3.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libXau.so.6.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libXcomposite.so.1.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libXcursor.so.1.0.2
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libXdamage.so.1.1.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libXdmcp.so.6.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libXext.so.6.4.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libXfixes.so.3.1.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libXi.so.6.1.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libXinerama.so.1.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libXrandr.so.2.2.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libXrender.so.1.3.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libXt.so.6.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libXxf86vm.so.1.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libaspell.so.15.2.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libatk-1.0.so.0.21409.1
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libatk-bridge-2.0.so.0.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libatspi.so.0.0.1
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libboost_filesystem.so.1.55.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libboost_system.so.1.55.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libcairo-gobject.so.2.11400.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libcairo.so.2.11400.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libcanberra-gtk3.so.0.1.9
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libcanberra.so.0.2.5
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libdatrie.so.1.3.1
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libdbus-glib-1.so.2.2.2
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libdrm.so.2.4.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libenchant.so.1.6.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libexslt.so.0.8.17
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libffi.so.6.0.2
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libfontconfig.so.1.8.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libfreetype.so.6.11.1
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libgdk-3.so.0.1400.6
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libgdk-x11-2.0.so.0.2400.25
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libgdk_pixbuf-2.0.so.0.3100.1
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libgeoclue.so.0.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libgio-2.0.so.0.4302.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libglapi.so.0.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libgmodule-2.0.so.0.4302.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libgobject-2.0.so.0.4302.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libgraphite2.so.3.0.1
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libgstapp-1.0.so.0.404.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libgstaudio-1.0.so.0.404.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libgstbase-1.0.so.0.404.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libgstfft-1.0.so.0.404.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libgstpbutils-1.0.so.0.404.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libgstreamer-1.0.so.0.404.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libgsttag-1.0.so.0.404.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libgstvideo-1.0.so.0.404.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libgthread-2.0.so.0.4302.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libgtk-3.so.0.1400.6
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libgtk-x11-2.0.so.0.2400.25
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libharfbuzz-icu.so.0.937.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libharfbuzz.so.0.937.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libhunspell-1.3.so.0.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libicudata.so.52.1
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libicui18n.so.52.1
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libicuuc.so.52.1
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libjavascriptcoregtk-3.0.so.0.16.15
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libjpeg.so.8.0.2
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libltdl.so.7.3.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       liblttng-ust-tracepoint.so.0.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libmirclient.so.8
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libmircommon.so.3
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libmirprotobuf.so.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libnspr4.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libnss3.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libnssutil3.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libogg.so.0.8.2
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       liborc-0.4.so.0.22.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libpango-1.0.so.0.3600.8
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libpangocairo-1.0.so.0.3600.8
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libpangoft2-1.0.so.0.3600.8
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libpixman-1.so.0.32.4
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libplc4.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libplds4.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libprotobuf.so.9.0.1
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libsecret-1.so.0.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libsmime3.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libsoup-2.4.so.1.7.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libsqlite3.so.0.8.6
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libssl3.so
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libstdc++.so.6.0.20
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libtdb.so.1.3.4
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libthai.so.0.2.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libunity-gtk3-parser.so.0.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       liburcu-bp.so.2.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libvorbis.so.0.4.7
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libvorbisfile.so.3.3.6
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libwayland-client.so.0.3.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libwayland-cursor.so.0.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libwebkitgtk-3.0.so.0.22.13
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libwebp.so.5.0.1
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libxcb-dri2.so.0.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libxcb-dri3.so.0.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libxcb-glx.so.0.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libxcb-present.so.0.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libxcb-render.so.0.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libxcb-shm.so.0.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libxcb-sync.so.1.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libxcb.so.1.1.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libxkbcommon.so.0.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libxml2.so.2.9.2
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libxshmfence.so.1.0.0
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libxslt.so.1.1.28
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libGL.so.1.2.0
  0.1    0.0    0.0    0.0    0.0  24558 rhythmbox       mimeinfo.cache
  0.1    0.0    0.0    0.0    0.0  24558 rhythmbox       yelp.desktop
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       enchant.ordering
  0.1    0.0    0.0    0.0    0.0  24558 rhythmbox       applications
  0.1    0.0    0.0    0.0    0.0  24558 rhythmbox       hand2
  0.1    0.0    0.1    0.0    0.0  13278 rhythmbox       icon-theme.cache
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       index.theme
  0.1    0.0    0.1    0.0    0.0  13278 rhythmbox       icon-theme.cache
  0.1    0.0    0.1    0.0    0.0  13278 rhythmbox       icon-theme.cache
  0.1    0.0    0.0    0.0    0.0  24558 rhythmbox       rhythmbox.png
  0.1    0.0    0.1    0.0    0.0  13278 rhythmbox       icon-theme.cache
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       index.theme
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       baobab.css
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       california.css
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       gedit.css
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       glade.css
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       gnome-panel.css
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       gnome-system-log.css
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       gnome-terminal.css
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       nautilus.css
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       unity-greeter.css
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       unity.css
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       entry-disabled.png
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       entry.png
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       gtk-main.css
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       gtk-widgets-assets.css
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       gtk.css
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       public-colors.css
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       settings.ini
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       gtk-keys.css
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       passwd
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       user
  0.1    0.1    0.0    0.0    0.0  24558 rhythmbox       user
  0.1    0.0    0.1    0.0    0.0  24558 rhythmbox       user (deleted)
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       mime.cache
  0.1    0.0    0.0    0.0    0.0  24558 rhythmbox       pulse-shm-1366707972
  0.1    0.0    0.0    0.0    0.0  24558 rhythmbox       pulse-shm-1669457513
  0.1    0.0    0.0    0.0    0.0  24558 rhythmbox       pulse-shm-2031561606 (deleted)
  0.1    0.0    0.0    0.0    0.0  24558 rhythmbox       pulse-shm-2112237257
  0.1    0.0    0.0    0.0    0.0  24558 rhythmbox       pulse-shm-2244561305
  0.1    0.0    0.0    0.0    0.0  24558 rhythmbox       pulse-shm-2635381376
  0.1    0.0    0.0    0.0    0.0  24558 rhythmbox       pulse-shm-321497414
  0.1    0.0    0.0    0.0    0.0  24558 rhythmbox       pulse-shm-3786106623
  0.1    0.0    0.0    0.0    0.0  24558 rhythmbox       pulse-shm-4089468629
  0.1    0.0    0.0    0.0    0.0  24558 rhythmbox       pulse-shm-4119676943
  0.1    0.0    0.0    0.0    0.0  24558 rhythmbox       pulse-shm-4216508789
  0.1    0.0    0.0    0.0    0.0  24558 rhythmbox       pulse-shm-558401616
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       locale-archive
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       gconv-modules.cache
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       gschemas.compiled
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       gtk30.mo
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       atk10.mo
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       gdk-pixbuf.mo
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       glib20.mo
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       gtk30-properties.mo
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       gtk30.mo
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       libc.mo
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       yelp.mo
  0.1    0.0    0.0    0.0    0.0  13278 rhythmbox       mime.cache
  0.0    0.0    0.0    0.0    0.0  24558 rhythmbox       pulse-shm-2368610133
  0.0    0.0    0.0    0.0    0.0  24558 rhythmbox       yelp.mo
```
