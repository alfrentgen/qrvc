date
vlc $2 --sout '#std{access=file,mux=avi,dst=-}"' --no-audio vlc://quit | ffmpeg -i - -f rawvideo -c:v rawvideo -pix_fmt gray - | ../build/qvsdec -f $1 -o yt_vlc.dec -m mixed -p 99 -w $3
date
