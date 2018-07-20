IN_FILE=$1
OUT_FILE=$2
THRESHOLD=$3

rm 1M_stg.yuv 1M.in.stg 1M_stg.mp4 1M_stg.mp4.yuv $2 $1.stg
ffmpeg -i short.mp4 -f rawvideo -pix_fmt yuv420p - 2>/dev/null | ./qrve -i $1 -o 1M_stg.yuv --stg th=$3
ffmpeg -y -f rawvideo -pix_fmt yuv420p -s:v 1280x720 -i 1M_stg.yuv -c:v libx264 -pix_fmt yuv420p 1M_stg.mp4 2>/dev/null
ffmpeg -i 1M_stg.mp4 -f rawvideo -pix_fmt yuv420p 1M_stg.mp4.yuv 2>/dev/null
./qrvd -i 1M_stg.mp4.yuv --stg kf=$1.stg -o $2
sha512sum $1 $2
