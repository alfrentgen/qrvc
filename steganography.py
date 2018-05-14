def algorithm_1(plane):
    try:
        for i in range(luma_size):
            lum_raw = int(i/width)
            lum_col = int(i%width)
            ch_raw = int((i/width)/2)
            ch_col = int((i%width)/2)
            ch_width = int(width/2)
            ch_i = int(ch_raw * ch_width + ch_col)

            if plane[ch_i] in range(64 + 32, 128):
                if qr_luma[i] == 0:
                    plane[ch_i] -= 64
                elif qr_luma[i] == 255:
                    plane[ch_i] += 64
                
            if plane[ch_i] in range(128, 128 + 64 - 32):
                if qr_luma[i] == 0:
                    plane[ch_i] -= 64
                elif qr_luma[i] == 255:
                    plane[ch_i] += 64
    except Exception as ex:
        print("exception")
    finally:
        print(len(anim_y), len(anim_u),len(anim_v))
        print(i, lum_raw, lum_col)
        print(ch_i, ch_raw, ch_col)

#
def algorithm_2_chroma(chroma_plane, qr_plane, frame_width, frame_height, mask):
    width = int(frame_width)
    frame_size = int(frame_width * frame_height)

    for i in range(frame_size):
        lum_raw = int(i/frame_width)
        lum_col = int(i%frame_width)
        ch_raw = int((i/frame_width)/2)
        ch_col = int((i%frame_width)/2)
        ch_width = int(frame_width/2)
        ch_i = int(ch_raw * ch_width + ch_col)

        if qr_plane[i] == 0:
            var = int(chroma_plane[ch_i] & mask).to_bytes(4, 'little', signed=True)
            chroma_plane[ch_i] = var[0]
        elif qr_plane[i] == 255:
            var = int(chroma_plane[ch_i] | (~mask)).to_bytes(4, 'little', signed=True)
            chroma_plane[ch_i] = var[0]

#
def algorithm_2_luma(luma_plane, qr_plane, frame_width, frame_height, mask):
    width = int(frame_width)
    frame_size = int(frame_width * frame_height)
    
    for i in range(frame_size):
        if qr_plane[i] == 0:
            var = int(luma_plane[i] & mask).to_bytes(4, 'little', signed=True)
            luma_plane[i] = var[0]
        elif qr_plane[i] == 255:
            var = int(luma_plane[i] | (~mask)).to_bytes(4, 'little', signed=True)
            luma_plane[i] = var[0]

#
def algorithm_3(frame, frame_width, frame_height, qr_plane, mask):

    luma_size = int(frame_width * frame_height)
    chroma_size = int(luma_size/4)
    plane_y = bytearray(frame[0:luma_size])
    plane_u = bytearray(frame[luma_size:luma_size + chroma_size])
    plane_v = bytearray(frame[luma_size + chroma_size:luma_size + 2 * chroma_size])
    print(len(plane_y), len(plane_u), len(plane_v), len(qr_plane))
    print(frame_width, frame_height, mask)

    
    for i in range(luma_size):
        #lum_raw = int(i/frame_width)
        #lum_col = int(i%frame_width)
        ch_raw = int((i/frame_width)/2)
        ch_col = int((i%frame_width)/2)
        ch_width = int(frame_width/2)
        ch_i = int(ch_raw * ch_width + ch_col)

        var = int(plane_y[i] + plane_u[ch_i] + plane_v[ch_i])
        var_m = int(var & mask)
        diff_up = mask - var_m
        diff_down = var_m

        y = int(plane_y[i])
        y = y.to_bytes(4, 'little', signed=True)
        
        if qr_plane[i] == 0:
            if y[0] != 0:
                val = int(plane_y[i] - diff_down)
                val = val.to_bytes(4, 'little', signed=True)
                plane_y[i] = val[0]
        elif qr_plane[i] == 255:
            if y[0] != 0xff:
                val = int(plane_y[i] + diff_up)
                val = val.to_bytes(4, 'little', signed=True)
                plane_y[i] = val[0]

    new_frame = plane_y + plane_u + plane_v
    return new_frame
