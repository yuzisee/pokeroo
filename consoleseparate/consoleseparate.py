
#for argv
import sys

#for threading.Thread
import threading

#for windows
import Tkinter


class SubProcessThread(threading.Thread):
    def __init__(self,args):
        threading.Thread.__init__(self)
        self.app_args = args
        self.stdout_txt = ""
        self.stderr_txt = ""

    def run():
        #From http://mail.python.org/pipermail/python-list/2007-June/618721.html
        doRead = True
        p = subprocess.Popen(self.app_args, stdout=sp.PIPE, stderr=sp.PIPE,bufsize=1)
        while doRead:
            #txt = os.read(p.stdout.fileno(), 2048)
            #txt = os.read(p.stderr.fileno(), 2048)
            time.sleep(0.5)   # read at most x times / sec


#class WindowThread(threading.Thread):





if __name__=='__main__':

    #console_app = SubProcessThread(sys.argv)

    root_window = Tkinter.Tk()
    expand_all =Tkinter.W+Tkinter.E+Tkinter.N+Tkinter.S

    #
    #The root window is composed of four quarters:
    #Left is stdout, Right is stderr
    #Top is history, Bottom is input
    #

    stdout_history = Tkinter.Label(root_window, text="out top", relief=Tkinter.SUNKEN)
    stdout_latest  = Tkinter.Label(root_window, text="out bottom", relief=Tkinter.SUNKEN)

    stderr_history = Tkinter.Label(root_window, text="err top", relief=Tkinter.SUNKEN)


    #The input frame contains the stderr latest with an entry field at the bottom
    stderr_input_frame = Tkinter.Frame(root_window)


    stderr_input  = Tkinter.Entry(stderr_input_frame, relief=Tkinter.RAISED,width=0)
    stderr_input.pack(side=Tkinter.BOTTOM,fill=Tkinter.X,expand=0)

    stderr_latest  = Tkinter.Label(stderr_input_frame, text="err bottom", relief=Tkinter.SUNKEN)
    stderr_latest.pack(side=Tkinter.TOP,fill=Tkinter.BOTH,expand=1)

    #Grid layout is resizable
    stdout_history.grid(row=0,column=0,sticky=expand_all)
    stdout_latest.grid(row=1,column=0,sticky=expand_all)
    stderr_history.grid(row=0,column=1,sticky=expand_all)
    stderr_input_frame.grid(row=1,column=1,sticky=expand_all)

    root_window.grid_columnconfigure(0,weight=1)
    root_window.grid_columnconfigure(1,weight=1)
    root_window.grid_rowconfigure(0,weight=1)
    root_window.grid_rowconfigure(1,weight=1)

    root_window.mainloop()