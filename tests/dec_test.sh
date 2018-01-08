date
echo $3
../build/qvsdec -f $1 -i ./$2.yuv -o ./$2.dec -p 99 -w $3 -m mixed -k #2>/dev/null
date
