# This file is executed on every boot (including wake-boot from deepsleep)
#import esp
#esp.osdebug(None)
import uos, machine
#uos.dupterm(None, 1) # disable REPL on UART(0)
import gc
#import webrepl
#webrepl.start()
gc.collect()

#set up wi-fi connection on boot. WiFi credential in config.py
def do_connect():
    import network
    import config
    sta_if = network.WLAN(network.STA_IF)
    if not sta_if.isconnected():
        print('connecting to network...')
        sta_if.active(True)
        sta_if.connect(config.WIFI_SSID, config.WIFI_PWD)
        while not sta_if.isconnected():
            pass
    print('network config:', sta_if.ifconfig())
    ap_if = network.WLAN(network.AP_IF)
    ap_if.active(False)

do_connect()
