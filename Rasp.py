#-*-coding:Utf-8 -*-

from urllib2 import Request, urlopen
import json
import datetime
import RPi.GPIO as GPIO
from gpiozero import LED
import time

url = "http://apis.data.go.kr/1741000/DisasterMsg2/getDisasterMsgList?ServiceKey=sIb%2BMzgO6QA%2FeIf3nFwuS9PrVkF9DR2qEsSa1vq9jOJ0CZDWszPL%2FOTVn25icM5y7Nj0vHFDjDra0l7QHxdJoQ%3D%3D&type=json&pageNo=1&numOfRows=1"


def getAPI():
	while True:
		request = Request(url)
		request.get_method = lambda: 'GET'
		response_body = urlopen(request).read()
		if response_body == None:
			print ("no respone")
			continue;
		j = json.loads(response_body)
                break

	return j


def alerm():
	GPIO.output(23, GPIO.HIGH)
	time.sleep(10)
	GPIO.output(23, GPIO.LOW)


# main
try:
	GPIO.setmode(GPIO.BCM)
	GPIO.setup(23, GPIO.OUT)
	GPIO.output(23, GPIO.LOW)

	while True:
		print("loop start")
		j = getAPI()
		now = datetime.datetime.now()

		dt = j["DisasterMsg"][1]["row"][0]
		msg = dt["msg"]
		loc = int(dt["location_id"])
		ctime = dt["create_date"]

		Y = int(ctime[0:4])
		M = int(ctime[5:7])
		D = int(ctime[8:10])
		h = int(ctime[11:13])
		m = int(ctime[14:16])

		print (loc)
		print (ctime)
		print (msg)

#		if Y == now.year and M == now.month and D == now.day and h == now.hour:
#			if m == now.minute and loc == 158:
		if loc==158:
			print("alerm")
			alerm()

		print("sleep start")
		time.sleep(20)

except KeyboardInterrupt:
	GPIO.output(23, GPIO.LOW)
	GPIO.cleanup()