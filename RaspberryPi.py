#-*-coding:Utf-8 -*-

from urllib2 import Request, urlopen, URLError
import urllib2
import json
import datetime
import RPi.GPIO as GPIO
from gpiozero import LED
import time
import picamera
import numpy as np
import Image
import paho.mqtt.client as mqtt
import random


url = "http://apis.data.go.kr/1741000/DisasterMsg2/getDisasterMsgList?ServiceKey=sIb%2BMzgO6QA%2FeIf3nFwuS9PrVkF9DR2qEsSa1vq9jOJ0CZDWszPL%2FOTVn25icM5y7Nj0vHFDjDra0l7QHxdJoQ%3D%3D&type=json&pageNo=1&numOfRows=1"


def getAPI():
    try:
        request = Request(url)
        print (request)
        request.get_method = lambda: 'GET'
        response_body = urlopen(request).read()
        print (response_body)
        j = json.loads(response_body)
    except urllib2.URLError:
        return getAPI()
    return j


def alerm():
	GPIO.output(23, GPIO.HIGH)
	time.sleep(10)
	GPIO.output(23, GPIO.LOW)



class Cluster(object):

    def __init__(self):
        self.pixels = []
        self.centroid = None

    def addPoint(self, pixel):
        self.pixels.append(pixel)

    def setNewCentroid(self):

        R = [colour[0] for colour in self.pixels]
        G = [colour[1] for colour in self.pixels]
        B = [colour[2] for colour in self.pixels]

        if len(R) != 0:
            R = sum(R) / len(R)
        if len(G) != 0:
            G = sum(G) / len(G)
        if len(B) != 0:
            B = sum(B) / len(B)

        self.centroid = (R, G, B)
        self.pixels = []

        return self.centroid


class Kmeans(object):

    def __init__(self, k=1, max_iterations=5, min_distance=5.0, size=200):
        self.k = k
        self.max_iterations = max_iterations
        self.min_distance = min_distance
        self.size = (size, size)

    def run(self, image):
        self.image = image
        self.image.thumbnail(self.size)
        self.pixels = np.array(image.getdata(), dtype=np.uint8)

        self.clusters = [None for i in range(self.k)]
        self.oldClusters = None

        randomPixels = random.sample(self.pixels, self.k)

        for idx in range(self.k):
            self.clusters[idx] = Cluster()
            self.clusters[idx].centroid = randomPixels[idx]

        iterations = 0

        while self.shouldExit(iterations) is False:

            self.oldClusters = [cluster.centroid for cluster in self.clusters]

            print iterations

            for pixel in self.pixels:
                self.assignClusters(pixel)

            for cluster in self.clusters:
                cluster.setNewCentroid()

            iterations += 1

        return [cluster.centroid for cluster in self.clusters]

    def assignClusters(self, pixel):
        shortest = float('Inf')
        for cluster in self.clusters:
            distance = self.calcDistance(cluster.centroid, pixel)
            if distance < shortest:
                shortest = distance
                nearest = cluster

        nearest.addPoint(pixel)

    def calcDistance(self, a, b):

        result = np.sqrt(sum((a - b) ** 2))
        return result

    def shouldExit(self, iterations):

        if self.oldClusters is None:
            return False

        for idx in range(self.k):
            dist = self.calcDistance(
                np.array(self.clusters[idx].centroid),
                np.array(self.oldClusters[idx])
            )
            if dist < self.min_distance:
                return True

        if iterations <= self.max_iterations:
            return False

        return True

    # ############################################
    # The remaining methods are used for debugging
    def showImage(self):
        self.image.show()

    def showCentroidColours(self):

        for cluster in self.clusters:
            image = Image.new("RGB", (200, 200), cluster.centroid)
            image.show()

    def showClustering(self):

        localPixels = [None] * len(self.image.getdata())

        for idx, pixel in enumerate(self.pixels):
                shortest = float('Inf')
                for cluster in self.clusters:
                    distance = self.calcDistance(cluster.centroid, pixel)
                    if distance < shortest:
                        shortest = distance
                        nearest = cluster

                localPixels[idx] = nearest.centroid

        w, h = self.image.size
        localPixels = np.asarray(localPixels)\
            .astype('uint8')\
            .reshape((h, w, 3))

        colourMap = Image.fromarray(localPixels)
        colourMap.show()
        
        
def getImage():
	with picamera.PiCamera() as camera:
                camera.brightness = 50
                camera.contrast = 50
                camera.saturation = 75
		time.sleep(2)
		camera.capture('image.jpg')
		im = Image.open('image.jpg')
                
                k = Kmeans()
                result = k.run(im)

		return result[0]

# main
try:
	print("start begin")
	GPIO.setmode(GPIO.BCM)
	GPIO.setup(23, GPIO.OUT)
	GPIO.output(23, GPIO.LOW)
	
	j = None
	cnt = 0
	while True:
		now = datetime.datetime.now()

		# get API
		if cnt == 20:
			print("getAPI start")
			j = getAPI()

			dt = j["DisasterMsg"][1]["row"][0]
			msg = dt["msg"]
			loc = int(dt["location_id"])
			ctime = dt["create_date"]

			print (loc)
			print (ctime)
			print (msg)


			Y = int(ctime[0:4])
			M = int(ctime[5:7])
			D = int(ctime[8:10])
			h = int(ctime[11:13])
			m = int(ctime[14:16])


			if Y == now.year and M == now.month and D == now.day and h == now.hour:
				if m == now.minute and loc == 158:
					print("alerm")
					alerm()

			cnt = 0
		
		# get Image
		print("getImage start")
		rgb = getImage()
		print (rgb)
		client = mqtt.Client()
		client.connect('localhost', 1883, 60)
		client.publish('/rasp', str(rgb[0]) + " " + str(rgb[1]) + " " + str(rgb[2]))

		# sleep 5 sec
		print("sleep")
		time.sleep(1)
		cnt += 1

except KeyboardInterrupt:
	GPIO.output(23, GPIO.LOW)
	GPIO.cleanup()
