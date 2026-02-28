import numpy as np
from numba import njit, prange
from packing import *

@njit(inline='always')
def zh(b,zobra):
    h = np.uint64(0)
    for i in range(8):
        for j in range(8):
            h ^= zobra[b[i, j], i, j]
    return h

@njit()
def moves(b, turn, allm, d):
    total = 0
    delta = turn * 2 - 1
    owned = 1 + turn
    for i in range(1,8):
        i = (8-1)*turn -i*delta
        pos = i+delta
        for j in range(8):
            # j = 8 - 1 -j
            j = (j%2)*(8-1) - (j//2)*(j%2-(j+1)%2)
            if b[i, j] != owned:
                continue
            if j>0 and b[pos, j-1] != owned:
                allm[d, total] = pack(i, j, -1)
                total += 1
            if j<8-1 and b[pos, j+1] != owned:
                allm[d, total] = pack(i, j, +1)
                total += 1
            if b[pos, j]==0:
                allm[d, total] = pack(i, j,  0)
                total += 1
    allm[d, 6*8] = total

@njit()
def bmoves(b, turn, allm, d):
    p1, p2 = bb(b)
    acn = ~(p1 | p2)
    if not turn:
        pn = ~p1
        pL = p1 & (pn >> 9) & 0x7f7f7f7f7f7f7f7f # bitmask leftmost moves
        pR = p1 & (pn >> 7) & 0xfefefefefefefefe # bitmask rightmost moves
        pC = p1 & (acn >> 8)
    else:
        pn = ~p2
        pL = p2 & (pn << 7) & 0x7f7f7f7f7f7f7f7f
        pR = p2 & (pn << 9) & 0xfefefefefefefefe
        pC = p2 & (acn << 8)
    
    total = 0
    last = 0
    for i in range(7, -1, -1):
        for j in range(7, -1, -1):
            iL = pL & 1
            iR = pR & 1
            iC = pC & 1

            total += iL

            total += iR

            total += iC

            pL >>= 1
            pR >>= 1
            pC >>= 1

        



@njit()
def temp_diff(b, replica, r, c, maximize):
    val = b[r, c]
    s = 3 - 2*val

    idx = (8-1)*(val-1) + s*r
    h = -s * replica[idx, c]

    DR = (-1, -1, -1,  1,  1,  1)
    DC = ( 0, -1,  1,  0, -1,  1)
    for k in range(6):
        rr = r + DR[k]
        if 0 <= rr < 8:
            cc = c + DC[k]
            if 0 <= cc < 8:
                h += s * (b[rr, cc] == val)
    return h

@njit(inline='always')
def move_diff(b):
    num = 0
    for i in range(0,8):
        if b[i, 0] == 1:
            if b[i-1, 1] != 1:
                num += -1
            if b[i-1, 0] == 0:
                num += -1
        elif b[i, 0] == 2:
            if b[i+1, 1] != 2:
                num += 1
            if b[i+1, 0] == 0:
                num += 1

        for j in range(1, 8-1):
            v = b[i, j]
            if v==1:
                if b[i-1, j-1]!=1:
                    num += -1
                if b[i-1, j+1]!=1:
                    num += -1
                if b[i-1, j]==0:
                    num += -1
            elif v==2:
                if b[i+1, j-1]!=2:
                    num += 1
                if b[i+1, j+1]!=2:
                    num += 1
                if b[i+1, j]==0:
                    num += 1
        
        j = 8-1
        if b[i, j] == 1:
            if b[i-1, j-1] != 1:
                num += -1
            if b[i-1, j] == 0:
                num += -1
        elif b[i, j] == 2:
            if b[i+1, j-1] != 2:
                num += 1
            if b[i+1, j] == 0:
                num += 1
        
    return num

@njit()
def state(b, replica):
    return move_diff(b) + quickeval(b, replica)

@njit(inline='always')
def quickeval(b, replica):
    h = 0
    for i in range(8):
        for j in range(8):
            val = b[i, j]
            if not val:
                continue
            h = h + (2*val-3) * replica[(8-1)*(val-1) - (2*val-3)*i, j]
    return h

@njit(inline='always')
def gameover(b):
    for i in range(8):
        if b[0, i]==1:
            return -127
        if b[8-1, i]==2:
            return 127
    return 0

@njit()
def sorter(b, allm, d, total, turn, replica, bm, scores):
    delta = 2*turn-1
    for i in range(total):
        r, c, t = unpack(allm[d, i])
        old = b[r+delta, c+t]
        b[r, c] = 0
        b[r+delta, c+t] = turn+1
        scores[i] = quickeval(b, replica)
        b[r, c] = turn+1
        b[r+delta, c+t] = old
    if turn:
        for i in range(bm, total):
            key_s = scores[i]
            key_m = allm[d, i]
            j = i - 1
            while j >= bm and scores[j] < key_s:
                scores[j+1] = scores[j]
                allm[d, j+1] = allm[d, j]
                j -= 1
            scores[j+1] = key_s
            allm[d, j+1] = key_m
    else:
        for i in range(bm, total):
            key_s = scores[i]
            key_m = allm[d, i]
            j = i - 1
            while j >= bm and scores[j] > key_s:
                scores[j+1] = scores[j]
                allm[d, j+1] = allm[d, j]
                j -= 1
            scores[j+1] = key_s
            allm[d, j+1] = key_m

@njit()
def ai_turn(b, allm, tt, depth, zobra, replica, scores):
    win = 8
    t = (-1, -1, -1)
    for i in range(8):
        if b[8-2, i]==2:
            win = i
            break
    if win<8-1:
        t = (8-2, win, +1)
    elif win==8-1:
        t = (8-2, win, -1)
    else:
        maxeval = -127
        alfa = -127
        beta = +127

        ndepth = depth-1
        moves(b, 1, allm, ndepth)
        h = zh(b,zobra)
        _, _, _, _, best_move_tt = unpack_tt(tt[h & ((1 << 19) - 1)])

        
        bm = 0
        sorter(b, allm, ndepth, allm[ndepth, 6*8],1, replica, bm, scores)
        if best_move_tt != 0:
            for i in range(allm[ndepth, 6*8]):
                if allm[ndepth, i] == best_move_tt:
                    allm[ndepth, i], allm[ndepth, 0] = allm[ndepth, 0], allm[ndepth, i]
                    bm = 1
                    break


        for i in range(allm[ndepth, 6*8]):
            m = unpack(allm[ndepth, i])
            if b[m[0], m[1]] != 2:
                continue
            # print('Tested move:',m)
            old = b[m[0]+1, m[1]+m[2]]
            b[m[0], m[1]] = 0
            b[m[0]+1, m[1]+m[2]] = 2
            p1, p2 = bb(b)
            ev = minimax(b, ndepth, 0, alfa, beta, allm, tt, 1, zobra, replica, scores, state(b, replica), zh(b, zobra), p1, p2)
            if ev>maxeval:
                maxeval = ev
                t = m
            alfa = max (alfa, ev)
            b[m[0], m[1]] = 2
            b[m[0]+1, m[1]+m[2]] = old
            if beta <= alfa:
                break
    # print(maxeval)
    if t!=(-1, -1, -1):
        b[t[0], t[1]] = 0
        b[t[0]+1, t[1]+t[2]] = 2
        return t[0], t[1], t[0]+1, t[1]+t[2]
    return 0, 0, 0, 0

@njit()
def minimax(b, depth, maximize, alfa0, beta0, allm, tt, deep, zobra, replica, scores, score0, h0, p10, p20):

    alfa = alfa0
    beta = beta0
    best_move = 0
    idx = h0 & ((1 << 21) - 1)
    uphash, score_tt, depth_tt, flag_tt, best_move_tt = unpack_tt(tt[idx])
    if uphash == (h0 >> 40):
        if depth_tt >= depth:
            if flag_tt == 0:
                return score_tt
            elif flag_tt == 1 and score_tt <= alfa:
                return score_tt
            elif flag_tt == 2 and score_tt >= beta:
                return score_tt
    # else:
        # print('collision!')
    
    ndepth = depth-1
    moves(b, maximize, allm, ndepth)

    p1, p2 = p10, p20

    bm = 0
    if deep<5:
        sorter(b, allm, ndepth, allm[ndepth, 6*8], maximize, replica, bm, scores)
    if best_move_tt != 0:
        for i in range(allm[ndepth, 6*8]):
            if allm[ndepth, i] == best_move_tt:
                allm[ndepth, i], allm[ndepth, 0] = allm[ndepth, 0], allm[ndepth, i]
                bm = 1
                break

    if maximize:
        maxeval = -127
        for i in range(allm[ndepth, 6*8]):
            r, c, mt = unpack(allm[ndepth, i])
            r2 = r+1
            if r2 == 8-1:
                ev = +127
            else:
                c2 = c+mt
                old = b[r2, c2]
                score = score0 - temp_diff(b,replica, r, c, 1)
                if old!=0:
                    score -= temp_diff(b, replica, r2, c2, 1)
                
                b[r, c] = 0
                b[r2, c2] = 2
                score = score + temp_diff(b, replica, r2, c2, 1)
                if ndepth==0:
                    ev = score
                else:
                    h = h0
                    h ^= zobra[2, r, c]
                    h ^= zobra[0, r, c]
                    h ^= zobra[old, r2, c2]
                    h ^= zobra[2, r2, c2]
                    ev = minimax(b, ndepth, 0, alfa, beta, allm, tt, deep+1, zobra, replica, scores, score, h, p1, p2)
                b[r, c] = 2
                b[r2, c2] = old
            if ev>maxeval:
                maxeval = ev
                best_move = allm[ndepth, i]
            alfa = max (alfa, ev)
            if beta <= alfa:
                break
        val = maxeval
    else:
        mineval = +127
        for i in range(allm[ndepth, 6*8]):
            r, c, mt = unpack(allm[ndepth, i])
            r2 = r-1
            if r2 == 0:
                ev = -127
            else:
                c2 = c+mt
                old = b[r2, c2]
                score = score0 - temp_diff(b, replica, r, c, 0)
                if old!=0:
                    score -= temp_diff(b, replica, r2, c2, 0)
                b[r, c] = 0
                b[r2, c2] = 1
                score = score + temp_diff(b, replica, r2, c2, 0)
                if ndepth == 0:
                    ev = score
                else:
                    h = h0
                    h ^= zobra[1, r, c]
                    h ^= zobra[0, r, c]
                    h ^= zobra[old, r2, c2]
                    h ^= zobra[1, r2, c2]
                    ev = minimax(b, ndepth, 1, alfa, beta, allm, tt, deep+1, zobra, replica, scores, score, h, p1, p2)
                b[r, c] = 1
                b[r2, c2] = old
            if ev < mineval:
                mineval = ev
                best_move = allm[ndepth, i]
            beta = min(beta, ev)
            if beta <= alfa:
                break
        val = mineval

    flag = 0
    if val <= alfa0: flag = 1
    elif val >= beta0: flag = 2
    if depth_tt < depth:
        tt[idx] = pack_tt(h0 >> 40, val, depth, flag, best_move)
    return val
