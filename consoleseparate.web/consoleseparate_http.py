#!/usr/bin/env python3

import collections
import http.server
import json
import os
import queue
import socketserver
import subprocess
import sys
import threading
import time
import typing
import urllib.parse

class ConsoleSeparateWebview:
    pass

class HeartbeatThread(threading.Thread):
    _kill_server: ConsoleSeparateWebview
    client_heartbeat_seconds: float
    server_heartbeat_seconds: float

    def __init__(self, kill_server):
        self._kill_server = kill_server

    def run(self) -> None:
        """Run until ABORT_IF_N_CONSECUTIVE_HEARTBEATS_MISSED heartbeats are all missing, and then `kill_server_and_console_app`"""
        missed_heartbeats = 0
        while missed_heartbeats < ABORT_IF_N_CONSECUTIVE_HEARTBEATS_MISSED:
            last_client = self.client_heartbeat_seconds
            last_server = self.server_heartbeat_seconds

            time.sleep(HEARTBEAT_INTERVAL_MILLIS / 1000.0)

            if (last_client != self.client_heartbeat_seconds) or (last_server != self.server_heartbeat_seconds):
                # New heartbeat! Restart the count.
                missed_heartbeats = 0
            else:
                # No new heartbeat from client yet.
                missed_heartbeats += 1

                if missed_heartbeats > 2:
                    print('Warning: lost connection to webpage ' + json.dump({'missed_heartbeats': missed_heartbeats}))

        # INVARIANT: If you get here, we exceeded ABORT_IF_N_CONSECUTIVE_HEARTBEATS_MISSED.

        self._kill_server.kill_server_and_console_app()

class SubProcessThread(threading.Thread):
    _fd: int
    _stopreading_test: collections.abc.Callable[[], bool]
    _txt_callbacks: list[collections.abc.Callable[[str], None]]

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
            next_chars = os.read(self._fd, SubProcessThread.MAXIMUM_BYTE_READ)
            #print str(self._fd) + "received [" + repr(next_chars) + "]"
            if next_chars:
                for cb in self._txt_callbacks:
                    # [!TIP]
                    # 'utf-8' is a superset of ASCII so it's safe to do this.
                    # Furthermore, when we set `self.POKER_REPLACE` we need to append unicode characters into this string so having it as Unicode here is best.
                    cb(next_chars.decode('utf-8', 'strict'))
            #except Exception:
            #    print "CONSOLESEPARATE: abort "

        print(str(self._fd) + "CONSOLESEPARATE: read terminate")

CONSOLESEPARATE_HTML_PROLOGUE = """
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8">
"""
ABORT_IF_N_CONSECUTIVE_HEARTBEATS_MISSED = 24000 / 3000
HEARTBEAT_INTERVAL_MILLIS = 3000
CONSOLESEPARATE_HTML_EPILOGUE = """
    <script>
      var heartbeat_millis = 3000;

      function heartBeat() {
        navigator.sendBeacon('/heartbeat', (new Date()).valueOf());
      }

      setInterval(heartBeat, heartbeat_millis);
    </script>
  </head>
  <body>
  </body>
</html>
"""

STREAMING_FPS = 24.0 # That's the Hollywood movie frames per second; should be fine for now

class ConsoleSeparateController(http.server.BaseHTTPRequestHandler):
    server: ConsoleSeparateWebview

    def do_POST(self):
        if self.path == '/heartbeat':
            data_len = int(self.headers.get('Content-Length'))
            self.server.observe_most_recent_heartbeat(self.rfile.read(data_len))
        else:
            self.send_response(405) # "Method Not Allowed"

    def do_GET(self):
        current_url = urllib.parse.urlparse(self.path)

        if current_url.path == '/':
            self.send_response(200)
            self.send_header('Content-Type', 'text/html; charset=utf-8')
            consoleseparate_html = CONSOLESEPARATE_HTML_PROLOGUE + self.server.render_html_title() + CONSOLESEPARATE_HTML_EPILOGUE
            consoleseparate_bytes = consoleseparate_html.encode('utf-8')
            self.send_header('Content-Length', len(consoleseparate_bytes))
            self.end_headers()

            self.wfile.write(consoleseparate_bytes)
        elif current_url.path == '/stream':
            query_string = urllib.parse.parse_qs(current_url.query)
            stdin_payload = query_string.get('stdin_txt', '').strip()

            if not stdin_payload:
                self.send_response(204) # "No Content"
                return

            if self.headers['Accept'] != 'text/event-stream':
                self.send_response(406) # "Not Acceptable"
                return

            # =================
            # OK! MAIN CODEPATH
            # =================

            # https://developer.mozilla.org/en-US/docs/Web/API/EventSource/message_event
            self.send_response(200)
            self.send_header('Content-Type', 'text/event-stream')
            self.send_header('Cache-Control', 'no-cache')
            self.send_header('Connection', 'keep-alive')
            self.end_headers()
            # Simulate streaming text with delays
            while not self.server.no_more_data_on_the_way():
                # https://developer.mozilla.org/en-US/docs/Web/API/MessageEvent
                message_payload = self.server.latest_data
                if all(v is None for v in message_payload.values):
                    # Nothing to send yet (but until `no_more_data_on_the_way()` we gotta keep waiting)
                    time.sleep(1.0 / STREAMING_FPS)
                else:
                    message_json = json.dump(message_payload)
                    self.wfile.write(f"data: {message_json}\n\n".encode('utf-8'))
                    self.wfile.flush()
        else:
            self.send_response(404) # "Not Found"
            return

class ConsoleSeparateMessageEvent(typing.TypedDict):
    stdout_append_txt: str
    stderr_append_txt: str

class ConsoleSeparateWebview(socketserver.ThreadingTCPServer):
    append_stdout_queue: queue.SimpleQueue
    append_stderr_queue: queue.SimpleQueue
    b_stderr_input_enable: bool
    _console_app: subprocess.Popen
    _html_title: str

    def __init__(self, *args, **kw):
        super().__init__(*args, **kw)
        self.append_stdout_queue = queue.SimpleQueue()
        self.append_stderr_queue = queue.SimpleQueue()
        self.b_stderr_input_enable = True
        self._html_title = '<title>👤👥🂠⛁</title>'
        self._heartbeat = HeartbeatThread(self)
        self._console_app = None

    def observe_most_recent_heartbeat(self, client_timestamp_str):
        self._heartbeat.client_heartbeat_seconds = int(client_timestamp_str) / 1000.0
        self._heartbeat.server_heartbeat_seconds = time.time()
        try:
            self._heartbeat.start()
        except RuntimeError:
            # If there's a race condition where we try to start it twice, this can happen. No worries.
            pass
            # https://docs.python.org/3/library/threading.html#threading.Thread.start

    def kill_server_and_console_app(self):
        if self._console_app is not None:
            self._console_app.terminate()
            # Do we also need `self._console_app.kill()` in the rare case that `.terminate()` fails?

        # https://docs.python.org/3/library/socketserver.html#socketserver.BaseServer.shutdown
        self.shutdown()

    def connect_to_console_app(self, subprocess_popen: subprocess.Popen):
        self._console_app = subprocess_popen

    def set_html_title(self, s: str):
        self._html_title = '<title>' + s + '</title>'

    def render_html_title(self) -> str:
        return self._html_title

    def append_stdout(self, append_text: str):
        self._stdout_queue.put(append_text.replace('\r',''), True)

    def append_stderr(self, append_text: str):
        self._stderr_queue.put(append_text.replace('\r',''), True)
        if len(append_text.strip()) > 0:
            self.b_stderr_input_enable = True

    def no_more_data_on_the_way(self) -> bool:
        return self.append_stdout_queue.empty() and self.append_stderr_queue.empty() and self.b_stderr_input_enable

    def latest_data(self) -> ConsoleSeparateMessageEvent:
        latest_stdout = None
        latest_stderr = None
        try:
            latest_stdout = self.append_stdout_queue.get_nowait()
        except queue.Empty:
            pass

        try:
            latest_stderr = self.append_stderr_queue.get_nowait()
        except queue.Empty:
            pass

        return {'stdout_append_txt': latest_stdout, 'stderr_append_txt': latest_stderr}


def run_server(httpd: socketserver.ThreadingTCPServer) -> threading.Thread:
    new_thr = threading.Thread(target=httpd.serve_forever)
    new_thr.start()
    return new_thr

def stop_server(httpd: socketserver.ThreadingTCPServer, server_thr: threading.Thread):
    httpd.shutdown()
    server_thr.join()

def open_browser(port: int):
    if sys.platform == 'darwin':
        subprocess.call(['open', 'http://localhost:' + str(port)])
    elif sys.platform == 'linux':
        subprocess.call(['xdg-open', 'http://localhost:' + str(port)])
    else:
        # Windows, I guess?
        subprocess.call(['start', 'http://localhost:' + str(port)])

def fake_out(stringme: str):
    print("STD " + stringme)

def run_cmd(cmd_args, cmd_cwd, cmd_env=None, stdout_callback = lambda s: sys.stdout.write(s)):
    #================================
    #   Execute our child process
    #================================
    console_app = subprocess.Popen(cmd_args, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,bufsize=0, cwd=cmd_cwd, env=cmd_env)

    #=====================================================
    #   Draw the application window on http://localhost
    #=====================================================

    server_thread = None
    with ConsoleSeparateWebview(("", 0), ConsoleSeparateController) as httpd:
        server_thread = run_server(httpd)
        httpd.connect_to_console_app(console_app)
        httpd.set_html_title('→'.join(cmd_args))
        _, port = httpd.server_address
        print(f"Open your browser to → http://localhost:{port}")
        open_browser(port)

        stdout_capturer = SubProcessThread(console_app.stdout, console_app.poll, [fake_out])
        stderr_capturer = SubProcessThread(console_app.stderr, console_app.poll, [fake_out])
        #============================================
        #   Bind stdout and stderr to the web view
        #============================================
        # stdout_capturer = SubProcessThread(console_app.stdout, console_app.poll, [root_window.append_stdout, stdout_callback])
        # stderr_capturer = SubProcessThread(console_app.stderr, console_app.poll, [root_window.append_stderr])

        stdout_capturer.start()
        stderr_capturer.start()
        # Run to completion! (We might abort early if `ConsoleSeparateWebview.kill_server_and_console_app` gets triggered by `HeartbeatThread`
        stdout_capturer.join()
        stderr_capturer.join()

        # INVARIANT: If you get here, the original subprocess.Popen is done

        stop_server(httpd, server_thread)

    # We get `httpd.server_close()` automatically because of the `with … as … httpd:` above

if __name__=='__main__':
    print(os.path.abspath(os.curdir))

    run_cmd(sys.argv[1:], os.getcwd())


