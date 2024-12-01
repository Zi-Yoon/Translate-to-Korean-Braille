#*************************************************#
# Name		:	get_medicine_info.py
# Author	:	An Jiyoon (Ziyoon)
#*************************************************#

# sudo systemctl daemon-reload
# sudo systemctl enable your_script.service
# sudo systemctl start your_script.service -> 즉시 시작
# sudo systemctl status your_script.servic -> 상태 확인

import requests
import pandas as pd
from bs4 import BeautifulSoup
from gtts import gTTS
#from pydub import AudioSegment
import pygame
import os
import RPi.GPIO as GPIO
import subprocess

GPIO.setmode(GPIO.BCM)
GPIO.setup(26, GPIO.IN)

# 의약품 일련 번호 조회
# https://biz.kpis.or.kr/kpis_biz/index.jsp?sso=ok 
# https://nedrug.mfds.go.kr/searchDrug

# 예시 품목
# 8806723001803 타이레놀

# 파일 Read
df = pd.read_csv("medicine_information.csv", sep='\t')
df = df.assign(표준코드명=df['표준코드명'].str.split(',')).explode('표준코드명')

barcode = 0

while (1):
    barcode_new = 0
    # 외부에서 바코드 인식 후 바코드 번호를 입력받아서 사용
    if barcode == 0:
        barcode = input("Scan barcode : ")

    # barcode = 8806723001827
    row = df[df['표준코드명'] == str(barcode)]
    if(row.empty == True):
        print("** Not in Database ** Check the barcode **")
        continue
    medicine_code = row['품목기준코드'].values[0]
    medicine_name = row['제품명'].values[0]
    medicine_name = medicine_name.replace("(", "").replace(")", "")

    # 이름 저장
    with open("medicine_name.txt", "w") as file:
        file.write(medicine_name)

    # Make request to the website
    url = f"https://nedrug.mfds.go.kr/pbp/CCBBB01/getItemDetailCache?cacheSeq={medicine_code}aupdateTs2023-07-19%2009:47:31.602187b"
    response = requests.get(url)

    # Parse HTML and save "용법 용량" contents
    soup = BeautifulSoup(response.text, 'html.parser')

    # Finding the elements containing "용법 용량". 
    medicine_use_elements = soup.select('.info_box.mt20.pt0 p')

    medicine_use = ''
    for i, element in enumerate(medicine_use_elements):
        if i < 4:  # Only process first 4 elements
            medicine_use += element.text.strip() + '\n'  # Add newline character after each line

    with open('medicine_use.txt', 'w', encoding='utf-8') as f:
        f.write(medicine_use.strip())  # Remove trailing newline character

    with open('medicine_use.txt', 'r', encoding='utf-8') as f:
        text = f.read()

    speech = gTTS(text, lang='ko')
    speech.save('medicine_use.mp3')
    c_program = subprocess.Popen(["./a.out"])
    while(1):
        sound = GPIO.input(26)
        if sound == 0:
            pygame.mixer.init()
            pygame.mixer.music.load('medicine_use.mp3')
            pygame.mixer.music.set_volume(1.0)
            pygame.mixer.music.play()
            pygame.time.Clock().tick(10)
        try:
            barcode_new = input("Press Enter to stop and scan again...")
            if barcode_new != 0:
                barcode = barcode_new
                c_program.terminate()
                break
        except:
            pass

# 사용하지않는 api
def get_medicine_info(medicine_name):
    url = "http://apis.data.go.kr/1471000/DrbEasyDrugInfoService/getDrbEasyDrugList"

    # 실제 서비스키
    service_key = "mcqNw9xhEDxZjS1wsVLgVXFE9l65fbHZlGzmcOnbEkubqtJfDcc1XiWGBL0NyCuoBQAm7uzxoVnFGYik/wBEVw=="
    # mcqNw9xhEDxZjS1wsVLgVXFE9l65fbHZlGzmcOnbEkubqtJfDcc1XiWGBL0NyCuoBQAm7uzxoVnFGYik%2FwBEVw%3D%3D
    # mcqNw9xhEDxZjS1wsVLgVXFE9l65fbHZlGzmcOnbEkubqtJfDcc1XiWGBL0NyCuoBQAm7uzxoVnFGYik/wBEVw==

    params = {
        "ServiceKey": service_key,
        "itemName": medicine_name, # 바코드 번호를 이용하여 의약품 정보를 조회합니다.
    }

    response = requests.get(url, params=params)

    if response.status_code == 200:
        return response.text
    else:
        return None