
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
    MAXIMUM_BYTE_READ = 2048

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

class AppendableLabel(Tkinter.Label):
    def __init__(self,parent):
        Tkinter.Label.__init__(self,parent)

        self.configure(anchor=Tkinter.SW,justify=Tkinter.LEFT)

        self._my_str = ""

    def set_font(self,new_font):
        self.configure(font=new_font)

    def add_text(self,new_text):
        self._my_str += new_text
        self.configure(text=self._my_str)

    def push_text(self,destination):
        if len(self._my_str) > 0:
            destination.add_text(self._my_str)
            self._my_str = ""
            self.configure(text="")

#http://www.pythonware.com/library/
#http://effbot.org/tkinterbook/
class ScrollableText(Tkinter.Frame):

    #It looks like Tkinter can't take too many commands at once.
    #It overflows or something...
    TKINTER_SPAM_RELIEF_TIME = 0.04

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
        #print repr(self._my_text)

        self._my_text.configure(state=Tkinter.NORMAL)
        self._my_text.insert(Tkinter.END,new_text)
        self._my_text.configure(state=Tkinter.DISABLED)
        time.sleep(ScrollableText.TKINTER_SPAM_RELIEF_TIME)

    def push_text(self,destination):
        raise AssertionError, "stderr_latest and stdout_latest are labels now"
        #print repr(self._my_text)

        self._my_text.configure(state=Tkinter.NORMAL)
        destination.add_text(self._my_text.get("1.0",Tkinter.END))
        self._my_text.delete("1.0",Tkinter.END)
        self._my_text.configure(state=Tkinter.DISABLED)
        time.sleep(ScrollableText.TKINTER_SPAM_RELIEF_TIME)

    def setup_geometry_manager(self):
        self._my_yscrollbar.pack(side=Tkinter.RIGHT, fill=Tkinter.Y)
        self._my_xscrollbar.pack(side=Tkinter.BOTTOM, fill=Tkinter.X)
        self._my_text.pack(fill=Tkinter.BOTH,expand=1)

    def scroll_down(self):
        self._my_text.configure(state=Tkinter.NORMAL)
        self._my_text.see(Tkinter.END)
        self._my_text.configure(state=Tkinter.DISABLED)
        time.sleep(ScrollableText.TKINTER_SPAM_RELIEF_TIME)

class HistoryText(ScrollableText):
    def __init__(self, parent):
        ScrollableText.__init__(self,parent)

    def bind_label(self,source_text):
        self.my_latest = source_text

    def rotate_label(self):
        self.my_latest.push_text(self)

class UserEntry(Tkinter.Entry):
    def __init__(self, parent, **options):
        Tkinter.Entry.__init__(self,parent,options)
        self.insert(0, "Press [Enter] to begin")

    def setup_geometry_manager(self):
        self.pack(side=Tkinter.BOTTOM,fill=Tkinter.X,expand=0)

    def bind_input_output(self,gui_lock,app_stdin,input_output_pairs):
        self._label_histories = input_output_pairs
        self._app_stdin = app_stdin
        self.bind("<Return>",self.capture_return_event)

        self.uninitialized = True
        self.gui_lock = gui_lock

        self.gui_lock.acquire(True)

    def capture_return_event(self,event):

        if self.uninitialized:
            event.widget.delete(0, Tkinter.END)
            self.uninitialized = False
            self.gui_lock.release()
            return

        self.gui_lock.acquire(True)

        #Send input text
        user_input_string = event.widget.get()
        self._app_stdin.write(user_input_string + '\r\n')
        self._app_stdin.flush()

        event.widget.delete(0, Tkinter.END)

        #Rotate label text into history
        for history_frame in self._label_histories:
            history_frame.rotate_label()

        self._label_histories[0].add_text(user_input_string + '\n')

        for history_frame in self._label_histories:
            history_frame.scroll_down()

        self.gui_lock.release()


class ConsoleSeparateWindow(Tkinter.Tk):

    EXPAND_ALL = Tkinter.W+Tkinter.E+Tkinter.N+Tkinter.S
    EXPAND_BOTTOM_HORIZONTAL =Tkinter.W+Tkinter.E+Tkinter.S

    DEFAULT_CONSOLE_FONT = font=("Lucida Console", 9)


    def __init__(self, my_console_app):
        Tkinter.Tk.__init__(self)

        self._my_subprocess = my_console_app

        self.stdout_history_frame = HistoryText(self)
        self.stderr_history_frame = HistoryText(self)

        #If Tkinter can't even handle a flood of commands from a single thread,
        #I wouldn't trust its ability to be threadsafe at all.
        #To make any GUI changes, you have to lock our GUI.
        self.gui_lock = threading.Lock()

        self._init_widgets();

    def _init_widgets(self):
        #
        #The root window is composed of four quarters:
        #Left is stdout, Right is stderr
        #Top is history, Bottom is input
        #

        self.stdout_history_frame.set_font(ConsoleSeparateWindow.DEFAULT_CONSOLE_FONT)
        self.stderr_history_frame.set_font(ConsoleSeparateWindow.DEFAULT_CONSOLE_FONT)

        stdout_latest = AppendableLabel(self)
        stdout_latest.set_font(ConsoleSeparateWindow.DEFAULT_CONSOLE_FONT)

        #The input frame contains the stderr latest with an entry field at the bottom
        stderr_input_frame = Tkinter.Frame(self, borderwidth=2, relief=Tkinter.GROOVE)
        stderr_input = UserEntry(stderr_input_frame,relief=Tkinter.SUNKEN,width=0)

        stderr_latest  = AppendableLabel(stderr_input_frame)
        stderr_latest.set_font(ConsoleSeparateWindow.DEFAULT_CONSOLE_FONT)

        #Grid layout is resizable
        stderr_input.setup_geometry_manager()

        stderr_latest.pack(side=Tkinter.TOP,fill=Tkinter.BOTH,expand=1)


        self.stdout_history_frame.setup_geometry_manager()
        self.stdout_history_frame.grid(row=0,column=0,sticky=ConsoleSeparateWindow.EXPAND_ALL)
        stdout_latest.grid(row=1,column=0,sticky=ConsoleSeparateWindow.EXPAND_BOTTOM_HORIZONTAL)

        self.stderr_history_frame.setup_geometry_manager()
        self.stderr_history_frame.grid(row=0,column=1,sticky=ConsoleSeparateWindow.EXPAND_ALL)
        stderr_input_frame.grid(row=1,column=1,sticky=ConsoleSeparateWindow.EXPAND_BOTTOM_HORIZONTAL)

        self.grid_columnconfigure(0,weight=1)
        self.grid_columnconfigure(1,weight=1)
        self.grid_rowconfigure(0,weight=1)
        self.grid_rowconfigure(1,weight=0)

        stderr_input.focus_set()

        #Event capture
        self.stderr_history_frame.bind_label(stderr_latest)
        self.stdout_history_frame.bind_label(stdout_latest)

        self.history_frames_list = [self.stderr_history_frame,self.stdout_history_frame]
        stderr_input.bind_input_output(self.gui_lock,self._my_subprocess.stdin,self.history_frames_list)
        self.protocol("WM_DELETE_WINDOW", self._destroy_handler)

    def _destroy_handler(self):

        self.gui_lock.acquire(True)

        #Clean up in a threadsafe manner
        self._my_subprocess.terminate()
        self.stdout_history_frame = None
        self.stderr_history_frame = None
        self.history_frames_list = None

        self.gui_lock.release()

        self.destroy()

    def append_stdout(self,append_text):
        self._synchronous_append(self.stdout_history_frame,append_text.replace('\r',''))
        sys.stdout.write(append_text.replace('\r',''))

    def append_stderr(self,append_text):
        self._synchronous_append(self.stderr_history_frame,append_text.replace('\r',''))

    def _synchronous_append(self,history_frame,append_text):
        self.gui_lock.acquire(True)

        if not history_frame is None:
            history_frame.my_latest.add_text(append_text)

        for f in self.history_frames_list:
            f.scroll_down()

        self.gui_lock.release()




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
    stdout_capturer = SubProcessThread(console_app.stdout, console_app.poll, root_window.append_stdout)
    stderr_capturer = SubProcessThread(console_app.stderr, console_app.poll, root_window.append_stderr)
    #stdout_capturer = SubProcessThread(console_app.stdout, console_app.poll, fake_out)
    #stderr_capturer = SubProcessThread(console_app.stderr, console_app.poll, fake_out)

    #==========
    #   Run!
    #==========
    stdout_capturer.start()
    stderr_capturer.start()
    root_window.mainloop()