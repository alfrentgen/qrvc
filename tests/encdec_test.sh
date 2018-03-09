date
#../build/qvsenc -f $1 -s 8 -w 2 -i ./$2.bin -r 1 -t 10 -p 99 | ../build/qvsdec -f $1 -p 99 -w 2 -m mixed -o ./$2.dec
#test auto params
../build/qrve -i ./$1.bin -p 99 | ../build/qrvd -o ./$1.dec -p 99
date
#sha512sum $2.bin $2.dec
sha512sum $1.bin $1.dec
