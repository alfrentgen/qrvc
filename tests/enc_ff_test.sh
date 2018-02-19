date
#with cyphering
#../build/qvsenc -f $1 -s 4 -i ./$2 -w $3 -a $4 -r 1 -t 10 -p 99 -c "./key.yuv" | ffmpeg -y -f rawvideo -pix_fmt gray -s:v $1 -i - -c:v libx264 -pix_fmt yuv420p -preset veryslow -tune ssim -x264-params subme=0:me="dia" $2_$1.mp4
../build/qvsenc -f $1 -s 4 -i ./$2 -w $3 -a $4 -r 1 -t 10 -p 99 -c | ffmpeg -y -f rawvideo -pix_fmt gray -s:v $1 -i - -c:v libx264 -pix_fmt yuv420p -preset medium -tune ssim -x264-params subme=0:me="dia" $2_$1.avi
#without cyphering
#../build/qvsenc -f $1 -s 4 -i ./$2 -w $3 -a $4 -r 1 -t 10 -p 99 | ffmpeg -y -f rawvideo -pix_fmt gray -s:v $1 -i - -c:v libx264 -pix_fmt yuv420p -preset veryslow -x264-params subme=0 $2_$1.avi
#../build/qvsenc -f $1 -s 4 -i ./$2 -w $3 -a $4 -r 1 -t 10 -p 99 | ffmpeg -y -f rawvideo -pix_fmt gray -s:v $1 -i - -c:v libx264 -pix_fmt yuv420p -preset fast -tune ssim -x264-params subme=0:me="dia" $2_$1.avi
#kvazaar
#../build/qvsenc -f $1 -s 4 -i ./$2 -w $3 -a $4 -r 1 -t 10 -p 99 | kvazaar -i - -o ./enc_kvaz.h265 --input-format P400 --input-bitdepth 8 --input-res $1 --subme 0 --bitrate 2000000
#../build/qvsenc -f $1 -s 4 -i ./$2 -w $3 -a $4 -r 1 -t 10 -p 99 | kvazaar -i - -o ./enc_kvaz.h265 --input-format P400 --input-bitdepth 8 --input-res $1 --preset veryslow
date
