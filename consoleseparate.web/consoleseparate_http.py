#!/usr/bin/env python3

import collections
import http.server
import json
import os
import queue
import re
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
        threading.Thread.__init__(self)
        self._kill_server = kill_server

    def run(self) -> None:
        """Run until ABORT_IF_N_CONSECUTIVE_HEARTBEATS_MISSED heartbeats are all missing, and then `kill_server_and_console_app`"""
        missed_heartbeats = 0
        while missed_heartbeats < ABORT_IF_N_CONSECUTIVE_HEARTBEATS_MISSED:
            last_client = self.client_heartbeat_seconds
            last_server = self.server_heartbeat_seconds

            # print('Heartbeat check wait...')
            time.sleep(HEARTBEAT_INTERVAL_MILLIS / 1000.0)
            # print('Heartbeat check go!')

            if (last_client != self.client_heartbeat_seconds) or (last_server != self.server_heartbeat_seconds):
                # New heartbeat! Restart the count.
                missed_heartbeats = 0
            else:
                # No new heartbeat from client yet.
                missed_heartbeats += 1

                if missed_heartbeats > 2:
                    print('Warning: lost connection to webpage ' + json.dumps({'missed_heartbeats': missed_heartbeats}))

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
ABORT_IF_N_CONSECUTIVE_HEARTBEATS_MISSED = 21000 / 3000
HEARTBEAT_MILLIS_JS = "var heartbeat_millis = 3000;"
HEARTBEAT_INTERVAL_MILLIS = 3000
CONSOLESEPARATE_HTML_EPILOGUE = """
    <style>
        .two-columns {
            display: flex;
            flex-direction: row;
        }
        .two-columns > div {
            flex: 1;

            white-space: pre-wrap;

            display: flex;
            flex-direction: column;
        }
        .history-text > div {
            border: 2px solid gray;
        }
        .stdout-theme {
            background-color: magenta;
        }
        .stderr-theme {
            background-color: blue;
            color: silver;
            font-family: sans-serif;
        }
        .raw-text-align-bottom {
            justify-content: flex-end;
        }
        #appendable-label-stderr {
            margin-top: auto;
            margin-bottom: 0px;
        }
        #stdin-user-entry {
            background-color: lime;
            width: 100%;
            font-size: 161.8%;
        }
    </style>
    <script src="heartbeat-init.js"></script>
    <script>
      function has_string_content(obj, keyname) {
        return obj.hasOwnProperty(keyname) && (obj[keyname] != null) && (obj[keyname] != "");
      }

      function stream_text_from_server(message_event) {
        message_payload = JSON.parse(message_event.data);
        if (has_string_content(message_payload, 'stdout_append_txt')) {
          document.getElementById('appendable-label-stdout').textContent += message_payload.stdout_append_txt;
          document.getElementById('stdin-user-entry').scrollIntoView();
        }
        if (has_string_content(message_payload, 'stderr_append_txt')) {
          document.getElementById('appendable-label-stdout').textContent += message_payload.stderr_append_txt;
          document.getElementById('stdin-user-entry').scrollIntoView();
        }
      }
      // https://developer.mozilla.org/en-US/docs/Web/API/EventSource

      const initial_text_stream = new EventSource("/stream");
      initial_text_stream.addEventListener("message", stream_text_from_server);
      // https://developer.mozilla.org/en-US/docs/Web/API/EventSource/readyState

      initial_text_stream.addEventListener("error", function(event) {
          alert("Stream error occurred " + JSON.stringify(event.data));
          // You could implement custom reconnection logic here if needed
      });
    </script>
  </head>
  <body>
    <div class="two-columns history-text">
        <div id="stdout-history" class="stdout-theme raw-text-align-bottom"> <p>Sample text in left column, aligned to bottom.</p>
        </div>
        <div id="stderr-history" class="raw-text-align-bottom">
            <p>Sample text in right column, aligned to bottom.</p>
        </div>
    </div>
    <div class="two-columns">
        <div id="appendable-label-stdout" class="stdout-theme raw-text-align-bottom"></div>
        <div class="stderr-theme"><p id="appendable-label-stderr"></p>
<input id="stdin-user-entry" type="text" placeholder="⌨"></div>
    </div>
    <script>
      var userEntryEl = document.getElementById('stdin-user-entry');
      userEntryEl.focus();

      userEntryEl.addEventListener('keydown', function(keyboard_event) {
          if (keyboard_event.keyCode === 13) {
              alert('TODO 1');
          }
      });
    </script>
  </body>
</html>
"""

POKER_REPLACE = True

STREAMING_FPS = 24.0 # That's the Hollywood movie frames per second; should be fine for now

class ConsoleSeparateController(http.server.BaseHTTPRequestHandler):
    server: ConsoleSeparateWebview

    def do_POST(self):
        # print('POST? ' + self.path)
        if self.path == '/heartbeat':
            data_len = int(self.headers.get('Content-Length'))
            self.server.observe_most_recent_heartbeat(self.rfile.read(data_len))

            self.send_response(200)
        else:
            self.send_response(405) # "Method Not Allowed"

        self.end_headers()

    def do_PUT(self):
        current_url = urllib.parse.urlparse(self.path)
        if current_url.path == '/user_entry':
            query_string = urllib.parse.parse_qs(current_url.query)
            stdin_payload = query_string.get('stdin_txt', '').strip()

            if stdin_payload:
               # Send the command to `self.server._console_app` and simulate a long-poll that waits for a response
               self.server.receive_stdin(stdin_payload)

               while not self.server.no_more_data_on_the_way():
                   time.sleep(1.0 / STREAMING_FPS)

            self.send_response(200)
        else:
            self.send_response(400) # "Bad Request"

        self.end_headers()

    def do_GET(self):

        if self.path == '/':
            # Loaded by the original `open_browser` below
            self.send_response(200)
            self.send_header('Content-Type', 'text/html; charset=utf-8')
            consoleseparate_html = CONSOLESEPARATE_HTML_PROLOGUE + self.server.render_html_title() + CONSOLESEPARATE_HTML_EPILOGUE
            consoleseparate_bytes = consoleseparate_html.encode('utf-8')
            self.send_header('Content-Length', len(consoleseparate_bytes))
            self.end_headers()

            self.wfile.write(consoleseparate_bytes)
        elif self.path == '/favicon.ico':
            self.send_response(206) # "No Content"
            self.end_headers()
        elif self.path == '/heartbeat-init.js':
            worker_js = """
if (typeof Worker !== "undefined") {
  const myWorker = new Worker("heartbeat-worker.js");

  // heartbeat-worker.js will post a message every HEARTBEAT_MILLIS_JS
  myWorker.onmessage = function(message_event) {
    navigator.sendBeacon('/heartbeat', message_event.data);
  };

  // TODO(from joseph): Use `document.addEventListener('visibilitychange', () => {` to extend the timeout when we know we're backgrounded
} else {
""" + HEARTBEAT_MILLIS_JS + """

  function heartBeat() {
    navigator.sendBeacon('/heartbeat', (new Date()).valueOf());
  }

  // This is less reliable than "heartbeat-worker.js" because not all browsers will allow it to run in the background
  setInterval(heartBeat, heartbeat_millis);
}
"""
            self.send_response(200)
            self.send_header('Content-Type', 'text/javascript')
            self.send_header('Content-Length', len(worker_js))
            self.end_headers()
            self.wfile.write(worker_js.encode('utf-8'))
        elif self.path == '/heartbeat-worker.js':
            worker_js = HEARTBEAT_MILLIS_JS + """

function heartBeat_workermessage() {
  self.postMessage((new Date()).valueOf());
}

setInterval(heartBeat_workermessage, heartbeat_millis);
"""
            self.send_response(200)
            self.send_header('Content-Type', 'text/javascript')
            self.send_header('Content-Length', len(worker_js))
            self.end_headers()
            self.wfile.write(worker_js.encode('utf-8'))
        elif self.path == '/stream':

            if self.headers['Accept'] != 'text/event-stream':
                self.send_response(406) # "Not Acceptable"
                self.end_headers()
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

            # sse_retry_millis = math.ceil(1000.0 / STREAMING_FPS)
            # self.wfile.write(f"retry: {sse_retry_millis}\n\n".encode('utf-8'))
            # Simulate streaming text with delays
            while self.server.server_sent_events_stream:
                # https://developer.mozilla.org/en-US/docs/Web/API/MessageEvent
                message_payload = self.server.latest_data()
                if all(v is None for v in message_payload.values()):
                    # Nothing to send yet (but until `no_more_data_on_the_way()` we gotta keep waiting)
                    time.sleep(1.0 / STREAMING_FPS)
                else:
                    message_json = json.dumps(message_payload)
                    self.wfile.write(f"data: {message_json}\n\n".encode('utf-8'))
                    self.wfile.flush()

            print('End stream')
        else:
            self.send_response(404) # "Not Found"
            self.end_headers()

class ConsoleSeparateMessageEvent(typing.TypedDict):
    stdout_append_txt: str
    stderr_append_txt: str

class ConsoleSeparateWebview(socketserver.ThreadingTCPServer):
    _stdout_queue: queue.SimpleQueue
    _stderr_queue: queue.SimpleQueue
    server_sent_events_stream: bool
    _b_stderr_input_enable: bool
    _html_title: str
    _heartbeat: HeartbeatThread
    _console_app: subprocess.Popen

    def __init__(self, *args, **kw):
        super().__init__(*args, **kw)
        self._stdout_queue = queue.SimpleQueue()
        self._stderr_queue = queue.SimpleQueue()
        self.server_sent_events_stream = True
        self._b_stderr_input_enable = True
        self._html_title = '<title>👤👥🂠⛁</title>'
        self._heartbeat = HeartbeatThread(self)
        self._console_app = None

    def observe_most_recent_heartbeat(self, client_timestamp_str):
        self._heartbeat.client_heartbeat_seconds = int(client_timestamp_str) / 1000.0
        self._heartbeat.server_heartbeat_seconds = time.time()

        # print(f'OBSERVE? {client_timestamp_str} {self._heartbeat.client_heartbeat_seconds} {self._heartbeat.server_heartbeat_seconds}')

        if not self._heartbeat.is_alive():
          try:
              self._heartbeat.start()
              print('Connected to webpage!')
          except RuntimeError as e:
              # If there's a race condition where we try to start it twice, this can happen. No worries.
              print(f'Race condition? {e.args}')
              # https://docs.python.org/3/library/threading.html#threading.Thread.start

    def shutdown_and_stop_stream(self):
        self.server_sent_events_stream = False
        # https://docs.python.org/3/library/socketserver.html#socketserver.BaseServer.shutdown
        super().shutdown()
        print('socketserver.BaseServer.shutdown DONE')

    def kill_server_and_console_app(self):
        if self._console_app is not None:
            self._console_app.terminate()
            # Do we also need `self._console_app.kill()` in the rare case that `.terminate()` fails?

        self.shutdown_and_stop_stream()

    def connect_to_console_app(self, subprocess_popen: subprocess.Popen):
        self._console_app = subprocess_popen

    def set_html_title(self, s: str):
        self._html_title = '<title>' + s + '</title>'

    def render_html_title(self) -> str:
        return self._html_title

    @staticmethod
    def render_text(new_text: str) -> str:
        # txt = new_text.replace('\r','')
        if POKER_REPLACE:
            return re.sub(r'\b[2-9TJQKA][cdhs]\b', lambda m: m.group(0).replace('s', u'\u2664').replace('h', u'\u2661').replace('c', u'\u2663').replace('d', u'\u2662'), new_text)
        else:
            return new_text

    def receive_stdin(self, user_input_send_chars: str):
        self._b_stderr_input_enable = False
        self._console_app.write(user_input_send_chars.encode('ascii'))
        self._console_app.flush()

    def append_stdout(self, append_text: str):
        # print('GOT MESSAGE: ' + append_text)
        self._stdout_queue.put(ConsoleSeparateWebview.render_text(append_text), True)
        # print('Enqueued.')

    def append_stderr(self, append_text: str):
        self._stderr_queue.put(ConsoleSeparateWebview.render_text(append_text), True)
        if len(append_text.strip()) > 0:
            self._b_stderr_input_enable = True

    def no_more_data_on_the_way(self) -> bool:
        return self._stdout_queue.empty() and self._stderr_queue.empty() and self._b_stderr_input_enable

    def latest_data(self) -> ConsoleSeparateMessageEvent:
        latest_stdout = None
        latest_stderr = None
        try:
            latest_stdout = self._stdout_queue.get_nowait()
        except queue.Empty:
            pass

        try:
            latest_stderr = self._stderr_queue.get_nowait()
        except queue.Empty:
            pass

        return {'stdout_append_txt': latest_stdout, 'stderr_append_txt': latest_stderr}


def run_server(httpd: socketserver.ThreadingTCPServer) -> threading.Thread:
    new_thr = threading.Thread(target=httpd.serve_forever)
    # new_thr.daemon = True # Is this needed?
    new_thr.start()
    return new_thr

def stop_server(httpd: socketserver.ThreadingTCPServer, server_thr: threading.Thread):
    httpd.shutdown_and_stop_stream()
    print('Waiting for server thread to complete')
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
    with ConsoleSeparateWebview(("", 0), ConsoleSeparateController) as root_window:
        root_window.set_html_title('→'.join(cmd_args))

        # stdout_capturer = SubProcessThread(console_app.stdout, console_app.poll, [fake_out])
        # stderr_capturer = SubProcessThread(console_app.stderr, console_app.poll, [fake_out])
        #============================================
        #   Bind stdout and stderr to the web view
        #============================================
        stdout_capturer = SubProcessThread(console_app.stdout, console_app.poll, [root_window.append_stdout, stdout_callback])
        stderr_capturer = SubProcessThread(console_app.stderr, console_app.poll, [root_window.append_stderr])

        #============
        #   Begin!
        #============
        root_window.connect_to_console_app(console_app)
        server_thread = run_server(root_window)
        _, port = root_window.server_address
        print(f"Open your browser to → http://localhost:{port}")
        open_browser(port)

        stdout_capturer.start()
        stderr_capturer.start()
        # Run to completion! (We might abort early if `ConsoleSeparateWebview.kill_server_and_console_app` gets triggered by `HeartbeatThread`
        stdout_capturer.join()
        print('Waiting to flush stderr', file=sys.stderr)
        stderr_capturer.join()

        # INVARIANT: If you get here, the original subprocess.Popen is done

        stop_server(root_window, server_thread)

    # We get `root_window.server_close()` automatically because of the `with … as … root_window:` above

if __name__=='__main__':
    print(os.path.abspath(os.curdir))

    run_cmd(sys.argv[1:], os.getcwd())


