"""
Tiny command-line client for the Unreal editor's MCP server (http://127.0.0.1:8000/mcp).

Lets tools and scripts drive the editor even when an AI chat's MCP connection is down
(e.g. the chat started before the editor did).

Usage:
    python Tools/ue_mcp.py list
    python Tools/ue_mcp.py describe <toolset>
    python Tools/ue_mcp.py call <toolset> <tool> [json-arguments]
    python Tools/ue_mcp.py call <toolset> <tool> @args.json
    python Tools/ue_mcp.py call ... --image out.png   (saves a returned base64 image instead of printing it)
"""

import base64
import json
import sys
import urllib.request

URL = "http://127.0.0.1:8000/mcp"


def _post(payload, session=None):
    headers = {"Content-Type": "application/json", "Accept": "application/json, text/event-stream"}
    if session:
        headers["Mcp-Session-Id"] = session
    request = urllib.request.Request(URL, data=json.dumps(payload).encode(), headers=headers, method="POST")
    with urllib.request.urlopen(request, timeout=600) as response:
        body = response.read().decode("utf-8", errors="replace")
        session = response.headers.get("Mcp-Session-Id") or session
    if not body.strip():
        return None, session
    # Streamable HTTP may answer as server-sent events: take the last "data:" line
    if body.lstrip().startswith("event:") or "\ndata:" in body or body.startswith("data:"):
        data = [line[5:].strip() for line in body.splitlines() if line.startswith("data:")]
        body = data[-1] if data else "{}"
    return json.loads(body), session


def _session():
    result, session = _post({
        "jsonrpc": "2.0", "id": 1, "method": "initialize",
        "params": {"protocolVersion": "2025-03-26", "capabilities": {},
                   "clientInfo": {"name": "ue_mcp.py", "version": "1.0"}}})
    _post({"jsonrpc": "2.0", "method": "notifications/initialized"}, session)
    return session


def call_tool(name, arguments, session):
    reply, _ = _post({"jsonrpc": "2.0", "id": 2, "method": "tools/call",
                      "params": {"name": name, "arguments": arguments}}, session)
    if reply is None:
        return None
    if "error" in reply:
        raise RuntimeError(json.dumps(reply["error"]))
    return reply.get("result")


def main(argv):
    if len(argv) < 2:
        print(__doc__)
        return 1

    image_path = None
    if "--image" in argv:
        index = argv.index("--image")
        image_path = argv[index + 1]
        argv = argv[:index] + argv[index + 2:]

    session = _session()
    command = argv[1]
    if command == "list":
        result = call_tool("list_toolsets", {}, session)
    elif command == "describe":
        result = call_tool("describe_toolset", {"toolset_name": argv[2]}, session)
    elif command == "call":
        arguments = {}
        if len(argv) > 4:
            raw = argv[4]
            arguments = json.load(open(raw[1:], encoding="utf-8")) if raw.startswith("@") else json.loads(raw)
        result = call_tool("call_tool", {"toolset_name": argv[2], "tool_name": argv[3], "arguments": arguments}, session)
    else:
        print(__doc__)
        return 1

    text = "\n".join(part.get("text", "") for part in (result or {}).get("content", []) if part.get("type") == "text")
    if image_path:
        # Tool results carry images as base64 "data" inside their JSON text
        data = json.loads(text)
        image = data.get("returnValue", data)
        image = image.get("image", image)
        with open(image_path, "wb") as file:
            file.write(base64.b64decode(image["data"]))
        print("saved", image_path)
    else:
        print(text if text else json.dumps(result, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
