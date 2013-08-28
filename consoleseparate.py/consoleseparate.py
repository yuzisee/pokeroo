
#for argv
import sys

#for threading.Thread, threading.Lock
import threading

#for windows
import Tkinter

#for Popen
import subprocess

#for os.read, os.getcwd
import os

#for sleep
import time

class SubProcessThread(threading.Thread):
    MAXIMUM_BYTE_READ = 2048

    def __init__(self,bytestream,returncode_test,text_append_callbacks):
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
            if not next_char is None:
                for cb in self._txt_callbacks:
                    cb(next_char)
            #except Exception:
            #    print "CONSOLESEPARATE: abort "

        print str(self._fd) + "CONSOLESEPARATE: read terminate"

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
        #Let it settle down so that we know how far we actually need to scroll
        time.sleep(ScrollableText.TKINTER_SPAM_RELIEF_TIME)
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

class StackableTimeout(object):
    """Similar to threading.Timer but allows you to stack calls.

    Example:
    >>> a = StackableTimeout(entry_callback, exit_callback)
    >>> a.set_timeout(1.0) # Since we weren't waiting before, call entry_callback immediately. Then, enter "sleep state". Expect to call exit_callback in 1.0 seconds from now ...
    >>> time.sleep(0.5) # ... but 0.5 seconds later, ...
    >>> a.set_timeout(1.0) # ... postpone the exit_callback until another 1.0 seconds from now!

    """
    def __init__(self, entry_callback, exit_callback):
        """entry_callback will be called when entering sleep state, and exit_callback will be called when exiting sleep state.


        """
        self._entry_callback = entry_callback
        self._exit_callback = exit_callback
        self._lock = threading.Lock()
        self._active_timeout = None # If in "sleep state", this is set to the unit epoch timestamp of when we should call exit_callback. Otherwise, it's set to None.

    def set_timeout(self, seconds):
        
        print "Set Timeout"

        # Enter sleep state if necessary
        entering_sleep_state = False
        with self._lock:
            if self._active_timeout is None:
                entering_sleep_state = True
                self._active_timeout = time.time() + seconds
            else:
                self._active_timeout = max(time.time() + seconds, self._active_timeout)


        # INVARIANT: If multiple threads call set_timeout at the same time, only one will emerge with entering_sleep_state
        
        # On transition *into* sleep state, call entry_callback once.
        if entering_sleep_state:
            print "Entering Sleep State"
            self._entry_callback()
            threading.Timer(seconds, self._try_wakeup) # Now, queue a wakeup event *seconds* seconds into the future.
            # INVARIANT: There should be only one threading.Timer at one time, ever.
            
    def _try_wakeup(self):
        """Try waking up. If we haven't yet passed self._active_timeout, then sleep a little longer"""
        wakeup_ok = False
        while not wakeup_ok:
            # there should only be one wakeup timer at once, so just acquire the lock to read the value in case it is being written to
            current_time = time.time()
            with self._lock:
                wakeup_time = self._active_timeout

                if current_time > wakeup_time:
                    # Okay, we're ready to wake up.
                    wakeup_ok = True

            if current_time < wakeup_time:
                # No wakeup yet. Sleep and try again later.
                time.sleep(wakeup_time - current_time)

        # We are waking up now.
        self._exit_callback()

        # Done! Now all we have to do is clear self._active_timeout
        # But what if _active_timeout was increased while we were calling exit_callback?

        need_to_return_to_sleep_state = False
        with self._lock:
            if wakeup_time == self._active_timeout:
                # Nobody has increased self._active_timeout since we called exit_callback.
                # All is good.
                self._active_timeout = None
            else:
                # Someone updated self._active_timeout, but we already called exit_callback.
                # Since self._active_timeout wasn't None for them, they didn't call entry_callback
                need_to_return_to_sleep_state = True

        if need_to_return_to_sleep_state:
            self._entry_callback()
            theading.Timer(0, self._try_wakeup) # Go ahead and restart this function

        

class UserEntry(Tkinter.Entry):
    """A textbox that responds specially to [Return] button presses

    When [Return] is pressed: (This is configured once in self.bind_input_output)
      + Send the contents of the textbox to our subprocess.Popen's stdin.
      + We ask all the history frames to rotate themselves.
      + We append the user's input to the stdin_history and clear it from the textbox

    """
    def __init__(self, parent, **options):
        Tkinter.Entry.__init__(self,parent,options)
        self.insert(0, "Press [Enter] to begin")
        self._delayer = StackableTimeout(self._set_disabled, self._set_enabled)

    def _set_disabled(self):
        with self.gui_lock:
            self.config(state=Tkinter.DISABLED)
            
    def _set_enabled(self):
        with self.gui_lock:
            self.config(state=Tkinter.ENABLED)

    def setup_geometry_manager(self):
        """When called, we will attach ourselves to the bottom of our parent widget, and expand horizontally to fill"""
        self.pack(side=Tkinter.BOTTOM,fill=Tkinter.X,expand=0)

    def bind_input_output(self,gui_lock,app_stdin,input_output_pairs):
        self._label_histories = input_output_pairs
        self._app_stdin = app_stdin
        self.bind("<Return>",self.capture_return_event)

        self.uninitialized = True
        self.gui_lock = gui_lock

        # self.gui_lock.acquire(True)
        # To be released on the first capture_return_event, see self.uninitialized
        # TODO(from yuzisee): Why did we have this?

    def capture_return_event(self,event):

        if self.uninitialized:
            with self.gui_lock: # TODO(from yuzisee): This should be just fine. Why did we carry the lock across bind_input_output --> capture_return_event before?
                event.widget.delete(0, Tkinter.END)
            self.uninitialized = False
            #self.gui_lock.release()
            return

        with self.gui_lock:

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

    def disable_for_one_second(self):
        if not self.uninitialized:
            self._delayer.set_timeout(1.0)


class ConsoleSeparateWindow(Tkinter.Tk):

    EXPAND_ALL = Tkinter.W+Tkinter.E+Tkinter.N+Tkinter.S
    EXPAND_BOTTOM_HORIZONTAL =Tkinter.W+Tkinter.E+Tkinter.S

#    DEFAULT_CONSOLE_FONT = font=("Lucida Console", 9)
#    DEFAULT_INPUT_FONT = font=("Courier New", 9,"bold")
#Portable font choices: http://wiki.tcl.tk/451
    DEFAULT_CONSOLE_FONT = font=("Courier", 9)
    DEFAULT_INPUT_FONT = font=("Courier", 9,"bold")


    def __init__(self, my_console_app):
        Tkinter.Tk.__init__(self)

        self._my_subprocess = my_console_app

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

        left_column = Tkinter.Frame(self, background='magenta') # Create a frame and attach it to the parent
        left_column.grid(row=0, column=0, sticky=ConsoleSeparateWindow.EXPAND_ALL)
        right_column = Tkinter.Frame(self, background='blue') # Create a frame and attach it to the parent
        right_column.grid(row=0, column=1, sticky=ConsoleSeparateWindow.EXPAND_ALL)

        self.stdout_history_frame = HistoryText(left_column)
        self.stderr_history_frame = HistoryText(right_column)

        self.stdout_history_frame.set_font(ConsoleSeparateWindow.DEFAULT_CONSOLE_FONT)
        self.stderr_history_frame.set_font(ConsoleSeparateWindow.DEFAULT_CONSOLE_FONT)

        stdout_latest = AppendableLabel(left_column)
        stdout_latest.set_font(ConsoleSeparateWindow.DEFAULT_CONSOLE_FONT)

        #The input frame contains the stderr latest with an entry field at the bottom
        stderr_input_frame = Tkinter.Frame(right_column, borderwidth=2, relief=Tkinter.GROOVE, background='green')
        stderr_input = UserEntry(stderr_input_frame,relief=Tkinter.SUNKEN,width=0,font=ConsoleSeparateWindow.DEFAULT_INPUT_FONT)

        stderr_latest  = AppendableLabel(stderr_input_frame)
        stderr_latest.set_font(ConsoleSeparateWindow.DEFAULT_CONSOLE_FONT)

        #Grid layout is resizable
        stderr_input.setup_geometry_manager()

        stderr_latest.pack(side=Tkinter.TOP,fill=Tkinter.BOTH,expand=1)


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

        stderr_input.focus_set()

        #Event capture
        self.stderr_history_frame.bind_label(stderr_latest)
        self.stdout_history_frame.bind_label(stdout_latest)

        self.history_frames_list = [self.stderr_history_frame,self.stdout_history_frame]
        stderr_input.bind_input_output(self.gui_lock,self._my_subprocess.stdin,self.history_frames_list)
        self.protocol("WM_DELETE_WINDOW", self._destroy_handler)

        self._delay_after_text = stderr_input.disable_for_one_second

    def _destroy_handler(self):

        with self.gui_lock:

            #Clean up in a threadsafe manner
            self._my_subprocess.terminate()
            self.stdout_history_frame = None
            self.stderr_history_frame = None
            self.history_frames_list = []

        self.destroy()

    def append_stdout(self,append_text):
        self._delay_after_text()
        self._synchronous_append(self.stdout_history_frame,append_text.replace('\r',''))
        
    def append_stderr(self,append_text):
        self._synchronous_append(self.stderr_history_frame,append_text.replace('\r',''))

    def _synchronous_append(self,history_frame,append_text):
        with self.gui_lock:

            if not history_frame is None:
                history_frame.my_latest.add_text(append_text)

            for f in self.history_frames_list:
                f.scroll_down()





def fake_out(stringme):
    print "STD " + str(stringme)


def run_cmd(cmd_args, cmd_cwd, cmd_env=None, stdout_callback = lambda s: sys.stdout.write(s)):

    #================================
    #   Execute our child process
    #================================
    #From http://mail.python.org/pipermail/python-list/2007-June/618721.html
    console_app = subprocess.Popen(cmd_args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,bufsize=1, cwd=cmd_cwd, env=cmd_env)


    #===============================================
    #   Create the application window via Tkinter
    #===============================================

    root_window = ConsoleSeparateWindow(console_app)

    #===========================================
    #   Bind stdout and stderr to root_window
    #===========================================
    stdout_capturer = SubProcessThread(console_app.stdout, console_app.poll, [root_window.append_stdout, stdout_callback])
    stderr_capturer = SubProcessThread(console_app.stderr, console_app.poll, [root_window.append_stderr])
    #stdout_capturer = SubProcessThread(console_app.stdout, console_app.poll, fake_out)
    #stderr_capturer = SubProcessThread(console_app.stderr, console_app.poll, fake_out)

    #==========
    #   Run!
    #==========
    stdout_capturer.start()
    stderr_capturer.start()
    root_window.mainloop()

if __name__=='__main__':
    print os.path.abspath(os.curdir)

    run_cmd(sys.argv[1:], os.getcwd())

