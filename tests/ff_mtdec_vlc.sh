date
ffmpeg -i $2 -f rawvideo -c:v rawvideo -pix_fmt gray - | ../build/qvsdec -f $1 -m mixed -p 99 -w 4 | vlc -
date
