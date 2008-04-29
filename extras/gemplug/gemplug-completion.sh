# gemplug(1) completion 
#
# Put this file into /etc/bash_completion.d/, or
# manually add it to /etc/bash_completion or to your profile
# in order to get your outlet names completed by bash.
#

_gemplug1(){
    local item shopt_nocasematch

    shopt_nocasematch=$(shopt -p nocasematch)
    shopt -s nocasematch
    for item in $(gemplug -nqq); do
        [[ "$item" == "$2"* ]] && COMPREPLY[${#COMPREPLY[@]}]="$item"
    done
    $shopt_nocasematch
}
complete -F _gemplug1  gemplug
