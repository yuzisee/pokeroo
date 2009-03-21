
#for argv
import sys

#for threading.Thread
import threading

#for windows
import Tkinter

#for Popen
import subprocess

#for os.read
import os

#for sleep
import time

class SubProcessThread(threading.Thread):
    MAXIMUM_BYTE_READ = 2

    def __init__(self,bytestream,returncode_test,text_append_callback):
        threading.Thread.__init__(self)
        self._fd = bytestream.fileno()
        self._stopreading_test = returncode_test
        self._txt_callback = text_append_callback

    def run(self):
        #From http://mail.python.org/pipermail/python-list/2007-June/618721.html
        time.sleep(1)
        while (self._stopreading_test() is None):
            #print str(self._fd) + "performing read " + repr(self._stopreading_test())
            #try:
                #time.sleep(1.0/24.0) # read at most x times / sec
            next_char = os.read(self._fd, SubProcessThread.MAXIMUM_BYTE_READ)
            #print str(self._fd) + "received [" + repr(next_char) + "]"
            if not next_char is None:
                self._txt_callback(next_char)
            #except Exception:
            #    print "CONSOLESEPARATE: abort "

        print "CONSOLESEPARATE: read terminate"

        #txt =
#

#http://www.pythonware.com/library/
#http://effbot.org/tkinterbook/
class ScrollableText(Tkinter.Frame):
    def __init__(self,parent):
        Tkinter.Frame.__init__(self,parent)
        self._my_yscrollbar = Tkinter.Scrollbar(self)
        self._my_xscrollbar = Tkinter.Scrollbar(self,orient=Tkinter.HORIZONTAL)


        self._my_text = Tkinter.Text(self,height=0,yscrollcommand=self._my_yscrollbar.set,xscrollcommand=self._my_xscrollbar.set)
        self._my_yscrollbar.config(command=self._my_text.yview)
        self._my_xscrollbar.config(command=self._my_text.xview)

        self.config(relief=self._my_text.cget("relief"))
        self._my_text.config(relief=Tkinter.FLAT)


    def set_font(self,new_font):
        self._my_text.configure(font=new_font)

    def add_text(self,new_text):
        self._my_text.configure(state=Tkinter.NORMAL)
        self._my_text.insert(Tkinter.END,new_text)
        self._my_text.configure(state=Tkinter.DISABLED)
        self._my_text.see(Tkinter.END)

    def clear_text(self):
        self._my_text.configure(state=Tkinter.NORMAL)
        self._my_text.delete(0,Tkinter.END)
        self._my_text.configure(state=Tkinter.DISABLED)

    def setup_geometry_manager(self):
        self._my_yscrollbar.pack(side=Tkinter.RIGHT, fill=Tkinter.Y)
        self._my_xscrollbar.pack(side=Tkinter.BOTTOM, fill=Tkinter.X)
        self._my_text.pack(fill=Tkinter.BOTH,expand=1)

class HistoryText(ScrollableText):
    def __init__(self, parent):
        ScrollableText.__init__(self,parent)

    def bind_label(self,source_text):
        self.my_latest = source_text

    def rotate_label(self):
        self.add_text(self.my_latest.get())
        self.my_latest.clear_text()

class UserEntry(Tkinter.Entry):
    def __init__(self, parent, **options):
        Tkinter.Entry.__init__(self,parent,options)
        self.insert(0, "Press [Enter] to begin")

    def setup_geometry_manager(self):
        self.pack(side=Tkinter.BOTTOM,fill=Tkinter.X,expand=0)

    def bind_input_output(self,app_stdin,input_output_pairs):
        self._label_histories = input_output_pairs
        self._app_stdin = app_stdin
        self.bind("<Return>",self.capture_return_event)

        self.uninitialized = True

        self.writer_semaphore = threading.Semaphore(0)
        return self.writer_semaphore

    def capture_return_event(self,event):

        user_input_string = event.widget.get()
        event.widget.delete(0, Tkinter.END)

        if self.uninitialized:
            print "GO"
            self.uninitialized = False
            for history_frame in self._label_histories:
                self.writer_semaphore.release()
            return

        #Send input text
        self._app_stdin.write(user_input_string + '\r\n')
        self._app_stdin.flush()


        #Rotate label text into history
        for history_frame in self._label_histories:
            self.writer_semaphore.acquire(True)

        for history_frame in self._label_histories:
            history_frame.rotate_label()

        for history_frame in self._label_histories:
            self.writer_semaphore.release()

        self._label_histories[0].add_text(user_input_string + '\n')


class ConsoleSeparateWindow(Tkinter.Tk):

    EXPAND_ALL = Tkinter.W+Tkinter.E+Tkinter.N+Tkinter.S
    EXPAND_BOTTOM_HORIZONTAL =Tkinter.W+Tkinter.E+Tkinter.S

    DEFAULT_CONSOLE_FONT = font=("Lucida Console", 9)

    def __init__(self, my_console_app):
        Tkinter.Tk.__init__(self)

        self._my_subprocess = my_console_app

        self.stdout_history_frame = HistoryText(self)
        self.stderr_history_frame = HistoryText(self)

        self._init_widgets();

    def _init_widgets(self):
        #
        #The root window is composed of four quarters:
        #Left is stdout, Right is stderr
        #Top is history, Bottom is input
        #

        self.stdout_history_frame.set_font(ConsoleSeparateWindow.DEFAULT_CONSOLE_FONT)
        self.stderr_history_frame.set_font(ConsoleSeparateWindow.DEFAULT_CONSOLE_FONT)

        stdout_latest = ScrollableText(self)
        stdout_latest.add_text("a")

        #The input frame contains the stderr latest with an entry field at the bottom
        stderr_input_frame = Tkinter.Frame(self, borderwidth=2, relief=Tkinter.GROOVE)
        stderr_input = UserEntry(stderr_input_frame,relief=Tkinter.SUNKEN,width=0)

        stderr_latest  = ScrollableText(stderr_input_frame)
        stderr_latest.add_text("b")
        stderr_latest.pack(side=Tkinter.TOP,fill=Tkinter.BOTH,expand=1)

        #Grid layout is resizable
        stderr_input.setup_geometry_manager()
        stdout_latest.setup_geometry_manager()
        stderr_latest.setup_geometry_manager()

        self.stdout_history_frame.setup_geometry_manager()
        self.stdout_history_frame.grid(row=0,column=0,sticky=ConsoleSeparateWindow.EXPAND_ALL)
        stdout_latest.grid(row=1,column=0,sticky=ConsoleSeparateWindow.EXPAND_BOTTOM_HORIZONTAL)

        self.stderr_history_frame.setup_geometry_manager()
        self.stderr_history_frame.grid(row=0,column=1,sticky=ConsoleSeparateWindow.EXPAND_ALL)
        stderr_input_frame.grid(row=1,column=1,sticky=ConsoleSeparateWindow.EXPAND_BOTTOM_HORIZONTAL)

        self.grid_columnconfigure(0,weight=1)
        self.grid_columnconfigure(1,weight=1)
        self.grid_rowconfigure(0,weight=1)
        self.grid_rowconfigure(1,weight=1)

        stderr_input.focus_set()

        #Event capture
        self.stderr_history_frame.bind_label(stderr_latest)
        self.stdout_history_frame.bind_label(stdout_latest)

        self.access_semaphore = stderr_input.bind_input_output(self._my_subprocess.stdin,[self.stderr_history_frame,self.stdout_history_frame])
        self.protocol("WM_DELETE_WINDOW", self._destroy_handler)

    def _destroy_handler(self):
        self._my_subprocess.terminate()
        self.destroy()

    def append_stdout(self,append_text):
        self.access_semaphore.acquire(True)
        self.stdout_history_frame.my_latest.add_text(append_text.replace('\r',''))
        self.access_semaphore.release()
        sys.stdout.write(append_text)

    def append_stderr(self,append_text):
        self.access_semaphore.acquire(True)
        self.stderr_history_frame.my_latest.add_text(append_text.replace('\r',''))
        self.access_semaphore.release()




def fake_out(stringme):
    print "STD " + str(stringme)

if __name__=='__main__':
    print os.path.abspath(os.curdir)
    #================================
    #   Execute our child process
    #================================
    #From http://mail.python.org/pipermail/python-list/2007-June/618721.html
    console_app = subprocess.Popen(sys.argv[1:], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,bufsize=1)


    #===============================================
    #   Create the application window via Tkinter
    #===============================================

    root_window = ConsoleSeparateWindow(console_app)

    #===========================================
    #   Bind stdout and stderr to root_window
    #===========================================
    #stdout_capturer = SubProcessThread(console_app.stdout, console_app.poll, root_window.append_stdout)
    #stderr_capturer = SubProcessThread(console_app.stderr, console_app.poll, root_window.append_stderr)
    #stdout_capturer = SubProcessThread(console_app.stdout, console_app.poll, fake_out)
    #stderr_capturer = SubProcessThread(console_app.stderr, console_app.poll, fake_out)

    #==========
    #   Run!
    #==========
    #stdout_capturer.start()
    #stderr_capturer.start()
    root_window.mainloop()