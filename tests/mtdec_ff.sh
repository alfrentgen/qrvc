date
ffmpeg -i $2 -f rawvideo -c:v rawvideo -pix_fmt gray - | ../build/qvsdec -f $1 -o yt_vlc.dec -m mixed -p 99 -w 2
date
