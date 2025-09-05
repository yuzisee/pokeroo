#!/usr/bin/env python3

import collections.abc

# needed by queue.Queue
import queue

# needed for argv
import sys

# needed by threading.Thread
import threading

# needed to draw windows on the screen
import tkinter
# needed by tkinter.font.Font
import tkinter.font

#for Popen
import subprocess

#for os.read, os.getcwd, os.linesep
import os

#for sleep
import time

import re

class SubProcessThread(threading.Thread):
    MAXIMUM_BYTE_READ = 2048

    def __init__(self,bytestream,returncode_test,text_append_callbacks: list[collections.abc.Callable[[str], None]]):
        threading.Thread.__init__(self)
        self._fd = bytestream.fileno()
        self._stopreading_test = returncode_test
        self._txt_callbacks = text_append_callbacks

    def run(self):
        #From http://mail.python.org/pipermail/python-list/2007-June/618721.html
        time.sleep(1)
        while (self._stopreading_test() is None):
            #print str(self._fd) + "performing read " + repr(self._stopreading_test())
            #try:
                #time.sleep(1.0/24.0) # read at most x times / sec
            next_char = os.read(self._fd, SubProcessThread.MAXIMUM_BYTE_READ)
            #print str(self._fd) + "received [" + repr(next_char) + "]"
            if next_char:
                for cb in self._txt_callbacks:
                    # [!TIP]
                    # 'utf-8' is a superset of ASCII so it's safe to do this.
                    # Furthermore, when we set `self.POKER_REPLACE` we need to append unicode characters into this string so having it as Unicode here is best.
                    cb(next_char.decode('utf-8', 'strict'))
            #except Exception:
            #    print "CONSOLESEPARATE: abort "

        print(str(self._fd) + "CONSOLESEPARATE: read terminate")

        #txt =
#

class AppendableLabel(tkinter.Label):
    def __init__(self,parent, POKER_REPLACE=False):
        tkinter.Label.__init__(self,parent)

        self.configure(anchor=tkinter.SW,justify=tkinter.LEFT)

        self._my_str = ""
        self.POKER_REPLACE = POKER_REPLACE

    def set_font(self,new_font):
        self.configure(font=new_font)

    def add_text(self,new_text):
        if self.POKER_REPLACE:
            new_text = re.sub(r'\b[2-9TJQKA][cdhs]\b', lambda m: m.group(0).replace('s', u'\u2664').replace('h', u'\u2661').replace('c', u'\u2663').replace('d', u'\u2662'), new_text)
        self._my_str += new_text
        self.configure(text=self._my_str)

    def push_text(self,destination):
        if len(self._my_str) > 0:
            destination.add_text(self._my_str)
            self._my_str = ""
            self.configure(text="")

#http://www.pythonware.com/library/
#http://effbot.org/tkinterbook/
class ScrollableText(tkinter.Frame):

    #It looks like Tkinter can't take too many commands at once.
    #It overflows or something...
    TKINTER_SPAM_RELIEF_TIME = 0.04

    def __init__(self,parent):
        tkinter.Frame.__init__(self,parent)
        self._my_yscrollbar = tkinter.Scrollbar(self)
        self._my_xscrollbar = tkinter.Scrollbar(self,orient=tkinter.HORIZONTAL)


        self._my_text = tkinter.Text(self,height=1,yscrollcommand=self._my_yscrollbar.set,xscrollcommand=self._my_xscrollbar.set)
        self._my_yscrollbar.config(command=self._my_text.yview)
        self._my_xscrollbar.config(command=self._my_text.xview)

        self.config(relief=self._my_text.cget("relief"))
        self._my_text.config(relief=tkinter.FLAT)


    def set_font(self,new_font):
        self._my_text.configure(font=new_font)

    def add_text(self,new_text):
        #print repr(self._my_text)

        self._my_text.configure(state=tkinter.NORMAL)
        self._my_text.insert(tkinter.END,new_text)
        self._my_text.configure(state=tkinter.DISABLED)
        time.sleep(ScrollableText.TKINTER_SPAM_RELIEF_TIME)

    def push_text(self,destination):
        raise AssertionError("stderr_latest and stdout_latest are labels now")
        #print repr(self._my_text)

        self._my_text.configure(state=tkinter.NORMAL)
        destination.add_text(self._my_text.get("1.0",tkinter.END))
        self._my_text.delete("1.0",tkinter.END)
        self._my_text.configure(state=tkinter.DISABLED)
        time.sleep(ScrollableText.TKINTER_SPAM_RELIEF_TIME)

    def setup_geometry_manager(self):
        self._my_yscrollbar.pack(side=tkinter.RIGHT, fill=tkinter.Y)
        self._my_xscrollbar.pack(side=tkinter.BOTTOM, fill=tkinter.X)
        self._my_text.pack(fill=tkinter.BOTH,expand=1)

    def scroll_down(self):
        #Let it settle down so that we know how far we actually need to scroll
        time.sleep(ScrollableText.TKINTER_SPAM_RELIEF_TIME)
        self._my_text.configure(state=tkinter.NORMAL)
        self._my_text.see(tkinter.END)
        self._my_text.configure(state=tkinter.DISABLED)
        time.sleep(ScrollableText.TKINTER_SPAM_RELIEF_TIME)


class HistoryText(ScrollableText):
    def __init__(self, parent):
        ScrollableText.__init__(self,parent)

    def bind_label(self,source_text):
        self.my_latest = source_text

    def rotate_label(self):
        self.my_latest.push_text(self)

class UserEntry(tkinter.Entry):
    """A textbox that responds specially to [Return] button presses

    When [Return] is pressed: (This is configured once in self.bind_input_output)
      + Send the contents of the textbox to our subprocess.Popen's stdin.
      + We ask all the history frames to rotate themselves.
      + We append the user's input to the stdin_history and clear it from the textbox

    """
    def __init__(self, parent, **options):
        tkinter.Entry.__init__(self,parent, **options)
        self.insert(0, "Press [Enter] to begin")

    def setup_geometry_manager(self):
        """When called, we will attach ourselves to the bottom of our parent widget, and expand horizontally to fill"""
        self.pack(side=tkinter.BOTTOM,fill=tkinter.X,expand=0)

    def on_first_input(self, callback):
        self._on_initialize = callback

    def bind_input_output(self,gui_lock,app_stdin,input_output_pairs):
        self._label_histories = input_output_pairs
        self._app_stdin = app_stdin
        self.bind("<Return>",self.capture_return_event)

        self.uninitialized = True
        self.gui_lock = gui_lock

        # self.gui_lock.acquire(True)
        # To be released on the first capture_return_event, see self.uninitialized
        # TODO(from yuzisee): Why did we have this?

    def enable(self):
        with self.gui_lock:
            self.config(state=tkinter.NORMAL)

    def capture_return_event(self,event):

        # Capture the first enter key.
        if self.uninitialized:
            with self.gui_lock: # TODO(from yuzisee): This should be just fine. Why did we carry the lock across bind_input_output --> capture_return_event before?
                event.widget.delete(0, tkinter.END)
            self._on_initialize()
            self.uninitialized = False
            #self.gui_lock.release()
            return


        with self.gui_lock:

            #Send input text
            user_input_string = event.widget.get()
            user_input_send_chars = user_input_string.strip() + os.linesep
            self._app_stdin.write(user_input_send_chars.encode('ascii'))
            self._app_stdin.flush()

            event.widget.delete(0, tkinter.END)

            #Rotate label text into history
            for history_frame in self._label_histories:
                history_frame.rotate_label()

            self._label_histories[0].add_text(user_input_string + '\n')

            for history_frame in self._label_histories:
                history_frame.scroll_down()

            event.widget.config(state=tkinter.DISABLED)


#Portable font choices: http://wiki.tcl.tk/451
def default_gamelog_font():
    return tkinter.font.Font(family="Helvetica", size=11)

def default_console_font():
    #    DEFAULT_CONSOLE_FONT = font=("Lucida Console", 9)
    #DEFAULT_CONSOLE_FONT = font=("Courier", 9)
    return tkinter.font.Font(family="Monaco", size=9)

def default_input_font():
    return tkinter.font.Font(family="Helvetica", size=9, weight="bold")
    #    DEFAULT_INPUT_FONT = font=("Courier New", 9,"bold")
    #DEFAULT_INPUT_FONT = font=("Courier", 9,"bold")

class ConsoleSeparateWindow(tkinter.Tk):
    """This is the window itself. The left side contains stdout, the right side contains stderr and echos stdin.

    One line of stdin input is available at a time in a tkinter.Entry at the bottom of the right side.

    """
    stdout_history_frame: HistoryText
    stderr_history_frame: HistoryText
    stderr_input: UserEntry
    _gamelog_font: tkinter.font.Font
    _console_font: tkinter.font.Font
    _input_font: tkinter.font.Font

    EXPAND_ALL = tkinter.W+tkinter.E+tkinter.N+tkinter.S

    def __init__(self, my_console_app):
        """Constructor

        Parameters:
          my_console_app:
            (subprocess.Popen) The app whose stdout/stderr/stdin we are interacting with.

        """

        tkinter.Tk.__init__(self)

        self._my_subprocess = my_console_app

        self._stdout_queue = queue.Queue()
        self._stderr_queue = queue.Queue()

        #If Tkinter can't even handle a flood of commands from a single thread,
        #I wouldn't trust its ability to be threadsafe at all.
        #To make any GUI changes, you have to lock our GUI.
        self.gui_lock = threading.Lock()

        self._init_widgets();

    def _init_widgets(self):
        """Layout all our widgets and connect them up as needed. This is a helper function for __init__"""
        #
        #The root window is composed of four quarters:
        #Left is stdout, Right is stderr
        #Top is history, Bottom is input
        #
        self._gamelog_font = default_gamelog_font()
        self._console_font = default_console_font()
        self._input_font = default_input_font()

        left_column = tkinter.Frame(self, background='orange') # Create a frame and attach it to the parent
        left_column.grid(row=0, column=0, sticky=ConsoleSeparateWindow.EXPAND_ALL)
        right_column = tkinter.Frame(self, background='green') # Create a frame and attach it to the parent
        right_column.grid(row=0, column=1, sticky=ConsoleSeparateWindow.EXPAND_ALL)

        self.stdout_history_frame = HistoryText(left_column)
        self.stderr_history_frame = HistoryText(right_column)

        self.stdout_history_frame.set_font(self._gamelog_font)
        self.stderr_history_frame.set_font(self._gamelog_font)

        stdout_latest = AppendableLabel(left_column, POKER_REPLACE=True)
        stdout_latest.set_font(self._gamelog_font)

        #The input frame contains the stderr latest with an entry field at the bottom
        stderr_input_frame = tkinter.Frame(right_column, borderwidth=2, relief=tkinter.GROOVE, background='yellow')
        self.stderr_input = UserEntry(stderr_input_frame,relief=tkinter.SUNKEN,width=0,font=self._input_font)

        stderr_latest  = AppendableLabel(stderr_input_frame)
        stderr_latest.set_font(self._console_font)

        #Grid layout is resizable
        self.stderr_input.setup_geometry_manager()

        stderr_latest.pack(side=tkinter.TOP,fill=tkinter.BOTH,expand=1)


        self.stdout_history_frame.setup_geometry_manager()
        self.stdout_history_frame.grid(row=0, column=0, sticky=ConsoleSeparateWindow.EXPAND_ALL)
        stdout_latest.grid(row=1, column=0, sticky=ConsoleSeparateWindow.EXPAND_ALL)

        self.stderr_history_frame.setup_geometry_manager()
        self.stderr_history_frame.grid(row=0, column=0, sticky=ConsoleSeparateWindow.EXPAND_ALL)
        stderr_input_frame.grid(row=1, column=0, sticky=ConsoleSeparateWindow.EXPAND_ALL)

        # Allow resizing
        right_column.grid_columnconfigure(0, weight=1, pad=0)
        right_column.grid_rowconfigure(0, weight=1, pad=0)
        right_column.grid_rowconfigure(1, weight=0, pad=0)
        left_column.grid_columnconfigure(0, weight=1, pad=0)
        left_column.grid_rowconfigure(0, weight=1, pad=0)
        left_column.grid_rowconfigure(1, weight=0, pad=0)
        self.grid_columnconfigure(0, weight=1, pad=0)
        self.grid_columnconfigure(1, weight=1, pad=0)
        self.grid_rowconfigure(0, weight=1, pad=0)

        self.stderr_input.focus_set()

        #Event capture
        self.stderr_history_frame.bind_label(stderr_latest)
        self.stdout_history_frame.bind_label(stdout_latest)

        self.history_frames_list = [self.stderr_history_frame,self.stdout_history_frame]
        self.stderr_input.bind_input_output(self.gui_lock,self._my_subprocess.stdin,self.history_frames_list)
        self.protocol("WM_DELETE_WINDOW", self._destroy_handler)

    def show_nominal_text(self):
      self.stdout_history_frame.add_text("@¤")
      self.stderr_history_frame.add_text("—")
      # self.stdout_history_frame.add_text("👤👥⛁")
      # self.stderr_history_frame.add_text("🂠")

    def _destroy_handler(self):

        with self.gui_lock:

            #Clean up in a threadsafe manner
            self._my_subprocess.terminate()
            self.stdout_history_frame = None
            self.stderr_history_frame = None
            self.history_frames_list = []

        self.destroy()

    def on_first_input(self, callback):
        self.stderr_input.on_first_input(callback)

    def append_stdout(self,append_text):
        self._stdout_queue.put(append_text.replace('\r',''), True)


    def append_stderr(self,append_text):
        self._stderr_queue.put(append_text.replace('\r',''), True)
        if len(append_text.strip()) > 0:
            self.stderr_input.enable()

    def render(self):
        """Render any queued text to the UI"""

        # Favour the stdout. Only show stderr if stdout is done giving us data.
        if not self._stdout_queue.empty():
            self._synchronous_append_all(self.stdout_history_frame,self._stdout_queue)
        elif not self._stderr_queue.empty():
            self._synchronous_append_all(self.stderr_history_frame,self._stderr_queue)


    def _synchronous_append_all(self,history_frame,append_queue):
        new_text = []
        while not append_queue.empty():
            new_text.append(append_queue.get())
        self._synchronous_append(history_frame, ''.join(new_text))


    def _synchronous_append(self,history_frame,append_text):
        with self.gui_lock:

            if not history_frame is None:
                history_frame.my_latest.add_text(append_text)

            for f in self.history_frames_list:
                f.scroll_down()


class RenderLoop(object):
    """Call subroutine in a loop. Use this to repeatedly call UI functions such as 'draw' or 'render'"""
    def __init__(self, root_window: ConsoleSeparateWindow):
        self.renderloop_go = True
        self._root_window = root_window
        self._after_id = None

    def start(self):
        assert not hasattr(self, '_after_id')
        self._run()

    def stop(self):
        self.renderloop_go = False
        if self._after_id is not None:
            self._root_window.after_cancel(self._after_id)
            self._after_id = None

    def _run(self):
        if self.renderloop_go:
            self._root_window.render()
            self._after_id = self._root_window.after(int(ScrollableText.TKINTER_SPAM_RELIEF_TIME * 1000), self._run)


def fake_out(stringme: str):
    print("STD " + stringme)


def run_cmd(cmd_args, cmd_cwd, cmd_env=None, stdout_callback = lambda s: sys.stdout.write(s)):

    #================================
    #   Execute our child process
    #================================
    #From http://mail.python.org/pipermail/python-list/2007-June/618721.html
    console_app = subprocess.Popen(cmd_args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,bufsize=0, cwd=cmd_cwd, env=cmd_env)


    #===============================================
    #   Create the application window via Tkinter
    #===============================================

    root_window = ConsoleSeparateWindow(console_app) # Create the window
    root_window.show_nominal_text()
    render_loop = RenderLoop(root_window.render)
    root_window.on_first_input(render_loop.start)

    #===========================================
    #   Bind stdout and stderr to root_window
    #===========================================
    stdout_capturer = SubProcessThread(console_app.stdout, console_app.poll, [root_window.append_stdout, stdout_callback])
    stderr_capturer = SubProcessThread(console_app.stderr, console_app.poll, [root_window.append_stderr])
    # stdout_capturer = SubProcessThread(console_app.stdout, console_app.poll, [fake_out])
    # stderr_capturer = SubProcessThread(console_app.stderr, console_app.poll, [fake_out])

    #==========
    #   Run!
    #==========
    stdout_capturer.start()
    stderr_capturer.start()
    print('Starting…')
    root_window.mainloop()
    print('STOPPED')
    render_loop.stop()
    print('EXIT')

if __name__=='__main__':
    print(os.path.abspath(os.curdir))

    run_cmd(sys.argv[1:], os.getcwd())
