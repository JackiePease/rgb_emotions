import machine
import neopixel
import network
import time
import random
from ucollections import deque
import utime

from minimalmdns import mdnshostnametoipnumber
from umqtt.robust import MQTTClient


number_of_leds = 90

wifi_ssid = 'DoESLiverpool'
wifi_password = 'decafbad00'
mqtt_broker_hostname = '10.0.100.1'
mqtt_client_name = "{}{}".format('dinky-esp32-', machine.unique_id())
mqtt_topic_base = "dinky/"

channels = [
    "motion",
]

state = {
    "motion": 0,
}

motiontimes = deque((),60)

# Set up neopixel string.
np = neopixel.NeoPixel(
    machine.Pin(4),
    number_of_leds,
    bpp=3
)


# The callback that handles MQTT messages.
def mqtt_message_callback(topic, msg):
    topic = topic.decode('utf-8')
    msg = msg.decode('utf-8').strip()
    print("MQTT message:", topic, msg)

    if topic.startswith(mqtt_topic_base):
        # Remove the mqtt_topic_base from the start of the topic,
        # leaving only the subtopic
        channel = topic[len(mqtt_topic_base):]

        print("channel: ", channel)
        print("msg: ", int(msg))
        # A single channel/colour pair.
        if channel in channels:
            state[channel] = int(msg)
            print("motion", int(msg))

            # Add this event to our list
            motiontimes.append(utime.ticks_ms())

            # Remove entries older than a minute
            checking = True
            while checking:
                currententry = motiontimes.popleft()
                if utime.ticks_ms() - currententry < 60000:
                    motiontimes.appendleft(currententry)
                    checking = False

           
def update_leds():


    for x in range(0, number_of_leds):
        if state["motion"] == 1:  
            np[x] = (255, 0, 0, 0)
        else:
            np[x] = (0, 255, 0, 0)

    np.write()



# Set all LEDs to the same colour. Useful for debugging.
def fill_all_leds(r, g, b, w):
    for x in range(0, number_of_leds):
        np[x] = (r, g, b, w)
    np.write()


# Clear LEDs before we start.
fill_all_leds(0, 0, 0, 0)


# Wait a little bit before we turn on wifi.
time.sleep(1.5)
wifi = network.WLAN(network.STA_IF)
wifi.active(True)


print("Connecting to", wifi_ssid)
fill_all_leds(0, 0, 255, 0) # blue


# Connect to the specified wifi network.
wifi.connect(wifi_ssid, wifi_password)
while not wifi.isconnected():
    time.sleep(0.1)


print("Connected to", wifi_ssid)
fill_all_leds(0, 255, 0, 0) # green
time.sleep(1)


print("Connecting to MQTT broker")
fill_all_leds(255, 150, 0, 0) # yellow
time.sleep(2)


# Find out the IP address of the MQTT broker.
if mqtt_broker_hostname[-6:] == ".local":
    print("Looking up MQTT broker IP address")
    mqtt_broker_ip = mdnshostnametoipnumber(wifi, mqtt_broker_hostname)
    print("mDNS completed:", mqtt_broker_hostname, "=", mqtt_broker_ip)
else:
    mqtt_broker_ip = mqtt_broker_hostname


# Connect to MQTT broker.
client = MQTTClient(mqtt_client_name, mqtt_broker_ip, 1883)
print("Connecting to MQTT broker", mqtt_broker_ip, "as client", mqtt_client_name)
for i in range(100):
    try:
        client.connect()
        break
    except OSError as e:
        print(e)
        fill_all_leds(255, 0, 0, 0)


print("Connected to MQTT broker", mqtt_broker_ip)
for i in range(0, 3):
    fill_all_leds(0, 0, 0, 0)
    time.sleep(0.3)
    fill_all_leds(0, 0, 0, 255)
    time.sleep(0.3)


# Reset LEDs again, ready for normal operation.
fill_all_leds(0, 0, 0, 0)


# Listen for MQTT messages.
client.set_callback(mqtt_message_callback)
client.subscribe( "{}#".format(mqtt_topic_base) )


while True:
    client.check_msg()
    update_leds()
    time.sleep_ms(25)
