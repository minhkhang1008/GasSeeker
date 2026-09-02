#!/usr/bin/env python3
"""GasSeeker Ground Station local launcher.

Serves the dashboard on localhost so Chromium-based browsers allow Web Serial.
No third-party Python packages are required.
"""
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
import os
import webbrowser

HERE = Path(__file__).resolve().parent
HOST, PORT = "127.0.0.1", 8765

# Compatibility patch for the base-station envelope:
# RX,<rssi>,<snr>,$GS,... must preserve every comma in the nested $GS packet.
PATCH = r"""
<script>
parseLine = function(raw) {
  let line = raw.trim(), rssi = '', snr = '';
  if (!line) return null;
  if (line.startsWith('RX,')) {
    const a = line.indexOf(',', 3);
    const b = a < 0 ? -1 : line.indexOf(',', a + 1);
    if (a < 0 || b < 0) return null;
    rssi = line.slice(3, a);
    snr = line.slice(a + 1, b);
    line = line.slice(b + 1);
  }
  if (!line.startsWith('$GS,')) { log(raw.trim()); return null; }
  if (!checksumOk(line)) {
    bad++; updateHealth(); log('Bỏ gói: checksum không hợp lệ'); return null;
  }
  const body = line.slice(1, line.lastIndexOf('*')).split(',');
  if (body.length < 16) { bad++; updateHealth(); return null; }
  const f = body.slice(1);
  return {
    t_s:+f[0], algo:f[1], state:f[2], adc:+f[3], norm:+f[4], ppm:+f[5],
    level:f[6], x_cm:+f[7], y_cm:+f[8], head_deg:+f[9], dist_cm:+f[10],
    cell_x:+f[11], cell_y:+f[12], best_norm:+f[13], finished:+f[14],
    rssi:rssi===''?null:+rssi, snr:snr===''?null:+snr, raw:line
  };
};
</script>
"""

class Handler(SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.path in ('/', '/index.html'):
            html = (HERE / 'index.html').read_text(encoding='utf-8')
            html = html.replace('</body>', PATCH + '\n</body>')
            data = html.encode('utf-8')
            self.send_response(200)
            self.send_header('Content-Type', 'text/html; charset=utf-8')
            self.send_header('Content-Length', str(len(data)))
            self.send_header('Cache-Control', 'no-store')
            self.end_headers()
            self.wfile.write(data)
            return
        return super().do_GET()

if __name__ == '__main__':
    os.chdir(HERE)
    url = f'http://{HOST}:{PORT}/'
    print('GasSeeker Ground Station')
    print(f'Open: {url}')
    print('Recommended browser: Google Chrome or Microsoft Edge')
    print('Press Ctrl-C to stop.\n')
    try:
        webbrowser.open(url)
    except Exception:
        pass
    ThreadingHTTPServer((HOST, PORT), Handler).serve_forever()
