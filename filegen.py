dest = open('sample.yuv', 'wb')
qr = open('1M-ff.yuv', 'rb')
anim = open('BAO.yuv', 'rb')
anim_frame = 66

chroma_size = int(1280*720/4)
luma_size = chroma_size * 4
frame_size = chroma_size + 2 * luma_size

luma = qr.read(luma_size)
luma = bytearray(luma)
ch_u = bytearray(chroma_size)
ch_v = bytearray(chroma_size)

synth_frame = bytearray(frame_size)

anim_offset = anim_frame * (luma_size + 2 * chroma_size)
#anim_offset = anim_offset + luma_size
#anim.seek(anim_offset)
#anim_chroma = anim.read(2 * chroma_size)
#anim_chroma = bytearray(anim_chroma)

for i in range(luma_size):
    synth_frame[i] = luma[i]

for i in range(2 * chroma_size):
    anim_chroma[i] = anim_chroma[i]
    synth_frame[luma_size + i] = 2 * anim_chroma[i]

#for i in range(luma_size, luma_size + chroma_size):
#    synth_frame[i] =127

#for i in range(luma_size + chroma_size, luma_size + 2 * chroma_size):
#    synth_frame[i] = 0

dest.write(synth_frame)
dest.close()
qr.close()
anim.close()
