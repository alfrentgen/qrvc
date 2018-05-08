import  os
working_path = '/media/alf/storage1/qrvc/qrvc_linux_x86_64'
os.chdir(working_path)

dest = open('sample.yuv', 'wb')
qr = open('1M_1280x720.yuv', 'rb')
anim = open('anim.yuv', 'rb')
anim_frame = 210
threshold = 0x7fffff
width = int(1280)
height = int(720)

chroma_size = int(width * height/4)
luma_size = chroma_size * 4
frame_size = chroma_size + 2 * luma_size

qr_luma = qr.read(luma_size)
qr_luma = bytearray(qr_luma)

anim_offset = anim_frame * (luma_size + 2 * chroma_size)
anim.seek(anim_offset)
anim_luma = anim.read(luma_size)
anim_y = bytearray(anim_luma)

anim.seek(anim_offset + luma_size)
anim_u = anim.read(chroma_size)
anim_u = bytearray(anim_u)

anim.seek(anim_offset + luma_size + chroma_size)
anim_v = anim.read(chroma_size)
anim_v = bytearray(anim_v)

for i in range(luma_size):
    if anim_y[i] in range(64, 128):
        if qr_luma[i] == 0:
            anim_y[i] = 63
        elif qr_luma[i] == 255:
            anim_y[i] = 127
            
    if anim_y[i] in range(128, 128 + 64):
        if qr_luma[i] == 0:
            anim_y[i] = 127
        elif qr_luma[i] == 255:
            anim_y[i] = 128 + 64

print(len(anim_y), len(anim_u),len(anim_v))
synth_frame = anim_y + anim_u + anim_v

dest.write(synth_frame)
dest.close()
qr.close()
anim.close()
