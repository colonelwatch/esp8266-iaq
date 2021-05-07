import requests
import matplotlib.pyplot as plt
import matplotlib.animation as animation

ID = 'YOUR ESP8266 ID HERE'
UPPER_CO2_THRESHOLD = 2000
LOWER_CO2_THRESHOLD = 1500

fig = plt.figure()
ax1 = fig.add_subplot(4, 1, (1, 2))
ax2 = fig.add_subplot(4, 1, 3)
ax3 = fig.add_subplot(4, 1, 4)

time = []
co2 = []
co2_SMA = []
co2_open_window_event = []
co2_close_window_event = []
humidity = []
fahrenheit = []

state = None
def update_co2_events():
    global state
    last = co2_SMA[-1]
    time_last = time[-1]
    if state == None: # default to below behavior
        state = 'increasing' if last < UPPER_CO2_THRESHOLD else 'decreasing'
    
    if state == 'increasing' and last > UPPER_CO2_THRESHOLD:
        state = 'decreasing'
        co2_open_window_event.append((time_last, last)) # This just adds a plot point, any arbitary action can go here
    elif state == 'decreasing' and last < LOWER_CO2_THRESHOLD:
        state = 'increasing'
        co2_close_window_event.append((time_last, last))

def update_SMA(array, SMA, n):
    count = 0
    sum = 0
    for j in range(n):
        try:
            sum += array[-1-j]
            count += 1
        except IndexError:
            pass
    SMA.append(sum/count)

def update(i):
    json_response = requests.get('https://dweet.io/get/latest/dweet/for/'+ID).json()
    co2.append(json_response['with'][0]['content']['co2'])
    humidity.append(json_response['with'][0]['content']['humidity'])
    fahrenheit.append(json_response['with'][0]['content']['celsius']*1.8+32)

    try:
        last = time[-1]
        time.append(last+2)
    except IndexError:
        time.append(0)

    update_SMA(co2, co2_SMA, 5)
    update_co2_events()
    
    ax1.clear()
    ax1.plot(time, co2)
    ax1.plot(time, co2_SMA)
    if co2_open_window_event:
        x, y = zip(*co2_open_window_event) # Makes tuple list plottable in matplotlib
        ax1.scatter(x, y)
    if co2_close_window_event:
        x, y = zip(*co2_close_window_event)
        ax1.scatter(x, y)
    ax1.set_ylabel('eCO2 (ppm)')

    ax2.clear()
    ax2.plot(time, humidity)
    ax2.set_ylabel('RH (%)')

    ax3.clear()
    ax3.plot(time, fahrenheit)
    ax3.set_ylabel('Temperature (F)')

    ax3.set_xlabel('Time (s)') # Functions as a global xlabel

ani = animation.FuncAnimation(fig, update, interval=2000)
plt.show()