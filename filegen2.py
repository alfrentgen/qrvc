dest = open('sample.yuv', 'wb')
qr = open('1M_640x360.yuv', 'rb')
anim = open('BAO.yuv', 'rb')
anim_frame = 66

chroma_size = int(1280*720/4)
luma_size = chroma_size * 4
frame_size = chroma_size + 2 * luma_size

qr_u = qr.read(chroma_size)
qr_u = bytearray(qr_u)
#ch_v = bytearray(ch_u)
#ch_v = bytearray(chroma_size)

anim_offset = anim_frame * (luma_size + 2 * chroma_size)
anim.seek(anim_offset)
anim_luma = anim.read(luma_size)
luma = bytearray(anim_luma)

anim.seek(anim_offset + luma_size + chroma_size)
anim_v = anim.read(chroma_size)
anim_v = bytearray(anim_v)

#print(len(luma), len(ch_u), len(ch_v))

synth_frame = bytearray()
synth_frame = luma + qr_u + anim_v
#synth_frame.join(ch_u)
#synth_frame.join(ch_v)

dest.write(synth_frame)
dest.close()
qr.close()
anim.close()
