import  os
from steganography import *
working_path = '/media/alf/storage1/qrvc/qrvc_linux_x86_64'
os.chdir(working_path)

dest = open('sample.yuv', 'wb')
qr = open('1M_1280x720.yuv', 'rb')
anim = open('anim.yuv', 'rb')
anim_frame = 66
threshold = 0x7fffff
width = int(1280)
height = int(720)

chroma_size = int(width * height/4)
luma_size = int(chroma_size * 4)
frame_size = int(chroma_size + 2 * luma_size)

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

anim.seek(anim_offset)
anim_frame = anim.read(luma_size + 2 * chroma_size)
anim_frame = bytearray(anim_frame)
print(len(anim_frame))

try:
    #algorithm_2_chroma(anim_u, qr_luma, width, height, mask)
    #algorithm_2_luma(anim_y, qr_luma, width, height, mask)

    mask = 0x00000003
    synth_frame = algorithm_3(anim_frame, width, height, qr_luma, mask)
except Exception as ex:
    raise
    
#synth_frame = anim_y + anim_u + anim_v

dest.write(synth_frame)
dest.close()
qr.close()
anim.close()
