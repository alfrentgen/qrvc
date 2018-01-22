date
#ffmpeg -i $2 -f rawvideo -c:v rawvideo -pix_fmt gray - 2>/dev/null | ../build/qvsdec -f $1 -o yt_vlc.dec -m mixed -p 99 -w $3 -c "./key.yuv"
#ffmpeg -i $2 -f rawvideo -c:v rawvideo -pix_fmt gray - 2>/dev/null | ../build/qvsdec -f $1 -o yt_vlc.dec -m mixed -p 99 -w $3 -c
ffmpeg -i $2 -f rawvideo -c:v rawvideo -pix_fmt gray - 2>/dev/null | ../build/qvsdec -f $1 -o /mnt/ramdisk/$2.dec -m mixed -p 99 -w $3 -c "./key.yuv"
date
