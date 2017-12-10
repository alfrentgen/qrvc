date
../build/qvsenc -f $1 -s 4 -i ./$2.bin 2>/dev/null | ../build/qvsdec -f $1 -p 99 -w 2 -m mixed >>$2.dec
date
