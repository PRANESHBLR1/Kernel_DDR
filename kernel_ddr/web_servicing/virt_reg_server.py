#!/usr/bin/env python3
import ssl
import os
import traceback
from flask import Flask, request, jsonify, abort
from werkzeug.exceptions import HTTPException

# ----- config -----
BASE = 0x80000000
SIZE = 0x10000
END = BASE + SIZE - 1
MEMFILE = "vreg.bin"


if os.path.exists(MEMFILE):
    with open(MEMFILE, "rb") as f:
        data = f.read()
    if len(data) == SIZE:
        memory = bytearray(data)
    else:
        memory = bytearray(SIZE)
else:
    memory = bytearray(SIZE)

app = Flask(__name__)

# JSON error handlers
@app.errorhandler(HTTPException)
def handle_http_exception(e):
    payload = {"error": e.name, "message": str(e.description)}
    return jsonify(payload), e.code

@app.errorhandler(Exception)
def handle_exception(e):
    tb = traceback.format_exc()
    payload = {"error": "InternalServerError", "message": str(e), "trace": tb}
    return jsonify(payload), 500

def save_memory():
    """Atomic save of memory to disk."""
    tmp = MEMFILE + ".tmp"
    with open(tmp, "wb") as f:
        f.write(memory)
        f.flush()
        os.fsync(f.fileno())
    os.replace(tmp, MEMFILE)

def check(addr, width):
    """Validate width, range, and alignment."""
    if width not in (1, 2, 4, 8):
        abort(400, "bad width (must be 1,2,4,8)")
    if addr < BASE or addr + width - 1 > END:
        abort(403, "address out of allowed range")
    if ((addr - BASE) % width) != 0:
        abort(400, "misaligned address (must be aligned to width)")

def addr_sequence_from_start_end_or_count(start, end=None, count=None, width=4):
    """Return a list of addresses from start..end inclusive stepping by width,
       or start with count items (start + i*width)."""
    if count is not None:
        if count <= 0:
            abort(400, "count must be >= 1")
        end_calc = start + width * (count - 1)
        end = end_calc
    if end is None:
        abort(400, "either end or count must be provided")
    if end < start:
        abort(400, "end must be >= start")
    # make sure start and end are aligned and in range via check for each
    addrs = []
    addr = start
    while addr <= end:
        check(addr, width)
        addrs.append(addr)
        addr += width
    return addrs

@app.route("/api/v1/read")
def api_read():
    addr_s = request.args.get("addr")
    width_s = request.args.get("width", "4")
    if not addr_s:
        abort(400, "addr parameter required")
    try:
        addr = int(addr_s, 0)
        width = int(width_s, 0)
    except ValueError:
        abort(400, "addr/width must be integers (use 0x... for hex)")
    check(addr, width)
    offset = addr - BASE
    val = int.from_bytes(memory[offset:offset + width], "little")
    return jsonify(addr=hex(addr), width=width, value=hex(val))

@app.route("/api/v1/write", methods=["POST"])
def api_write():
    j = request.get_json(force=True)
    if not j:
        abort(400, "JSON body required")
    try:
        addr = int(j["addr"], 0)
        width = int(j["width"])
        value = int(j["value"], 0)
    except (KeyError, ValueError):
        abort(400, "JSON must contain addr, width, value (addr/value can be hex)")
    check(addr, width)
    offset = addr - BASE

    # refuse write if existing bytes are non-zero
    existing = memory[offset:offset + width]
    for b in existing:
        if b != 0:
            abort(403, "existing value is non-zero; clear before writing")

    try:
        memory[offset:offset + width] = value.to_bytes(width, "little")
    except OverflowError:
        abort(400, "value too large for given width")
    save_memory()
    return jsonify(status="ok", addr=hex(addr), width=width, value=hex(value))

@app.route("/api/v1/read_range")
def api_read_range():
    # supports start+end OR start+count
    start_s = request.args.get("start")
    end_s = request.args.get("end")
    count_s = request.args.get("count")
    width_s = request.args.get("width", "4")
    if not start_s:
        abort(400, "start parameter required")
    try:
        start = int(start_s, 0)
        width = int(width_s, 0)
    except ValueError:
        abort(400, "start/width must be integers")
    end = None
    count = None
    if end_s:
        try:
            end = int(end_s, 0)
        except ValueError:
            abort(400, "end must be integer")
    if count_s:
        try:
            count = int(count_s, 0)
        except ValueError:
            abort(400, "count must be integer")
    addrs = addr_sequence_from_start_end_or_count(start, end=end, count=count, width=width)
    result = {}
    for addr in addrs:
        offset = addr - BASE
        val = int.from_bytes(memory[offset:offset + width], "little")
        result[hex(addr)] = hex(val)
    return jsonify(status="ok", width=width, count=len(addrs), data=result)

@app.route("/api/v1/write_range", methods=["POST"])
def api_write_range():
    """
    POST JSON:
      { "start":"0x80000000", "count":N, "width":4, "values":[v1,v2,...] }
    OR:
      { "start":"0x80000000", "count":N, "width":4, "value":v }  # repeat v N times
    Rules:
      - alignment, range checks apply
      - write refused if any target bytes are non-zero
      - length of values must equal count if using values array
    """
    j = request.get_json(force=True)
    if not j:
        abort(400, "JSON body required")
    try:
        start = int(j["start"], 0)
        count = int(j["count"])
        width = int(j.get("width", 4))
    except (KeyError, ValueError):
        abort(400, "JSON must contain start (hex), count (int), optional width (int)")
    # build address list
    addrs = addr_sequence_from_start_end_or_count(start, count=count, width=width)
    # decide values
    values = None
    if "values" in j:
        if not isinstance(j["values"], list):
            abort(400, "values must be a list")
        if len(j["values"]) != len(addrs):
            abort(400, "values list length must equal count")
        try:
            values = [int(x, 0) for x in j["values"]]
        except ValueError:
            abort(400, "values must be integers (hex ok)")
    elif "value" in j:
        try:
            v = int(j["value"], 0)
        except ValueError:
            abort(400, "value must be integer")
        values = [v] * len(addrs)
    else:
        abort(400, "provide either 'values' list or single 'value'")

    # Ensure none of the target slots are non-zero
    offending = []
    for addr in addrs:
        offset = addr - BASE
        existing = memory[offset:offset + width]
        if any(b != 0 for b in existing):
            offending.append(hex(addr))
    if offending:
        abort(403, "existing non-zero at addresses: " + ",".join(offending))

    # perform writes
    written = []
    for addr, val in zip(addrs, values):
        offset = addr - BASE
        try:
            memory[offset:offset + width] = int(val).to_bytes(width, "little")
        except OverflowError:
            abort(400, f"value {val} too large for width {width} at addr {hex(addr)}")
        written.append({"addr": hex(addr), "value": hex(int(val))})
    save_memory()
    return jsonify(status="ok", count=len(written), written=written)

@app.route("/api/v1/clear")
def api_clear():
    addr_s = request.args.get("addr")
    width_s = request.args.get("width", "4")
    if not addr_s:
        abort(400, "addr parameter required")
    try:
        addr = int(addr_s, 0)
        width = int(width_s, 0)
    except ValueError:
        abort(400, "addr/width must be integers")
    check(addr, width)
    offset = addr - BASE
    memory[offset:offset + width] = (0).to_bytes(width, "little")
    save_memory()
    return jsonify(status="cleared", addr=hex(addr), width=width, value="0x0")

@app.route("/api/v1/clear_range", methods=["POST"])
def api_clear_range():
    j = request.get_json(force=True)
    try:
        start = int(j["start"], 0)
        end = int(j["end"], 0)
        width = int(j.get("width", 4))
    except (KeyError, ValueError):
        abort(400, "JSON must contain start,end,width")
    if end < start:
        abort(400, "end must be >= start")
    addrs = addr_sequence_from_start_end_or_count(start, end=end, width=width)
    for addr in addrs:
        offset = addr - BASE
        memory[offset:offset + width] = (0).to_bytes(width, "little")
    save_memory()
    return jsonify(status="cleared_range", start=hex(start), end=hex(end), width=width, count=len(addrs))

@app.route("/api/v1/clear_all", methods=["POST"])
def api_clear_all():
    j = request.get_json(silent=True)
    if not j or j.get("confirm") is not True:
        abort(400, "must POST JSON {\"confirm\": true} to clear all")
    for i in range(SIZE):
        memory[i] = 0
    save_memory()
    return jsonify(status="cleared_all", size=SIZE, base=hex(BASE))

if __name__ == "__main__":
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.load_cert_chain("certs/server.crt", "certs/server.key")
    app.run(host="0.0.0.0", port=8443, ssl_context=context, threaded=True)
