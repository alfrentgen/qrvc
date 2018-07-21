IN_FILE=$1
YUV_FILE=$IN_FILE.yuv
KEY_FILE=$IN_FILE.stg
MP4_FILE=$IN_FILE.mp4
MP4_YUV=$MP4_FILE.yuv
OUT_FILE=$IN_FILE.dec
THRESHOLD=$2

if [ -z "$3" ]
then
      HOST_FILE=host.mp4
else
      HOST_FILE=$3
fi

rm $IN_FILE $YUV_FILE $KEY_FILE $MP4_FILE $MP4_YUV $OUT_FILE
head -c 1M < /dev/urandom > $IN_FILE
ffmpeg -i $HOST_FILE -f rawvideo -pix_fmt yuv420p - 2>/dev/null | ./qrve -i $IN_FILE -o $YUV_FILE --stg th=$THRESHOLD
ffmpeg -y -f rawvideo -pix_fmt yuv420p -s:v 1280x720 -i $YUV_FILE -c:v libx264 -pix_fmt yuv420p $MP4_FILE 2>/dev/null
ffmpeg -i $MP4_FILE -f rawvideo -pix_fmt yuv420p $MP4_YUV 2>/dev/null
./qrvd -i $MP4_YUV --stg kf=$KEY_FILE -o $OUT_FILE
sha512sum $IN_FILE $OUT_FILE
