#!/bin/bash
#   install.sh
#
# Time-stamp: <2008-04-30 08:53:26 hcz>
#
# Script to install gemplug

#  $   1            2                      3    4           5
#      source       target                 mode create dir? overwrite?
set -- gemplug      /usr/local/bin         755  n           n \
       gemplug.1    /usr/local/man/man1    644  y           n \
       74-sispmctl.rules /etc/udev/rules.d 644  n           n \
       gemplug-completion.sh /etc/bash_completion.d 755 n   n

abort(){
    echo "Installation aborted."
    exit 1
}

while [ "$1" ]; do
    if [ ! -e "$2/$1" ] || [ "$2/$1" -ot "$1" ] || [ "$5" = "y" ]; then
        printf "installing %s into %s\n" "$1" "$2"
        [ "$4" = "y" ] && mkdir -p "$2"
        install -m "$3" -o root -g root "$1" "$2" || abort
    else
        echo "Skipping $1 (target not older)."
    fi
    shift 5
done
echo "gemplug files done."
       
