date
#../build/mtqvsenc -f $1 -s 4 -i ./$2 -w $3 -a $4 -r 1 -t 10 -p 99 | ffmpeg -y -f rawvideo -pix_fmt gray -s:v $1 -i - -c:v libx264 -pix_fmt yuv420p -preset fast -x264-params subme=0:me="dia" $2_$1.avi
#../build/mtqvsenc -f $1 -s 4 -i ./$2 -w $3 -a $4 -r 1 -t 10 -p 99 | ffmpeg -y -f rawvideo -pix_fmt gray -s:v $1 -i - -c:v libxvid -flags gray $2_$1.avi
#../build/mtqvsenc -f $1 -s 4 -i ./$2 -w $3 -a $4 -r 1 -t 10 -p 99 | ffmpeg -y -f rawvideo -pix_fmt gray -s:v $1 -i - -c:v libx264 -b:v 1000k -pix_fmt yuv420p -strict -2 -passlogfile /home/alf/workspace/pass.log  $2_$1.avi
ffmpeg -y -i $1 -c:v libx265 -preset placebo -strict experimental -b:v 1000k $1_reenc.mp4
date
