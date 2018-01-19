date
../build/mtqvsenc -f $1 -s 4 -i ./$2 -w $3 -a $4 -r 1 -t 10 -p 99 | ffmpeg -y -f rawvideo -pix_fmt gray -s:v $1 -i - -c:v libx264 -pix_fmt yuv420p -preset medium -tune ssim -x264-params subme=0:me="dia" $2_$1.avi
#../build/mtqvsenc -f $1 -s 4 -i ./$2 -w $3 -a $4 -r 1 -t 10 -p 99 -c | ffmpeg -y -f rawvideo -pix_fmt gray -s:v $1 -i - -c:v libx264 -pix_fmt yuv420p -preset veryslow -x264-params subme=0 $2_$1.avi
#../build/mtqvsenc -f $1 -s 4 -i ./$2 -w $3 -a $4 -r 1 -t 10 -p 99 | ffmpeg -y -f rawvideo -pix_fmt gray -s:v $1 -i - -c:v libx264 -pix_fmt yuv420p -preset fast -tune ssim -x264-params subme=0:me="dia" $2_$1.avi
date
