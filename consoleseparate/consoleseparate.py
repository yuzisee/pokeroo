
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


class HistoryText(Tkinter.Frame):

    def __init__(self, parent, **options):
        Tkinter.Frame.__init__(self,parent,options)
        self.my_scrollbar = Tkinter.Scrollbar(self)

        self.my_text = Tkinter.Text(self,height=0,yscrollcommand=self.my_scrollbar.set)
        self.my_scrollbar.config(command=self.my_text.yview)

        self.config(relief=self.my_text.cget("relief"))

    def set_font(self,new_font):
        self.my_text.configure(font=new_font)

    def add_text(self,new_text):
        self.my_text.configure(state=Tkinter.NORMAL)
        self.my_text.insert(Tkinter.END,new_text)
        self.my_text.configure(state=Tkinter.DISABLED)
        self.my_text.see(Tkinter.END)

    def setup_geometry_manager(self):
        self.my_scrollbar.pack(side=Tkinter.RIGHT, fill=Tkinter.Y)
        self.my_text.pack(fill=Tkinter.BOTH,expand=1)




def user_input(event):
    print "User pressed" + repr(event.char)


if __name__=='__main__':

    #console_app = SubProcessThread(sys.argv)

    root_window = Tkinter.Tk()
    expand_all =Tkinter.W+Tkinter.E+Tkinter.N+Tkinter.S
    expand_bottom_horizontal =Tkinter.W+Tkinter.E+Tkinter.S

    console_font = font=("Lucida Console", 9)


    #
    #The root window is composed of four quarters:
    #Left is stdout, Right is stderr
    #Top is history, Bottom is input
    #

    stdout_history_frame = HistoryText(root_window)
    stdout_history_frame.set_font(console_font)
    stdout_history_frame.add_text("out t\no\np\n\n\n\n\nd")




    stderr_history_frame = HistoryText(root_window)
    stderr_history_frame.set_font(console_font)
    stderr_history_frame.add_text("err top")


    stdout_latest  = Tkinter.Label(root_window, text="out bottom\n \ne\nr\nr bottom\nnbb\nf err bottom\nnbb\nf", borderwidth=2, relief=Tkinter.GROOVE, font=console_font, justify=Tkinter.LEFT, anchor=Tkinter.W)

    #The input frame contains the stderr latest with an entry field at the bottom
    stderr_input_frame = Tkinter.Frame(root_window, borderwidth=2, relief=Tkinter.GROOVE)


    stderr_input  = Tkinter.Entry(stderr_input_frame, relief=Tkinter.SUNKEN,width=0)
    stderr_input.pack(side=Tkinter.BOTTOM,fill=Tkinter.X,expand=0)

    stderr_latest  = Tkinter.Label(stderr_input_frame, text="err bottom\nnbb\nf", relief=Tkinter.FLAT, font=console_font, justify=Tkinter.LEFT, anchor =Tkinter.W)
    stderr_latest.pack(side=Tkinter.TOP,fill=Tkinter.BOTH,expand=1)

    #Grid layout is resizable
    stdout_history_frame.setup_geometry_manager()
    stdout_history_frame.grid(row=0,column=0,sticky=expand_all)
    stdout_latest.grid(row=1,column=0,sticky=expand_bottom_horizontal)

    stderr_history_frame.setup_geometry_manager()
    stderr_history_frame.grid(row=0,column=1,sticky=expand_all)
    stderr_input_frame.grid(row=1,column=1,sticky=expand_bottom_horizontal)

    root_window.grid_columnconfigure(0,weight=1)
    root_window.grid_columnconfigure(1,weight=1)
    root_window.grid_rowconfigure(0,weight=1)
    root_window.grid_rowconfigure(1,weight=0)

    #Event capture
    stderr_input.bind("<Key>",user_input)

    #Initialize view

    root_window.mainloop()