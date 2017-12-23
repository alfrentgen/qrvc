date
../build/mtqvsenc -f $1 -s 4 -w 2 -i ./$2.bin -r 1 -t 10 | ../build/qvsdec -f $1 -p 99 -w 2 -m mixed -o ./$2.dec
date
sha512sum $2.bin $2.dec
