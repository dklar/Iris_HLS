import cv2
import os

f = open("results.txt")
if not os.path.exists("Test"):
    os.mkdir("Test")

i = 0
for line in f:
    values = line.split(";")
    path = values[0]
    path = path.replace("\n","/")
    path = path.replace("//","/")
    x  = int(values[1])
    y  = int(values[2])
    r  = int(values[3])
    r2  = int(values[4])
    image = cv2.imread(path)
    cv2.circle(image,(x,y),r,(0,255,0),2)
    cv2.circle(image,(x,y),r2,(0,255,0),2)
    cv2.imwrite(os.getcwd()+"/Test/"+str(i)+".jpg",image)
    i+=1
