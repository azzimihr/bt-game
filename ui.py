import tkinter as tk
from tkinter import font, messagebox
import time
import numpy as np
import random

# from old.logic import moves, unpack  # Keep for moves() which doesn't have C++ version yet
import breakthrough_engine

def geo(tkw, w, h):
    tkw.geometry(f"{w}x{h}+{(tkw.winfo_screenwidth()-w)//2}+{(tkw.winfo_screenheight()-h-25)//2}")

def start():
    global b, size
    size = 8
    b = np.ascontiguousarray([[n]*size for n in [2]*2+[0]*(size-4)+[1]*2], dtype=np.uint8)
    breakthrough_engine.init_board()
    breakthrough_engine.set_board(b)

def window():
    global select, thinking, win, btns, menubar, size, b, p
    start()
    p = 0, 0, 0, 0
    size = 8
    select = []
    thinking = False
    win = tk.Tk()
    
    menubar = tk.Menu(win)
    menus = tk.Menu(menubar)
    menubar.add_cascade(label="Time: 0.00", menu=menus)
    menubar.add_cascade(label="Score: 0", menu=menus)
    menubar.add_command(label="New game", command=(lambda: win.destroy() * window()))
    win.config(menu=menubar)

    win.config(bg='#000')
    win.title(f"Breakthrough - {size}x{size} - Depth 10")
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
        breakthrough_engine.set_board(b)
        p = breakthrough_engine.ai_turn()
        menubar.entryconfig(1, label=f"Time: {time.perf_counter()-timer:.2f}s")
        print(f"{time.perf_counter()-timer:.2f}")
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

