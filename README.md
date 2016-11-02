OpenBRF _by Marco Martini_
------------------------

A modding tool used to view, edit and convert the proprietary _Binary Resource files_ (BRFs) loaded by the popular «Mount&Blade» and «Mount&Blade: Warband» games.

This GitHub repo was last updated from the source code ZIP file at:

https://forums.taleworlds.com/index.php?topic=72279.0

and modified to compile under Linux.

### License
This code and the original ZIP file are licensed under the _GNU
General Public License_, according to the dicussion forum:

https://forums.taleworlds.com/index.php?topic=72279.0

### Build instructions for Linux:

    # Now requires Qt5!
    qmake -makefile openBrf.pro
    make
    
    # run it overriding the float dot notation so that it can load
    # `carry_positions.txt` in other languages other than English
    env LC_NUMERIC=C ./openBrf

Alternatively there's also a ready-to-use _AUR_ package that might come in handy if you are using _Arch Linux_ (helpfully provided by Swyter):

https://aur.archlinux.org/packages/openbrf/
