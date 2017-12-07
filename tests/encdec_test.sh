date
../build/qvsenc -f $1 -s 4 -i ./$2.bin 2>/dev/null | ../build/qvsdec -f $1 -o ./$2.dec -p 99 -w 4 -m mixed
date
