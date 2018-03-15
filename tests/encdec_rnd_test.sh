head -c $1 < /dev/urandom > $1.bin
date
../build/qrve -i $1.bin -p 99 | ../build/qrvd -o $1.dec -p 99
date
sha512sum $1.bin $1.dec
