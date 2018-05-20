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


#in luma plane 2 the most meaning beats of a value:
#if they are 11xxxxxx[192...255] or 01xxxxxx[64...127], we counts them as 1 in qr code,
#othewise they are 00xxxxxx[0...63] or 10xxxxxx[128...191], we counts them as 0 in qr code.
#
#in luma plane 3 the least meaning beats of a value:
#signaling inversion with 3 the least meaning bits:
#let the values from 0...3(i.e. 0x00...0x03) counts as no inversion(set 0x02),
#and the values from 4...7(i.e. 0x04...0x07) counts as inversion signal(set 0x05).
            
def algorithm_4(frame, frame_width, frame_height, qr_plane):
    print("algorithm_4")
    luma_size = int(frame_width * frame_height)
    chroma_size = int(luma_size/4)
    plane_y = bytearray(frame[0:luma_size])
    plane_u = bytearray(frame[luma_size:luma_size + chroma_size])
    plane_v = bytearray(frame[luma_size + chroma_size:luma_size + 2 * chroma_size])
    print(len(plane_y), len(plane_u), len(plane_v), len(qr_plane))
    print(frame_width, frame_height)

    
    for i in range(luma_size):
        ch_raw = int((i/frame_width)/2)
        ch_col = int((i%frame_width)/2)
        ch_width = int(frame_width/2)
        ch_i = int(ch_raw * ch_width + ch_col)


        y = int(plane_y[i] & 0xffffffC0)  #11000000b
        val = int(plane_y[i] & 0xfffffff8)#11111000b
            
        if qr_plane[i] == 0: #black
            if y != 0 or y != 128:
                val += 2
                val = val.to_bytes(4, 'little', signed=True)
                plane_y[i] = val[0]
        elif qr_plane[i] == 255: #white
            if y != 64 or y != 192:
                val += 5
                val = val.to_bytes(4, 'little', signed=True)
                plane_y[i] = val[0]

    new_frame = plane_y + plane_u + plane_v
    return new_frame

def algorithm_4_chroma(frame, frame_width, frame_height, qr_plane):
    print("algorithm_4")
    luma_size = int(frame_width * frame_height)
    chroma_size = int(luma_size/4)
    plane_y = bytearray(frame[0:luma_size])
    plane_u = bytearray(frame[luma_size:luma_size + chroma_size])
    plane_v = bytearray(frame[luma_size + chroma_size:luma_size + 2 * chroma_size])
    print(len(plane_y), len(plane_u), len(plane_v), len(qr_plane))
    print(frame_width, frame_height)

    
    for i in range(luma_size):
        ch_raw = int((i/frame_width)/2)
        ch_col = int((i%frame_width)/2)
        ch_width = int(frame_width/2)
        ch_i = int(ch_raw * ch_width + ch_col)

        y = int(plane_y[i] & 0xffffffC0)  #11000000b
        u = int(plane_u[ch_i] & 0xffffffC0)  #11000000b
        v = int(plane_v[ch_i] & 0xffffffC0)  #11000000b
        
        val = int(plane_y[i] & 0xfffffff8)#11111000b
        val = val.to_bytes(4, 'little', signed=True)
        plane_y[i] = val[0]
            
        if qr_plane[i] == 0: #black
            if y != 0 or y != 128:
                #signal somehow that inversion is needed
                val += 2
                val = val.to_bytes(4, 'little', signed=True)
                plane_y[i] = val[0]
        elif qr_plane[i] == 255: #white
            if y != 64 or y != 192:
                #signal somehow that inversion is needed
                val += 5
                val = val.to_bytes(4, 'little', signed=True)
                plane_y[i] = val[0]

    new_frame = plane_y + plane_u + plane_v
    return new_frame

#####
def calc_mean(pixels):
    l = len(pixels)
    summ = int(0)
    for p in pixels:
        summ += p
    return int(summ/len(pixels))

def change_pels(pels, neigh, bit):
    try:
        inc = 1
        if bit == 255:
            inc = -1

        #neighbourpels
        for i in range(0, len(neigh)):
            if (neigh[i] < 255) and(neigh[i] > 0):
                neigh[i] = neigh[i] + inc

        #selected pel
        for i in range(0, len(pels)):
            if (pels[0] < 255) and(pels[0] > 0):
                    pels[i] = pels[i] - inc
                    
    except:
        print(len(pels))
        print(len(neigh))
        print(pels[i], bit)
        raise

    return

def render_pels(plane, stride, idx, threshold, bit_val):
    #print(idx)
    p = plane
    pels_inds = (idx,
                 #idx + 1, idx + stride,
                 idx + stride + 1)# 4 inner pels

    neigh_inds = (
                #idx - stride,
                idx - stride + 1,#two top border pels
                idx - stride - 1,
                #idx - 1,
                idx + stride - 1,
                #idx + 2 * stride - 1,#left border
                idx - stride + 2,
                #idx + 2,
                idx + stride + 2,
                #idx + 2 * stride + 2,#right border
                idx + 2 * stride,
                #idx + 2 * stride + 1
                )#two bottom border pels

    pels = bytearray()
    for i in pels_inds:
        pels.append(p[i])

    neigh = bytearray()
    for i in neigh_inds:
        neigh.append(p[i])
    
    thr = int(threshold)
    #cur_pel = int(p[idx])
    mean_pel = int(calc_mean(neigh))
    cur_pel = int(calc_mean(pels))

    try:
        n = 0
        if bit_val == 0:
            while mean_pel - cur_pel <= thr:
                #print('under thr')
                n += 1
                if n > 256:
                    print('mean_pel - cur_pel = ', mean_pel - cur_pel)
                    print(pels)
                    print(cur_pel)
                change_pels(pels, neigh, bit_val)
                mean_pel = int(calc_mean(neigh))
                cur_pel = int(calc_mean(pels))#int(pels[0])
        elif bit_val == 255:
            while mean_pel - cur_pel >= -thr:
                #print('above thr')
                n += 1
                if n > 256:
                    print('cur_pel - mean_pel = ', cur_pel - mean_pel)
                    print(cur_pel)
                change_pels(pels, neigh, bit_val)
                mean_pel = int(calc_mean(neigh))
                cur_pel = int(calc_mean(pels))#int(pels[0])
                
        for j in range(0, len(pels_inds)):
            p[pels_inds[j]] = pels[j]
        for j in range(0, len(neigh_inds)):
            p[neigh_inds[j]] = neigh[j]

    except:
        print(pels, len(pels))
        print(pels_inds)
        print(neigh_inds)
        raise

    return

def algorithm_5(frame, frame_width, frame_height, qr_plane, threshold):
    print("algorithm_5")
    thr = int(threshold)
    luma_size = int(frame_width * frame_height)
    lum_width = int(frame_width)
    
    chroma_size = int(luma_size/4)
    ch_width = int(frame_width/2)
    
    plane_y = bytearray(frame[0:luma_size])
    plane_u = bytearray(frame[luma_size:luma_size + chroma_size])
    plane_v = bytearray(frame[luma_size + chroma_size:luma_size + 2 * chroma_size])
    print(len(plane_y), len(plane_u), len(plane_v), len(qr_plane))
    print(frame_width, frame_height)

    for i in range(0,frame_height,4):    
        for j in range(0,frame_width,4):
            lum_raw = int(i)
            lum_col = int(j)
            lum_i = int(lum_raw * lum_width + lum_col)
            
            ch_raw = int(i/2)
            ch_col = int(j/2)
            ch_i = int(ch_raw * ch_width + ch_col)

            idx = int(lum_i + lum_width + 1)
            render_pels(plane_y, lum_width, idx, thr, qr_plane[lum_i])

    new_frame = plane_y + plane_u + plane_v
    return new_frame
