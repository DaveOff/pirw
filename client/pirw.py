import websocket, win32gui, win32con

def on_message(ws, message):
    if(message != "Welcome"):
        hwnd = win32gui.GetForegroundWindow()
        win32gui.ShowWindow(hwnd, win32con.SW_MINIMIZE)

def on_error(ws, error):
    print(error)

websocket.WebSocketApp("ws://ip:8081", on_message=on_message, on_error=on_error).run_forever()
