
#for argv
import sys

#for threading.Thread
import threading

#for windows
import Tkinter

#for Popen
import subprocess

class SubProcessThread(threading.Thread):
    MAXIMUM_BYTE_READ = 2048

    def __init__(self,bytestream,returncode_test,text_append_callback):
        threading.Thread.__init__(self)
        self._fd = bytestream.fileno()
        self._stopreading_test = returncode_test
        self._txt_callback = text_append_callback

    def run():
        #From http://mail.python.org/pipermail/python-list/2007-June/618721.html
        while self._stopreading_test():
            self._txt_callback(os.read(self._fd, MAXIMUM_BYTE_READ))

        #txt =
#        time.sleep(0.5)   # read at most x times / sec

#http://www.pythonware.com/library/
#http://effbot.org/tkinterbook/
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

class UserEntry(Tkinter.Entry):
    def __init__(self, parent, **options):
        Tkinter.Entry.__init__(self,parent,options)

    def setup_geometry_manager(self):
        self.pack(side=Tkinter.BOTTOM,fill=Tkinter.X,expand=0)

    def bind_input_output(self,app_stdin,input_output_pairs):
        self.label_history_pairs = input_output_pairs
        self.app_stdin = app_stdin
        stderr_input.bind("<Return>",self.capture_return_event)

    def capture_return_event(self,event):
#        print "User pressed" + repr(event.char)
#        if event.char == '\r' or event.char == '\n':
        for lh_pair in self.label_history_pairs:
            #print "Input contents" + lh_pair[0].cget("text")#1,Tkinter.END)
            lh_pair[1].add_text(lh_pair[0].cget("text"))
            lh_pair[0].configure(text='')
#        else:
#            for lh_pair in self.label_history_pairs:
#                lh_pair[0].configure(text=lh_pair[0].cget("text")+event.char+'\r\n')
        self.app_stdin.write(event.widget.get())
        event.widget.delete(0, END)


class ConsoleSeparateWindow(Tkinter.Tk):

    EXPAND_ALL = Tkinter.W+Tkinter.E+Tkinter.N+Tkinter.S
    EXPAND_BOTTOM_HORIZONTAL =Tkinter.W+Tkinter.E+Tkinter.S

    DEFAULT_CONSOLE_FONT = font=("Lucida Console", 9)

    def __init__(self, my_console_app, **options):
        Tkinter.Tk.__init__(self,options)

        self._my_subprocess = my_console_app
        self._stdout_latest = None
        self._stderr_latest = None

        self._init_widgets();

    def _init_widgets(self):
        #
        #The root window is composed of four quarters:
        #Left is stdout, Right is stderr
        #Top is history, Bottom is input
        #

        stdout_history_frame = HistoryText(self)
        stdout_history_frame.set_font(DEFAULT_CONSOLE_FONT)
        stdout_history_frame.add_text("out t\no\np\n\n\n\n\nd")
        self._stdout_latest  = Tkinter.Label(self,borderwidth=2,relief=Tkinter.GROOVE, font=DEFAULT_CONSOLE_FONT, justify=Tkinter.LEFT, anchor=Tkinter.W)


        stderr_history_frame = HistoryText(self)
        stderr_history_frame.set_font(DEFAULT_CONSOLE_FONT)
        stderr_history_frame.add_text("err top")

        #The input frame contains the stderr latest with an entry field at the bottom
        stderr_input_frame = Tkinter.Frame(self, borderwidth=2, relief=Tkinter.GROOVE)
        stderr_input = UserEntry(stderr_input_frame,relief=Tkinter.SUNKEN,width=0)

        self._stderr_latest  = Tkinter.Label(stderr_input_frame,relief=Tkinter.FLAT, font=DEFAULT_CONSOLE_FONT, justify=Tkinter.LEFT, anchor =Tkinter.W)
        self._stderr_latest.pack(side=Tkinter.TOP,fill=Tkinter.BOTH,expand=1)

        #Grid layout is resizable
        stderr_input.setup_geometry_manager()

        stdout_history_frame.setup_geometry_manager()
        stdout_history_frame.grid(row=0,column=0,sticky=EXPAND_ALL)
        self._stdout_latest.grid(row=1,column=0,sticky=EXPAND_BOTTOM_HORIZONTAL)

        stderr_history_frame.setup_geometry_manager()
        stderr_history_frame.grid(row=0,column=1,sticky=EXPAND_ALL)
        stderr_input_frame.grid(row=1,column=1,sticky=EXPAND_BOTTOM_HORIZONTAL)

        self.grid_columnconfigure(0,weight=1)
        self.grid_columnconfigure(1,weight=1)
        self.grid_rowconfigure(0,weight=1)
        self.grid_rowconfigure(1,weight=0)

        #Event capture
        stderr_input.bind_input_output([(self._stdout_latest,stdout_history_frame),(self._stderr_latest,stderr_history_frame)])
        self.protocol("WM_DELETE_WINDOW", self._destroy_handler)

    def _destroy_handler(self):
        self._my_subprocess.terminate()
        self.destroy()

    def append_stdout(self,append_text):
        self._stdout_latest.configure(text=self._stdout_latest.cget("text")+append_text)

    def append_stderr(self,append_text):
        self._stderr_latest.configure(text=self._stderr_latest.cget("text")+append_text)


if __name__=='__main__':
    #================================
    #   Execute our child process
    #================================
    #From http://mail.python.org/pipermail/python-list/2007-June/618721.html
    console_app = subprocess.Popen(sys.argv, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,bufsize=1)

    #===============================================
    #   Create the application window via Tkinter
    #===============================================

    root_window = ConsoleSeparateWindow(console_app)

    #===========================================
    #   Bind stdout and stderr to root_window
    #===========================================
    stdout_capturer = SubProcessThread(console_app.stdout, console_app.returncode, root_window.append_stdout)
    stderr_capturer = SubProcessThread(console_app.stderr, console_app.returncode, root_window.append_stderr)

    #Initialize view
    root_window.mainloop()