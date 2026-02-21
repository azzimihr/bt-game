import numpy as np

from numba import njit, prange

import tkinter as tk
from tkinter import font, messagebox

import time, random

def geo(tkw, w, h):
    tkw.geometry(f"{w}x{h}+{(tkw.winfo_screenwidth()-w)//2}+{(tkw.winfo_screenheight()-h-25)//2}")

def compile():
    global win
    win = False
    compiler = tk.Tk()
    ts = 5  # temporary size

    def precomp():
        start(ts, 3)

        # print("Fetching zh()")
        # zh(b, ts, zobra)
        # print("Fetching pack()")
        # pack(3, 4, 0)
        # print("Fetching unpack()")
        # unpack(7)
        # print("Fetching pack_tt()")
        # pack_tt(1000, 20, 1, 0, 3)
        # print("Fetching unpack_tt()")
        # unpack_tt(7)
        # print("Fetching moves()")
        # moves(b, 0, allm, 1, ts)
        # print("Fetching move_diff()")
        # move_diff(b, ts)
        # print("Fetching state()")
        # state(b, ts, replica)
        # print("Fetching gameover()")
        # gameover(b, ts)
        # print("Fetching sorter()")
        # sorter(b, allm, 1, 3, ts, 0, replica, 0, scores)
        # print("Fetching minimax()")
        # minimax(b, 0, True, 0, 0, allm, tt, ts, 0, zobra, replica, scores)
        # print("Fetching ai_turn()")
        print("Compiling...")
        ai_turn(b, allm, tt, 2, ts, zobra, replica, scores)
        print("Done.")
        compiler.destroy()
        newgame()
    
    geo(compiler, 150, 50)
    tk.Label(compiler, text="Loading...", fg="#fff", bg = "#222", font=font.Font(family="Tahoma", size=13, slant="italic")).pack(expand=True)
    compiler.config(bg='#222')
    compiler.after(100, precomp)
    compiler.resizable(False, False)
    compiler.mainloop()

def start(_size, _depth):
    global b, size, depth, tt, zobra, replica, allm, p, scores
    size = _size
    depth = _depth
    tt = np.zeros(2**27, dtype=np.uint64)
    zobra = np.random.randint(0, 2**64-1, size=(3, size, size), dtype=np.uint64)
    scores = np.zeros(6*size, dtype=np.int16)

    replica = np.full((size, size), 255, dtype=np.uint8)
    for i in range(size):
        replica[1, i] = 60
        replica[2, i] = 40
        replica[3, i] = 25

    for i in range(4, size):
        for j in range ((size+1)//2):
            val = max(3, -i -j +size+size//2 +5 )
            replica [i, j] = val
            replica [i, size-j-1] = val

    
    #print(replica)
    b = np.array([[n]*size for n in [2]*2+[0]*(size-4)+[1]*2], dtype=np.uint8)
    allm = np.zeros((depth, 6*size+1), dtype=np.uint16)
    p = 0, 0, 0, 0

def newgame():
    global thinking, depth, size, win
    if win:
        win.destroy()
    thinking = True
    conf = tk.Tk()
    conf.config(bg='#222')
    geo(conf, 200, 200)

    tk.Label(conf, text="depth:", font=("Arial", 10), bg="#222", fg="white")\
        .grid(row=0, column=0, padx=10, pady=5, sticky="e")
    depthpick = tk.Spinbox(conf, from_=1, to=9, width=2, relief="sunken", 
                           repeatdelay=500, repeatinterval=100,
                           font=("Arial", 10), bg="white", fg="black")
    depthpick.grid(row=0, column=1, padx=10, pady=5, sticky="w")
    depthpick.delete(0, "end")
    depthpick.insert(0, "7")

    tk.Label(conf, text="size:", font=("Arial", 10), bg="#222", fg="white")\
        .grid(row=1, column=0, padx=10, pady=5, sticky="e")
    sizepick = tk.Spinbox(conf, from_=5, to=16, width=2, relief="sunken",
                          font=("Arial", 10), bg="white", fg="black")
    sizepick.grid(row=1, column=1, padx=10, pady=5, sticky="w")
    sizepick.delete(0, "end")
    sizepick.insert(0, "8")

    tk.Label(conf, text="AI first move:", font=("Arial", 10), bg="#222", fg="white")\
        .grid(row=2, column=0, padx=10, pady=5, sticky="e")
    option_var = tk.BooleanVar()
    tk.Checkbutton(conf, variable=option_var, bg="#222", activebackground="#222",
                   selectcolor="#444", fg="white", font=("Arial", 10))\
        .grid(row=2, column=1, padx=10, pady=5, sticky="w")
    
    def begin():
        try:
            s = int(sizepick.get())
            d = int(depthpick.get())
            if (not 5<=s<=16) or (not 1<=d<=9):
                raise Exception
            start(s, d)
            conf.destroy()
            if option_var.get():
                moves(b, 1, allm, 0, size)
                m = unpack(allm[0, random.randint(0, allm[0, -1] - 1)])
                b[m[0], m[1]] = 0
                b[m[0]+1, m[1]+m[2]] = 2
            window()
        except:
            messagebox.showwarning('Error',"Size must be 5-16.\nDepth must be 1-9.")
    
    ok_btn = tk.Button(conf, text="OK", font=("Arial", 10), bg="#555", fg="white", width=20,
                       command=begin)
    ok_btn.grid(row=3, column=0, columnspan=2, padx=10, pady=10)

    conf.grid_columnconfigure(0, weight=1)
    conf.grid_columnconfigure(1, weight=1)

    conf.mainloop()

def window():
    global select, thinking, win, btns, menubar
    select = False
    thinking = False
    win = tk.Tk()
    
    menubar = tk.Menu(win)
    menus = tk.Menu(menubar)
    menubar.add_cascade(label="Time: 0.00", menu=menus)
    menubar.add_cascade(label="Score: 0", menu=menus)
    menubar.add_command(label="New game", command=newgame)
    win.config(menu=menubar)

    win.config(bg='#000')
    win.title(f"Breakthrough - {size}x{size} - Depth {depth}")
    win.resizable(False, False)
    win.update()
    wsize = size*61 - 1
    geo(win, wsize, wsize)
    win.focus_force()
    text = '◒◓'
    btns=[]
    for i in range(size):
        win.grid_rowconfigure(i, weight=1)
        win.grid_columnconfigure(i, weight=1)

    for i in range(size):
        btns_temp = []
        for j in range(size):
            frame = tk.Frame(win, bg='#000', width=160, height=160)
            frame.grid(row=i, column=j, sticky='nsew')
            frame.grid_propagate(False)

            btn = tk.Button(frame, bg='#000', fg='#000', borderwidth=1, anchor='s', padx=0, pady=0,highlightthickness=0, text = '', font=font.Font(family="Arial", size=44, weight="bold", underline=True))
            btn.config(command=lambda r=i, c=j:click(r, c))
            btn.pack(expand=True, fill='both')
            btns_temp.append(btn)

        btns.append(btns_temp)
    draw()
    win.after(100, win.focus_force)
    win.mainloop()

def click(r, c):
    def initiate():
        global thinking, p
        timer = time.perf_counter()
        p = ai_turn(b, allm, tt, depth, size, zobra, replica, scores)
        menubar.entryconfig(1, label=f"Time: {time.perf_counter()-timer:.2f}")
        menubar.entryconfig(2, label=f"Score: {state(b, size, replica)}")
        if p[2]>p[0]:
            b[p[0], p[1]] = 20
            b[p[2], p[3]] = 22
            col(p[0], p[1])
            col(p[2], p[3])
        
        thinking = False
        draw()

    global select, thinking, p
    if thinking or gameover(b, size):
        return
    if p[2]>p[0]:
        b[p[0], p[1]] = 0
        b[p[2], p[3]] = 2
        col(p[0], p[1])
        col(p[2], p[3])
    if not select:
        if b[r, c]==1:
            select = [r, c]
            flipall(*select, 1)
            # win.after(1000, lambda: print(b))
            draw()
    else:
        flipall(*select, 1)
        if b[r, c]==1:
            flipall(*select, 0)
            select = [r, c]
            flipall(*select, 1)
        elif (b[r, c]) in {12,10}:
            flipall(*select, 0)
            b[*select] = 0
            col(*select)
            b[r, c] = 1
            col(r, c)
            select = False
            thinking = True
            if not gameover(b, size):
                win.after(50, initiate)
            
        else:
            flipall(*select, 0)
            select = False
    # print (b)


def flip(i, j, up):
    b[i, j] = (b[i, j]%10 + 10) if up else b[i,j]%10
    col(i, j)

def flipall(r, c, up):
    flip(r, c, up)
    if c>0 and b[r-1, c-1]%10!=1:
        flip(r-1, c-1, up)
    if c<size-1 and b[r-1, c+1]%10!=1:
        flip(r-1, c+1, up)
    if b[r-1, c]%10==0:
        flip(r-1, c  , up)
    draw()


def col(i, j):
    val = b[i,j]
    btns[i][j].config(text='◈', fg='#d04' if val%10==2 else '#05d' if val%10==1 else '#411' if val==20 else '#333' if (i+j)%2 else '#222', bg= '#442' if val==11 else '#353' if val in {10, 12} else '#522' if val>15 else '#333' if (i+j)%2 else '#222')

def draw():
    for i in range(size):
        for j in range(size):
            col(i, j)

@njit(inline='always')
def zh(b, size, zobra):
    h = np.uint64(0)
    for i in range(size):
        for j in range(size):
            h ^= zobra[b[i, j], i, j]
    return h

@njit(inline='always')
def pack(r, c, move_type):
    move_rep = np.uint16(0) 
    move_rep |= r
    move_rep |= c << 4
    move_rep |= (move_type+1) << 8
    return np.uint16(move_rep)

@njit(inline='always')
def unpack(move_int):
    r = move_int & 0xf
    c = (move_int >> 4) & 0xf
    move_type = (move_int >> 8) & 0xff
    return r, c, move_type - 1

@njit(inline='always')
def pack_tt(uphash, score, depth, flag, best_move):
    result = np.uint64(0)
    result |= best_move
    result |= flag << 16
    result |= depth << 18
    result |= score + 131072 << 22
    result |= uphash << 40
    return np.uint64(result)

@njit(inline='always')
def unpack_tt(tt_int):

    best_move = np.uint16(tt_int & 0xffff)
    flag = np.uint8((tt_int >> 16) & 0x3)
    depth = np.uint8((tt_int >> 18) & 0xf)
    score = np.int16(((tt_int >> 22) & 0x3ffff) - 131072)
    uphash = np.uint32((tt_int >> 40) & 0xffffffff)
    return uphash, score, depth, flag, best_move

@njit
def moves(b, turn, allm, d, size):
    total = 0
    delta = turn * 2 - 1
    owned = 1 + turn
    for i in range(1,size):
        i = (size-1)*turn -i*delta
        pos = i+delta
        for j in range(size):
            # j = size - 1 -j
            j = (j%2)*(size-1) - (j//2)*(j%2-(j+1)%2)
            if b[i, j] != owned:
                continue
            if j>0 and b[pos, j-1] != owned:
                allm[d, total] = pack(i, j, -1)
                total += 1
            if j<size-1 and b[pos, j+1] != owned:
                allm[d, total] = pack(i, j, +1)
                total += 1
            if b[pos, j]==0:
                allm[d, total] = pack(i, j,  0)
                total += 1
    allm[d, 6*size] = total

@njit
def temp_diff(b, size, replica, r, c, maximize):
    val = b[r, c]
    s = 3 - 2*val

    idx = (size-1)*(val-1) + s*r
    h = -s * replica[idx, c]

    DR = (-1, -1, -1,  1,  1,  1)
    DC = ( 0, -1,  1,  0, -1,  1)

    for k in range(6):
        rr = r + DR[k]
        if 0 <= rr < size:
            cc = c + DC[k]
            if 0 <= cc < size:
                h += s * (b[rr, cc] == val)

    return h



@njit(inline='always')
def move_diff(b, size):
    num = 0
    for i in range(0,size):
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

        for j in range(1, size-1):
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
        
        j = size-1
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

@njit
def state(b, size, replica):
    return move_diff(b, size) + quickeval(b, size, replica)

@njit(inline='always')
def quickeval(b, size, replica):
    h = 0
    for i in range(size):
        for j in range(size):
            val = b[i, j]
            if not val:
                continue
            h = h + (2*val-3) * replica[(size-1)*(val-1) - (2*val-3)*i, j]
    return h

@njit(inline='always')
def gameover(b, size):
    for i in range(size):
        if b[0, i]==1:
            return -400
        if b[size-1, i]==2:
            return 400
    return 0

@njit
def sorter(b, allm, d, total, size, turn, replica, bm, scores):
    delta = 2*turn-1
    for i in range(total):
        r, c, t = unpack(allm[d, i])
        old = b[r+delta, c+t]
        b[r, c] = 0
        b[r+delta, c+t] = turn+1
        scores[i] = quickeval(b, size, replica)
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

@njit
def ai_turn(b, allm, tt, depth, size, zobra, replica, scores):
    win = size
    t = (-1, -1, -1)
    for i in range(size):
        if b[size-2, i]==2:
            win = i
            break
    if win<size-1:
        t = (size-2, win, +1)
    elif win==size-1:
        t = (size-2, win, -1)
    else:
        maxeval = -999
        alfa = -999
        beta = +999

        ndepth = depth-1
        moves(b, 1, allm, ndepth, size)
        h = zh(b, size, zobra)
        _, _, _, _, best_move_tt = unpack_tt(tt[h & ((1 << 26) - 1)])

        
        bm = 0
        if best_move_tt != 0:
            for i in range(allm[ndepth, 6*size]):
                if allm[ndepth, i] == best_move_tt:
                    allm[ndepth, i], allm[ndepth, 0] = allm[ndepth, 0], allm[ndepth, i]
                    bm = 1
                    break
        sorter(b, allm, ndepth, allm[ndepth, 6*size], size, 1, replica, bm, scores)


        for i in range(allm[ndepth, 6*size]):
            m = unpack(allm[ndepth, i])
            if b[m[0], m[1]] != 2:
                continue
            # print('Tested move:',m)
            old = b[m[0]+1, m[1]+m[2]]
            b[m[0], m[1]] = 0
            b[m[0]+1, m[1]+m[2]] = 2
            ev = minimax(b, ndepth, 0, alfa, beta, allm, tt, size, 1, zobra, replica, scores, state(b, size, replica), zh(b, size, zobra))
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

@njit
def minimax(b, depth, maximize, alfa0, beta0, allm, tt, size, deep, zobra, replica, scores, score0, h0):

    alfa = alfa0
    beta = beta0
    best_move = 0
    idx = h0 & ((1 << 26) - 1)
    uphash, score_tt, depth_tt, flag_tt, best_move_tt = unpack_tt(tt[idx])
    if uphash == (h0 >> 40) and depth_tt >= depth:
        if flag_tt == 0:
            return score_tt
        elif flag_tt == 1 and score_tt <= alfa:
            return score_tt
        elif flag_tt == 2 and score_tt >= beta:
            return score_tt
    
    ndepth = depth-1

    moves(b, maximize, allm, ndepth, size)


    bm = 0
    if best_move_tt != 0:
        for i in range(allm[ndepth, 6*size]):
            if allm[ndepth, i] == best_move_tt:
                allm[ndepth, i], allm[ndepth, 0] = allm[ndepth, 0], allm[ndepth, i]
                bm = 1
                break
    if deep<3:
        sorter(b, allm, ndepth, allm[ndepth, 6*size], size, maximize, replica, bm, scores)

    if maximize:
        maxeval = -999
        for i in range(allm[ndepth, 6*size]):
            r, c, mt = unpack(allm[ndepth, i])
            r2 = r+1
            if r2 == size-1:
                ev = 400
            else:
                c2 = c+mt
                old = b[r2, c2]
                score = score0 - temp_diff(b, size, replica, r, c, maximize)
                if old!=0:
                    score -= temp_diff(b, size, replica, r2, c2, maximize)
                b[r, c] = 0
                b[r2, c2] = 2
                score = score + temp_diff(b, size, replica, r2, c2, maximize)
                if ndepth==0:
                    ev = state(b, size, replica)
                else:
                    h = h0
                    h ^= zobra[2, r, c]
                    h ^= zobra[0, r, c]
                    h ^= zobra[old, r2, c2]
                    h ^= zobra[2, r2, c2]
                    ev = minimax(b, ndepth, 0, alfa, beta, allm, tt, size, deep+1, zobra, replica, scores, score, h)
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
        mineval = +999
        for i in range(allm[ndepth, 6*size]):
            r, c, mt = unpack(allm[ndepth, i])
            r2 = r-1
            if r2 == 0:
                ev = -400
            else:
                c2 = c+mt
                old = b[r2, c2]
                score = score0 - temp_diff(b, size, replica, r, c, maximize)
                if old!=0:
                    score -= temp_diff(b, size, replica, r2, c2, maximize)
                b[r, c] = 0
                b[r2, c2] = 1
                score = score + temp_diff(b, size, replica, r2, c2, maximize)
                if ndepth == 0:
                    ev = state(b, size, replica)
                else:
                    h = h0
                    h ^= zobra[1, r, c]
                    h ^= zobra[0, r, c]
                    h ^= zobra[old, r2, c2]
                    h ^= zobra[1, r2, c2]
                    ev = minimax(b, ndepth, 1, alfa, beta, allm, tt, size,deep+1, zobra, replica, scores, score, h)
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
    tt[idx] = pack_tt(h0 >> 40, val, depth, flag, best_move)
    return val

def bench():
    start(8, 6)

    # warmup
    ai_turn(b, allm, tt, depth, size, zobra, replica, scores)

    t0 = time.perf_counter()

    for _ in range(30):
        ai_turn(b, allm, tt, depth, size, zobra, replica, scores)

    t1 = time.perf_counter()
    print("Time:", t1 - t0)


if __name__ == "__main__":
    import sys
    if len(sys.argv) > 1 and sys.argv[1] == "bench":
        bench()
    else:
        compile()

