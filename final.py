import urequests, ujson
import xtools, utime
from machine import UART, Pin
import gc
from umqtt.simple import MQTTClient
import ssl

gc.collect()
xtools.connect_wifi_led()

# OpenWeatherMap 設定
API_key = "9ff2e87bf134ad54b2e067f9c68c0a3f"
country = "TW"
# Adafruit
ADAFRUIT_IO_USERNAME = "csyhh"
ADAFRUIT_IO_KEY      = "aio_DEPF42QHobHSDY6TNVNzjbbT1LG8"

topic = "csy01157034"

mode = 5

cities = ["Taipei", "New%20Taipei", "Taoyuan", "Taichung", "Tainan", "Kaohsiung"]
short_names = ["TPE ", "NTP ", "TYN ", "TXG ", "TNN ", "KHH "]  # 六都縮寫
temps = [0] * 6

# UART 初始化（串接 8051）
uart = UART(2, baudrate=9600, tx=17, rx=16)

# MQTT 客戶端
client = MQTTClient (
    client_id = xtools.get_id(),
    server = "broker.hivemq.com",
    ssl = False,
)

num_pad_feed ="final-num"
get_time_city = "TPE"
write_time = 0

city_time_zone_dic = {
    "TPE": "Asia/Taipei",
    "TYO": "Asia/Tokyo",
    "MOW": "Europe/Moscow",
    "LON": "Europe/London",
    "PAR": "Europe/Paris",
    "LAX": "America/Los_Angeles",
    "NYC": "America/New_York",
}

get_time_city_dic = {
    "tpe": "TPE",
    "taipei": "TPE",
    "台北": "TPE",
    "臺北": "TPE",
    "mow": "MOW",
    "moscow": "MOW",
    "莫斯科": "MOW",
    "la": "LAX",
    "lax": "LAX",
    "los angeles": "LAX",
    "losangeles": "LAX",
    "洛杉磯": "LAX",
    "nyc": "NYC",
    "new york": "NYC",
    "newyork": "NYC",
    "紐約": "NYC",
    "tyo": "TYO",
    "tokyo": "TYO",
    "東京": "TYO",
    "lon": "LON",
    "london": "LON",
    "倫敦": "LON",
    "paris": "PAR",
    "par": "PAR",
    "巴黎": "PAR",
}

def sub_cb(topic, msg):
    global mode
    if isinstance(msg, bytes):
        msg = msg.decode().strip()
    print("收到訊息: ", msg)
    if msg.startswith("mode"):
        mode = int(msg[-1])
        print(f"change to mode {mode}")
        uart.write(b'M' + str(mode).encode() + b'      ')
        if mode == 1:
            print(f"更新氣溫資料...")
            uart.write(f"UPDATE..")
            for i in range(6):
                temps[i] = get_temperature(cities[i])
                print(f"{short_names[i]}: {temps[i]} °C")
                utime.sleep(1)
            uart.write(f"DONE    ")
    elif mode == 2:
        P2(msg)
    elif mode == 3:
        P3(msg)
    elif mode == 5:
        get_time(msg)
    elif mode == 6:
        P6(msg)

# 抓取指定城市的氣溫（整數）
def get_temperature(city):
    url  = "https://api.openweathermap.org/data/2.5/weather?"
    url += "q=" + city + "," + country
    url += "&units=metric&lang=zh_tw&"
    url += "appid=" + API_key
    try:
        response = urequests.get(url)
        data = ujson.loads(response.text)
        response.close()
        del response
        gc.collect()
        return int(round(data["main"]["temp"]))
    except Exception as e:
        print("取得溫度錯誤:", e)
        return 0

client.set_callback(sub_cb)   # 指定回撥函數來接收訊息
client.connect()              # 連線

topic = "csy01157034"
print(topic)
client.subscribe(topic)      # 訂閱主題

# 主迴圈：監聽來自 8051 的按鍵請求

def get_highest_temp_index():
    max_temp = temps[0]
    idx = 0
    for i in range(1, 6):
        if temps[i] > max_temp:
            max_temp = temps[i]
            idx = i
    return idx

def send_to_feed(feed, value):
    url = f"https://io.adafruit.com/api/v2/{ADAFRUIT_IO_USERNAME}/feeds/{feed}/data"
    headers = {"X-AIO-Key": ADAFRUIT_IO_KEY}
    data = {"value": value}
    try:
        res = urequests.post(url, headers=headers, json=data)
        res.close()
        del res
        gc.collect()
    except Exception as e:
        print("上傳失敗:", feed, "錯誤:", e)

def get_time(city_key):
    city_key = city_key.lower()
    if city_key not in get_time_city_dic:
        print("未知城市")
        return
    timezone = city_time_zone_dic[get_time_city_dic[city_key]]
    url = f"https://io.adafruit.com/api/v2/{ADAFRUIT_IO_USERNAME}/integrations/time/strftime"
    url += f"?x-aio-key={ADAFRUIT_IO_KEY}&tz={timezone}"
    url += "&fmt=%25Y-%25m-%25d+%25H%3A%25M%3A%25S.%25L+%25j+%25u+%25z+%25Z"

    try:
        global datetime_str
        global get_time_city
        get_time_city = get_time_city_dic[city_key]
        res = urequests.get(url)
        datetime_str = res.text.strip()
        print(f"{city_key} {get_time_city_dic[city_key]} 現在時間：{datetime_str}")
        # uart.write(datetime_str.encode())  
        res.close()
        gc.collect()
    except Exception as e:
        print("取得時間失敗:", e)

def P1():
    key = uart.readline()
    print(f"接收到key: {key}")
    if key in [b'0', b'1', b'2', b'3', b'4', b'5']:
        print(f"接收到key: {key}")
        idx = int(key)
        temp_val = temps[idx]
        temp_str = str(temp_val)
        uart.write(f"{short_names[idx]}{temp_str}%C")
        print(f"{short_names[idx]} {temp_str}%C")
        print(f"回傳城市 {short_names[idx]} 溫度: {temp_str[0]}{temp_str[1]}")
    elif key == b'6':  # 最高溫城市
        print(f"接收到key: {key}")
        idx = get_highest_temp_index()
        temp_val = temps[idx]
        temp_str = str(temp_val)
        uart.write(f"HOT {temp_str}%C")
        print(f"HOT {temp_str}%C")
        print(f"回傳最高溫城市 {short_names[idx]} 溫度: {temp_str}")
    elif key == b'7':  
        print(f"更新氣溫資料...")
        uart.write(f"UPDATE..")
        for i in range(6):
            temps[i] = get_temperature(cities[i])
            print(f"{short_names[i]}: {temps[i]} °C")
            utime.sleep(1)
        uart.write(f"DONE    ")

def P2(cmd):
    print(f"接收到指令: {cmd}")
    valid_notes = {
        "C4", "D4", "E4", "F4", "G4", "A4", "B4",
        "C5", "D5", "E5", "F5", "G5", "A5", "B5",
        "C6", "D6", "E6", "F6", "G6", "A6", "B6",
        "C7"
    }
    
    if cmd in valid_notes:
        uart.write(f"{cmd:<8}")
        print(f"發出{cmd}")
    else:
        print("錯誤指令，不發聲")

    
def P3(song_str):
    print(f"接收到編曲: {song_str}")
    # 檢查內容合法：只能有 1-8 或 .，長度大於0
    for c in song_str:
        if c not in '12345678.':
            print("格式錯誤，只允許 1~8 和 '.'")
            return
    song_str = song_str + '_'
    # 每8個字元為一組送出
    i = 0
    while i < len(song_str):
        group = song_str[i:i+8]
        group_padded = group + ('_' * (8 - len(group)))  # 若不足8字補滿
        uart.write(group_padded)
        print(f"送出: {group_padded}")
        i += 8
        utime.sleep(1)  # 可視實際播放需求調整間隔
    
def P4():
    num = uart.readline()
    print(f"接收到num: {num}")
    if num in [b'1', b'2', b'3', b'4', b'5', b'6', b'7', b'8', b'9', b'*', b'0', b'#']:
        print(f"接收到num: {num}")
        send_to_feed(num_pad_feed, num)

def P5():
    global datetime_str
    global get_time_city
    global write_time
    if write_time == 0:
        get_time(get_time_city)
        uart.write(f"{get_time_city:<8}")
        write_time = 1
    elif write_time == 1:
        uart.write(f"{datetime_str[5]}{datetime_str[6]}{datetime_str[8]}{datetime_str[9]}{datetime_str[11]}{datetime_str[12]}{datetime_str[14]}{datetime_str[15]}")
        write_time = 0
    #uart.write(f"{get_time_city} {datetime_str[11]}{datetime_str[12]}{datetime_str[14]}{datetime_str[15]}")
    print(f"{get_time_city} {datetime_str[11]}{datetime_str[12]}{datetime_str[14]}{datetime_str[15]}")
    
def P6(cmd):
    # 若是純數字，代表設定倒數秒數
    if cmd.isdigit():
        val = int(cmd)
        if val > 9999:
            print("錯誤：秒數超過 9999")
            return
        timer_cmd = '0' * (4 - len(cmd)) + cmd
        uart.write(f"{timer_cmd:<8}")
        print(f"傳送倒數秒數：{timer_cmd:<8}")
        return

    # 特殊指令處理：ON、OFF、+增加、-減少
    cmd = cmd.upper()

    if cmd == "ON":
        uart.write("ON      ")  # 固定 8 字元
        print("傳送：ON(繼續倒數)")
    elif cmd == "OFF":
        uart.write("OFF     ")
        print("傳送：OFF(暫停倒數)")
    elif cmd.startswith("+"):
        num = cmd[1:].strip()
        if not num.isdigit() or len(num) > 4:
            print("錯誤：增加秒數格式錯誤(如 +30)")
            return
        timer_sec = '0' * (4 - len(num)) + num
        uart.write(f"+   {timer_sec}")
        print(f"傳送：增加 {timer_sec} 秒")
    elif cmd.startswith("-"):
        num = cmd[1:].strip()
        if not num.isdigit() or len(num) > 4:
            print("錯誤：減少秒數格式錯誤(如 -10)")
            return
        timer_sec = '0' * (4 - len(num)) + num
        uart.write(f"-   {timer_sec}")
        print(f"傳送：減少 {timer_sec} 秒")
    else:
        print("無效指令，請輸入：數字、ON、OFF、+xxxx 或 -xxxx")
        
uart.write(f"M{mode:<7}")
while True:
    client.check_msg()
    print(f"current mode {mode} ")
    if mode == 2:
        print(f"可接收發聲指令 例如:C4")
    elif mode == 5:
        P5()
    else:
        if uart.any() > 0:
            if mode == 1:
                P1()
            elif mode == 4:
                P4()
    utime.sleep(5)
