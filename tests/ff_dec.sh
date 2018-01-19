date
ffmpeg -y -i $1 -f rawvideo -c:v rawvideo -pix_fmt gray ff_dec.yuv
date
