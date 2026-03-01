import tkinter as tk
from tkinter import font, messagebox
import time
import numpy as np
import random

from logic import moves, unpack  # Keep for moves() which doesn't have C++ version yet
import breakthrough_engine


def geo(tkw, w, h):
    tkw.geometry(f"{w}x{h}+{(tkw.winfo_screenwidth()-w)//2}+{(tkw.winfo_screenheight()-h-25)//2}")

def compile():
    global win
    win = False
    compiler = tk.Tk()

    def precomp():
        global depth
        start(8, 3)
        print("Compiling...")
        # ai_turn(b, allm, tt, 2, zobra, replica, scores)
        breakthrough_engine.init_board()
        breakthrough_engine.set_board(b)
        breakthrough_engine.ai_turn(2)
        print("Done.")
        compiler.destroy()
        depth = 8
        newgame()
    
    geo(compiler, 150, 50)
    tk.Label(compiler, text="Loading...", fg="#fff", bg = "#222", font=font.Font(family="Tahoma", size=12)).pack(expand=True)
    compiler.config(bg='#222')
    compiler.after(100, precomp)
    compiler.resizable(False, False)
    compiler.mainloop()

def start(_size, _depth):
    global b, size, depth, tt, zobra, replica, allm, p, scores
    size = _size
    depth = _depth
    tt = np.zeros(2**27, dtype=np.uint64)
    zobra = np.ascontiguousarray(np.random.randint(0, 2**64-1, size=(3, size, size), dtype=np.uint64))
    scores = np.zeros(6*size, dtype=np.int16)

    replica = np.full((size, size), 99, dtype=np.uint8)
    for i in range(size):
        cs = 1 if i in (0, 2, 3, 4, 5, 7) else 0
        # replica[1, i] = 14 + cs
        # replica[2, i] = 11 + cs
        # replica[3, i] = 9 + cs
        # replica[4, i] = 7 + cs
        # replica[5, i] = 6 + cs
        # replica[6, i] = 5 + cs
        # replica[7, i] = 4 + cs

        replica[1, i] = 9 + cs
        replica[2, i] = 7 + cs
        replica[3, i] = 5 + cs
        replica[4, i] = 4 + cs
        replica[5, i] = 3 + cs
        replica[6, i] = 2 + cs
        replica[7, i] = 2 + cs

    # for i in range(4, size):
    #     for j in range ((size+1)//2):
    #         val = max(1, -i +size+size//2 -1 )
    #         replica [i, j] = val
    #         replica [i, size-j-1] = val
    print(replica)

    
    #print(replica)
    b = np.ascontiguousarray([[n]*size for n in [2]*2+[0]*(size-4)+[1]*2], dtype=np.uint8)
    allm = np.zeros((depth, 64), dtype=np.uint8)
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
    depthpick.insert(0, f"{depth}")


    tk.Label(conf, text="AI first move:", font=("Arial", 10), bg="#222", fg="white")\
        .grid(row=2, column=0, padx=10, pady=5, sticky="e")
    option_var = tk.BooleanVar()
    tk.Checkbutton(conf, variable=option_var, bg="#222", activebackground="#222",
                   selectcolor="#444", fg="white", font=("Arial", 10))\
        .grid(row=2, column=1, padx=10, pady=5, sticky="w")
    
    def begin():
        try:
            d = int(depthpick.get())
            start(8, d)
            conf.destroy()
            if option_var.get():
                moves(b, 1, allm, 0)
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
    select = []
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
    win.after(1, lambda: geo(win, wsize, wsize))
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

            btn = tk.Button(frame, bg='#000', fg='#000', borderwidth=1, anchor='center', padx=0, pady=0,highlightthickness=0, text = '', font=font.Font(family="Arial", size=39, underline=True))
            btn.config(command=lambda r=i, c=j:click(r, c))
            btn.pack(expand=True, fill='both')
            btns_temp.append(btn)

        btns.append(btns_temp)
    geo(win, wsize, wsize)
    draw()
    win.after(100, win.focus_force)
    win.mainloop()

def click(r, c):
    def initiate():
        global thinking, p
        timer = time.perf_counter()
        # p = ai_turn(b, allm, tt, depth,  zobra, replica, scores)
        breakthrough_engine.set_board(b)
        p = breakthrough_engine.ai_turn(depth)
        menubar.entryconfig(1, label=f"Time: {time.perf_counter()-timer:.2f}")
        menubar.entryconfig(2, label=f"Score: {breakthrough_engine.state()}")
        if p[2]>p[0]:
            b[p[0], p[1]] = 20
            b[p[2], p[3]] = 22
            col(p[0], p[1])
            col(p[2], p[3])
        
        thinking = False
        draw()

    global select, thinking, p
    if thinking or breakthrough_engine.gameover():
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
            select = []
            thinking = True
            if not breakthrough_engine.gameover():
                win.after(50, initiate)
            
        else:
            flipall(*select, 0)
            select = []
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
    for i in range(8):
        for j in range(8):
            col(i, j)

def bench():
    start(8, 6)
    
    breakthrough_engine.init_board()
    breakthrough_engine.set_board(b)

    # warmup
    # ai_turn(b, allm, tt, depth,  zobra, replica, scores)
    breakthrough_engine.ai_turn(depth)

    t0 = time.perf_counter()

    for _ in range(30):
        breakthrough_engine.set_board(b)
        # ai_turn(b, allm, tt, depth, zobra, replica, scores)
        breakthrough_engine.ai_turn(depth)

    t1 = time.perf_counter()
    print("Time:", t1 - t0)

