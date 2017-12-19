date
../build/mtqvsenc -f $1 -s 8 -i ./$2.bin -w 2 | ffmpeg -y -f rawvideo -pix_fmt gray -s:v $1 -i - -c:v libx264 -x264opts keyint=1 $2_$1.avi
date
