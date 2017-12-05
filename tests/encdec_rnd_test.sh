date
head -c $2 < /dev/urandom > $2.bin
../build/qvsenc -i $2.bin -f $1 -s 4 |
../build/qvsdec -f $1 -o $2.dec -p 99 -w 3 -m quick
date
